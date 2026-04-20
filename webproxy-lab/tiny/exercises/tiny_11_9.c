/* $begin tinymain */
/*
 * tiny.c - GET 메서드를 사용해 정적 및 동적 콘텐츠를 제공하는
 *     단순한 반복형 HTTP/1.0 웹 서버
 *
 * 2019/11 droh 수정
 *   - serve_static()와 clienterror()의 sprintf() 별칭 문제 수정
 */
#include "csapp.h"

void doit(int fd);
void read_requesthdrs(rio_t *rp);
int parse_uri(char *uri, char *filename, char *cgiargs);
void serve_static(int fd, char *filename, int filesize);
void get_filetype(char *filename, char *filetype);
void serve_dynamic(int fd, char *filename, char *cgiargs);
void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg);

/*
 * 반복실행 서버로 명령줄에서 넘겨받은 포트로의 연결 요청을 듣는다.
 * open_listenfd 함수를 호출해서 듣기 소켓을 오픈한 후에 무한 서버 로프를 실행하고, 반복적으로 연결 요청을 접수, 트랜잭션을 수행, 자신 쪽의 연결 끝을 닫는다.
 */
int main(int argc, char **argv)
{
    int listenfd, connfd;
    char hostname[MAXLINE], port[MAXLINE];
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;

    /* 명령행 인자 확인 */
    if (argc != 2)
    {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(1);
    }

    listenfd = Open_listenfd(argv[1]);
    while (1)
    {
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen); // 교재 라인 참조: netp:tiny:accept
        Getnameinfo((SA *) &clientaddr, clientlen, hostname, MAXLINE, 
                    port, MAXLINE, 0);
        printf("Accepted connection from (%s, %s)\n", hostname, port);
        doit(connfd);  // 교재 라인 참조: netp:tiny:doit
        Close(connfd); // 교재 라인 참조: netp:tiny:close
    }
}

/*
 * 요청 한 건을 처리하는 큰 뼈대: 한 개의 HTTP 트랜잭션을 처리한다.
 * 요청 라인 읽기 -> GET 검사 -> 헤더 읽기 -> URI 파싱 -> 정적/동적 분기 -> 에러 처리
 */
void doit(int fd)
{
    int is_static;
    struct stat sbuf;
    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
    char filename[MAXLINE], cgiargs[MAXLINE];
    rio_t rio;

    /* 요청 라인을 읽고 메서드, URI, HTTP 버전을 파싱한다. */
    Rio_readinitb(&rio, fd);
    Rio_readlineb(&rio, buf, MAXLINE);
    printf("Request headers:\n");
    printf("%s", buf);
    sscanf(buf, "%s %s %s", method, uri, version);

    /* Tiny는 GET 메서드만 처리한다. */
    if (strcasecmp(method, "GET")) {
        clienterror(fd, method, "501", "Not implemented",
                    "Tiny does not implement this method");
        return;
    }

    read_requesthdrs(&rio);

    /* URI를 파일 이름으로 바꾸고, 정적/동적 콘텐츠 여부를 결정한다. */
    is_static = parse_uri(uri, filename, cgiargs);
    if (stat(filename, &sbuf) < 0) {
        clienterror(fd, filename, "404", "Not found",
                    "Tiny couldn't find this file");
        return;
    }

    if (is_static) { // 정적 컨텐츠 제공
        /* 일반 파일이면서 읽기 권한이 있어야 정적 파일로 제공할 수 있다. */
        if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode)) {
            clienterror(fd, filename, "403", "Forbidden",
                        "Tiny couldn't read the file");
            return;
        }
        serve_static(fd, filename, sbuf.st_size);
    }
    else { // 동적 컨텐츠 제공
        /* CGI 프로그램은 일반 파일이면서 실행 권한이 있어야 한다. */
        if (!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode)) {
            clienterror(fd, filename, "403", "Forbidden",
                        "Tiny couldn't run the CGI program");
            return;
        }
        serve_dynamic(fd, filename, cgiargs);
    }
}

// 헤더 읽기 (Tiny 서버에서는 요청 헤더 내의 어떤 정보도 사용하지 않음.)
void read_requesthdrs(rio_t *rp)
{
    char buf[MAXLINE];

    Rio_readlineb(rp, buf, MAXLINE);
    while(strcmp(buf, "\r\n")) {
        rio_readlineb(rp, buf, MAXLINE);
        printf("%s", buf);
    }
    return;
}

// 정적 콘텐츠인지 CGI 동적 콘텐츠인지 갈라주는 분기점
int parse_uri(char *uri, char *filename, char *cgiargs)
{
    char *ptr;

    if (!strstr(uri, "cgi-bin")) {
        strcpy(cgiargs, "");
        strcpy(filename, ".");
        strcat(filename, uri);
        if (uri[strlen(uri)-1] == '/')
            strcat(filename, "home.html");
        return 1;
    }
    else {
        ptr = index(uri, '?');
        if (ptr) {
            strcpy(cgiargs, ptr+1);
            *ptr = '\0';
        }
        else
            strcpy(cgiargs, "");
        strcpy(filename, ".");
        strcat(filename, uri);
        return 0;
    }
}

// 오류 응답 처리
void clienterror(int fd, char *cause, char *errnum,
                char *shortmsg, char *longmsg)
{
    char buf[MAXLINE], body[MAXBUF];

    // HTTP 응답 body 빌드
    snprintf(body, sizeof(body),
             "<html><title>Tiny Error</title>"
             "<body bgcolor=\"ffffff\">\r\n"
             "%s: %s\r\n"
             "<p>%s: %s\r\n"
             "<hr><em>The Tiny Web server</em>\r\n",
             errnum, shortmsg, longmsg, cause);

    // HTTP 응답 프린트
    sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-type: text/html\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-length: %d\r\n\r\n", (int)strlen(body));
    Rio_writen(fd, buf, strlen(buf));
    Rio_writen(fd, body, strlen(body));
}

// 정적 파일 제공
void serve_static(int fd, char *filename, int filesize)
{
    int srcfd;
    int len;
    char *srcp, filetype[MAXLINE], buf[MAXBUF];

    // 응답 헤더를 클라이언트에게 전송
    get_filetype(filename, filetype);
    len = snprintf(buf, sizeof(buf), "HTTP/1.0 200 OK\r\n");
    len += snprintf(buf + len, sizeof(buf) - len,
                    "Server: Tiny Web Server\r\n");
    len += snprintf(buf + len, sizeof(buf) - len, "Connection: close\r\n");
    len += snprintf(buf + len, sizeof(buf) - len,
                    "Content-length: %d\r\n", filesize);
    len += snprintf(buf + len, sizeof(buf) - len,
                    "Content-type: %s\r\n\r\n", filetype);
    Rio_writen(fd, buf, strlen(buf));
    printf("Response headers:\n");
    printf("%s", buf);

    // 클라이언트에게 응답 body를 전송
    srcfd = Open(filename, O_RDONLY, 0);
    srcp = Malloc(filesize);
    Rio_readn(srcfd, srcp, filesize);
    Close(srcfd);
    Rio_writen(fd, srcp, filesize);
    Free(srcp);
}

// 파일 이름으로부터 파일 타입을 얻음
void get_filetype(char *filename, char *filetype)
{
    if (strstr(filename, ".html"))
        strcpy(filetype, "text/html");
    else if (strstr(filename, ".gif"))
        strcpy(filetype, "image/gif");
    else if (strstr(filename, ".png"))
        strcpy(filetype, "image/png");
    else if (strstr(filename, ".jpg"))
        strcpy(filetype, "image/jpg");
    else if (strstr(filename, ".mpg")) // 숙제 문제 11.7에서 확장
        strcpy(filetype, "video/mpeg");
    else
        strcpy(filetype, "text/plain");
}

// 동적 컨텐츠 제공
void serve_dynamic(int fd, char *filename, char *cgiargs)
{
    char buf[MAXLINE], *emptylist[] = { NULL };

    // HTTP 응답의 첫 번째 부분을 반환
    sprintf(buf, "HTTP/1.0 200 Ok\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Server: Tiny Web Server\r\n");
    Rio_writen(fd, buf, strlen(buf));

    if (Fork() == 0) { // 자식
        // 실제 서버는 모든 CGI 변수를 여기에 세팅
        setenv("QUERY_STRING", cgiargs, 1);
        Dup2(fd, STDOUT_FILENO); // stdout 을 클라이언트에게 리다이렉트한다.
        Execve(filename, emptylist, environ); // CGI 프로그램을 실행한다.
    }
    wait(NULL); // 부모 프로세스가 자식 프로세스가 끝나기를 기다린다. - 자식 프로세스가 종료돼도 부모가 아직 종료 상태를 회수하지 않으면 zombie process가 남기 때문
}
