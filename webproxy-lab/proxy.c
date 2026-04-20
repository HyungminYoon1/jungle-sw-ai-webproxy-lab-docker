#include "csapp.h"

/* 권장 최대 캐시 및 객체 크기 */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

/* 이 긴 줄을 코드에 포함해도 스타일 점수에서 불이익은 없습니다 */
static const char *user_agent_hdr =
    "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 "
    "Firefox/10.0.3\r\n";

/* 프록시가 처리할 요청 한 건을 수행한다. */
void doit(int connfd);
/* 연결 하나를 전담하는 스레드 진입 함수 */
void *thread(void *vargp);
/* 절대 URI에서 호스트, 경로, 포트를 분리한다. */
int parse_uri(const char *uri, char *host, char *path, char *port);
/* 원 서버로 전달할 HTTP/1.0 요청 헤더를 다시 구성한다. */
void build_http_header(char *http_header, const char *host, const char *path,
                       rio_t *client_rio);
/* 원 서버 응답을 읽어 클라이언트에 그대로 전달한다. */
void forward_response(int serverfd, int connfd);
/* 프록시 쪽 오류를 HTTP 응답 형태로 클라이언트에 돌려준다. */
void clienterror(int fd, const char *cause, const char *errnum,
                 const char *shortmsg, const char *longmsg);

/*
 * main - 반복형 프록시 서버의 진입점
 * 명령행 포트로 리스닝 소켓을 열고, 들어오는 연결을 하나씩 받아 doit()에 넘긴다.
 */
int main(int argc, char **argv)
{
    int listenfd;
    int *connfdp;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    char hostname[MAXLINE], port[MAXLINE];
    pthread_t tid;

    if (argc != 2)
    {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(1);
    }

    listenfd = Open_listenfd(argv[1]);
    while (1)
    {
        clientlen = sizeof(clientaddr);
        connfdp = Malloc(sizeof(int));
        *connfdp = Accept(listenfd, (SA *)&clientaddr, &clientlen);
        Getnameinfo((SA *)&clientaddr, clientlen, hostname, MAXLINE,
                    port, MAXLINE, 0);
        printf("Accepted connection from (%s, %s)\n", hostname, port);

        /* 각 연결은 분리된 스레드가 처리해서 다른 요청을 막지 않게 한다. */
        Pthread_create(&tid, NULL, thread, connfdp);
    }
}

/*
 * thread - 연결 하나를 처리하는 분리(detached) 스레드
 * 메인 스레드는 새 연결 수락에 집중하고,
 * 작업 스레드는 요청 처리 후 자신이 맡은 소켓을 닫는다.
 */
void *thread(void *vargp)
{
    int connfd = *((int *)vargp);

    Free(vargp);
    Pthread_detach(Pthread_self());

    doit(connfd);
    Close(connfd);
    return NULL;
}

/*
 * doit - 프록시 요청 한 건 처리
 * 1. 클라이언트의 요청 라인을 읽는다.
 * 2. URI에서 호스트/포트/경로를 파싱한다.
 * 3. 원 서버용 HTTP/1.0 헤더를 만든다.
 * 4. 원 서버에 연결해 요청을 보내고 응답을 그대로 중계한다.
 */
void doit(int connfd)
{
    int serverfd;
    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
    char host[MAXLINE], path[MAXLINE], port[MAXLINE];
    char http_header[MAXBUF];
    rio_t rio;

    /* 요청 라인을 읽고 메서드, URI, HTTP 버전을 파싱한다. */
    Rio_readinitb(&rio, connfd);
    if (!Rio_readlineb(&rio, buf, MAXLINE))
    {
        /* 연결 직후 바로 EOF가 오면 더 할 일이 없다. */
        return;
    }
    printf("Request headers:\n");
    printf("%s", buf);
    sscanf(buf, "%s %s %s", method, uri, version);

    /* Proxy는 GET 메서드만 처리한다. */
    if (strcasecmp(method, "GET"))
    {
        clienterror(connfd, method, "501", "Not implemented",
                    "Proxy does not implement this method");
        return;
    }

    if (parse_uri(uri, host, path, port) < 0)
    {
        /* 절대 URI를 기대했는데 파싱에 실패한 경우 */
        clienterror(connfd, uri, "400", "Bad request",
                    "Proxy could not parse the URI");
        return;
    }

    /* 클라이언트 헤더를 읽어가며 원 서버로 보낼 헤더를 다시 구성한다. */
    build_http_header(http_header, host, path, &rio);

    /* 파싱한 host, port로 실제 원 서버에 TCP 연결을 건다. */
    /*
     * 원 서버 연결 실패는 현재 요청만 실패시키고,
     * 프록시 프로세스 전체는 계속 살아 있도록 non-wrapper 버전을 사용한다.
     */
    serverfd = open_clientfd(host, port);
    if (serverfd < 0)
    {
        clienterror(connfd, host, "502", "Bad gateway",
                    "Proxy could not connect to the end server");
        return;
    }

    /* 서버에 요청을 보내고, 서버 응답은 클라이언트에 그대로 흘려보낸다. */
    Rio_writen(serverfd, http_header, strlen(http_header));
    forward_response(serverfd, connfd);
    Close(serverfd);
}

/*
 * parse_uri - 절대 URI를 host/path/port로 분해한다.
 * 예: http://example.com:8080/index.html
 *  -> host="example.com", port="8080", path="/index.html"
 * 포트가 생략되면 기본값 80을 사용한다.
 */
int parse_uri(const char *uri, char *host, char *path, char *port)
{
    char uri_copy[MAXLINE];
    char *host_begin;
    char *path_begin;
    char *port_begin;

    strcpy(uri_copy, uri);

    /* "http://"가 있으면 건너뛰고 실제 호스트 부분부터 본다. */
    host_begin = strstr(uri_copy, "//");
    host_begin = host_begin ? host_begin + 2 : uri_copy;

    /* 첫 '/' 이후는 경로, 그 이전은 host[:port] 영역이다. */
    path_begin = strchr(host_begin, '/');
    if (path_begin)
    {
        strcpy(path, path_begin);
        *path_begin = '\0';
    }
    else
    {
        strcpy(path, "/");
    }

    /* host에 ':port'가 포함되면 포트를 분리하고, 아니면 80을 사용한다. */
    port_begin = strchr(host_begin, ':');
    if (port_begin)
    {
        *port_begin = '\0';
        strcpy(host, host_begin);
        strcpy(port, port_begin + 1);
    }
    else
    {
        strcpy(host, host_begin);
        strcpy(port, "80");
    }

    return 0;
}

/*
 * build_http_header - 프록시가 원 서버로 보낼 요청 헤더 생성
 * 요청 라인은 "GET path HTTP/1.0"으로 고정하고,
 * Host / User-Agent / Connection / Proxy-Connection 헤더를 표준 형태로 맞춘다.
 * 그 외 클라이언트 헤더 중 의미 있는 것은 그대로 덧붙인다.
 */
void build_http_header(char *http_header, const char *host, const char *path,
                       rio_t *client_rio)
{
    int len;
    char buf[MAXLINE], request_hdr[MAXLINE], other_hdr[MAXBUF], host_hdr[MAXLINE];

    /* 요청 라인과 기본 Host 헤더를 준비한다. */
    snprintf(request_hdr, sizeof(request_hdr), "GET %s HTTP/1.0\r\n", path);
    snprintf(host_hdr, sizeof(host_hdr), "Host: %s\r\n", host);
    other_hdr[0] = '\0';

    /*
     * 빈 줄을 만날 때까지 클라이언트 요청 헤더를 읽는다.
     * Host는 따로 보관하고, Connection/Proxy-Connection/User-Agent는
     * 프록시가 직접 덮어쓸 것이므로 건너뛴다.
     */
    while (Rio_readlineb(client_rio, buf, MAXLINE) > 0)
    {
        if (!strcmp(buf, "\r\n"))
        {
            break;
        }

        if (!strncasecmp(buf, "Host:", 5))
        {
            strcpy(host_hdr, buf);
        }
        else if (strncasecmp(buf, "Connection:", 11) &&
                 strncasecmp(buf, "Proxy-Connection:", 17) &&
                 strncasecmp(buf, "User-Agent:", 11))
        {
            strcat(other_hdr, buf);
        }
    }

    /* 최종적으로 원 서버에 보낼 요청 헤더를 한 덩어리로 조립한다. */
    len = snprintf(http_header, MAXBUF, "%s", request_hdr);
    len += snprintf(http_header + len, MAXBUF - len, "%s", host_hdr);
    len += snprintf(http_header + len, MAXBUF - len, "%s", user_agent_hdr);
    len += snprintf(http_header + len, MAXBUF - len, "Connection: close\r\n");
    len += snprintf(http_header + len, MAXBUF - len, "Proxy-Connection: close\r\n");
    len += snprintf(http_header + len, MAXBUF - len, "%s", other_hdr);
    snprintf(http_header + len, MAXBUF - len, "\r\n");
}

/*
 * forward_response - 원 서버 응답을 클라이언트로 복사한다.
 * 프록시는 응답 본문을 해석하지 않고, 읽은 바이트를 그대로 전달한다.
 */
void forward_response(int serverfd, int connfd)
{
    size_t n;
    char buf[MAXBUF];
    rio_t server_rio;

    Rio_readinitb(&server_rio, serverfd);
    while ((n = Rio_readnb(&server_rio, buf, MAXBUF)) > 0)
    {
        Rio_writen(connfd, buf, n);
    }
}

/*
 * clienterror - 프록시가 감지한 오류를 간단한 HTML 응답으로 돌려준다.
 */
void clienterror(int fd, const char *cause, const char *errnum,
                 const char *shortmsg, const char *longmsg)
{
    char buf[MAXLINE], body[MAXBUF];

    snprintf(body, sizeof(body),
             "<html><title>Proxy Error</title>"
             "<body bgcolor=\"ffffff\">\r\n"
             "%s: %s\r\n"
             "<p>%s: %s\r\n"
             "<hr><em>The Proxy Web server</em>\r\n",
             errnum, shortmsg, longmsg, cause);

    snprintf(buf, sizeof(buf), "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
    Rio_writen(fd, buf, strlen(buf));
    snprintf(buf, sizeof(buf), "Content-type: text/html\r\n");
    Rio_writen(fd, buf, strlen(buf));
    snprintf(buf, sizeof(buf), "Content-length: %d\r\n\r\n", (int)strlen(body));
    Rio_writen(fd, buf, strlen(buf));
    Rio_writen(fd, body, strlen(body));
}
