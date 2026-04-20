/* $begin tinymain */
/*
 * tiny.c - GET 메서드를 사용해 정적 및 동적 콘텐츠를 제공하는
 *     단순한 반복형 HTTP/1.0 웹 서버
 *
 * 2019/11 droh 수정
 *   - serve_static()와 clienterror()의 sprintf() 별칭 문제 수정
 */
#include "csapp.h"

void doit(int fd); // 클라이언트 요청 한 건을 끝까지 처리합니다.
void read_requesthdrs(rio_t *rp, char *reqbuf); // 요청 헤더를 읽어 응답용 버퍼 뒤에 이어 붙입니다.
int parse_uri(char *uri, char *filename, char *cgiargs); // URI를 파일 경로와 CGI 인자로 해석합니다.
void serve_static(int fd, char *filename, int filesize); // 정적 파일을 HTTP 응답으로 전송합니다.
void get_filetype(char *filename, char *filetype); // 파일 확장자에 맞는 MIME 타입을 구합니다.
void serve_dynamic(int fd, char *filename, char *cgiargs); // CGI 프로그램을 실행해 동적 응답을 만듭니다.
void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg); // 오류 상황을 HTTP 에러 응답으로 돌려줍니다.

/*
 * 반복실행 서버로 명령줄에서 넘겨받은 포트로의 연결 요청을 듣는다.
 * open_listenfd 함수를 호출해서 듣기 소켓을 오픈한 후에 무한 서버 로프를 실행하고, 반복적으로 연결 요청을 접수, 트랜잭션을 수행, 자신 쪽의 연결 끝을 닫는다.
 */
int main(int argc, char **argv)
{
    int listenfd, connfd; // listenfd는 리스닝 소켓, connfd는 연결된 클라이언트 소켓입니다.
    char hostname[MAXLINE], port[MAXLINE]; // 접속한 클라이언트의 호스트/포트 문자열을 담습니다.
    socklen_t clientlen; // 주소 구조체의 실제 크기를 커널과 주고받을 때 사용합니다.
    struct sockaddr_storage clientaddr; // IPv4/IPv6 모두 담을 수 있는 범용 주소 저장 공간입니다.

    /* 명령행 인자 확인 */
    if (argc != 2) // tiny는 실행 인자로 포트 번호 하나만 받아야 합니다.
    {
        fprintf(stderr, "usage: %s <port>\n", argv[0]); // 잘못 실행했을 때 올바른 사용 형식을 알려줍니다.
        exit(1); // 인자가 틀리면 서버를 시작할 수 없으므로 종료합니다.
    }

    listenfd = Open_listenfd(argv[1]); // 지정 포트에서 연결을 받을 리스닝 소켓을 엽니다.
    while (1) // 반복형 서버이므로 연결을 하나 처리할 때마다 다시 대기합니다.
    {
        clientlen = sizeof(clientaddr); // Accept 전에 주소 버퍼 크기를 알려줍니다.
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen); // 새 클라이언트 연결을 수락하고 통신용 소켓을 얻습니다.
        Getnameinfo((SA *)&clientaddr, clientlen, hostname, MAXLINE, // 수락한 주소를 사람이 읽기 쉬운 문자열로 바꿉니다.
                    port, MAXLINE, 0);
        printf("Accepted connection from (%s, %s)\n", hostname, port); // 어떤 클라이언트가 접속했는지 로그로 남깁니다.
        doit(connfd); // 실제 HTTP 요청 처리는 doit가 맡습니다.
        Close(connfd); // 이 요청 처리가 끝났으므로 연결 소켓을 닫습니다.
    }
}

/*
 * 요청 한 건을 처리하는 큰 뼈대: 한 개의 HTTP 트랜잭션을 처리한다.
 * 요청 라인 읽기 -> GET 검사 -> 헤더 읽기 -> URI 파싱 -> 정적/동적 분기 -> 에러 처리
 */
void doit(int fd)
{
    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE]; // 요청 라인과 그 안의 토큰들을 저장합니다.
    char reqbuf[MAXBUF], header[MAXBUF]; // reqbuf는 에코할 요청 문자열, header는 HTTP 응답 헤더입니다.
    rio_t rio; // Robust I/O용 버퍼 상태 구조체입니다.

    /* 11.6A용: 요청 라인과 헤더를 그대로 클라이언트에 돌려준다. */
    Rio_readinitb(&rio, fd); // 이 연결 소켓을 rio 버퍼드 읽기 상태에 연결합니다.
    Rio_readlineb(&rio, buf, MAXLINE); // 요청 라인의 첫 줄을 읽습니다. 예: GET / HTTP/1.1
    sscanf(buf, "%s %s %s", method, uri, version); // 요청 라인을 메서드/URI/버전 세 부분으로 나눕니다.
    strcpy(reqbuf, buf); // 요청 라인을 응답용 버퍼의 시작으로 복사합니다.

    read_requesthdrs(&rio, reqbuf); // 나머지 헤더들을 reqbuf 뒤에 이어 붙입니다.

    snprintf(header, sizeof(header),
             "HTTP/1.0 200 OK\r\n"
             "Content-type: text/plain\r\n"
             "Content-length: %zu\r\n\r\n",
             strlen(reqbuf)); // 요청 문자열 길이에 맞는 단순 text/plain 응답 헤더를 만듭니다.
    Rio_writen(fd, header, strlen(header)); // 응답 헤더를 먼저 보냅니다.
    Rio_writen(fd, reqbuf, strlen(reqbuf)); // 요청 라인과 헤더 전체를 본문으로 돌려줍니다.
}

// 헤더 읽기 (Tiny 서버에서는 요청 헤더 내의 어떤 정보도 사용하지 않음.)
void read_requesthdrs(rio_t *rp, char *reqbuf)
{
    char buf[MAXLINE]; // 헤더 한 줄씩 읽어 둘 임시 버퍼입니다.

    Rio_readlineb(rp, buf, MAXLINE); // 요청 라인 다음 첫 헤더 줄을 읽습니다.
    while(strcmp(buf, "\r\n")) { // 빈 줄이 나올 때까지 헤더 구간이 계속된다는 뜻입니다.
        strcat(reqbuf, buf); // 읽은 헤더 한 줄을 응답용 요청 버퍼 뒤에 이어 붙입니다.
        Rio_readlineb(rp, buf, MAXLINE); // 다음 헤더 줄을 읽습니다.
    }
    strcat(reqbuf, buf); // 마지막 빈 줄까지 포함해 원래 요청 모양을 유지합니다.
    return; // 헤더 영역을 모두 소비했으므로 호출한 곳으로 돌아갑니다.
}

// 정적 콘텐츠인지 CGI 동적 콘텐츠인지 갈라주는 분기점
int parse_uri(char *uri, char *filename, char *cgiargs)
{
    char *ptr; // '?' 위치를 가리켜 CGI 인자를 분리할 때 사용합니다.

    if (!strstr(uri, "cgi-bin")) { // URI에 cgi-bin이 없으면 정적 파일 요청으로 봅니다.
        strcpy(cgiargs, ""); // 정적 파일은 CGI 인자가 없으므로 빈 문자열로 둡니다.
        strcpy(filename, "."); // 현재 디렉터리를 기준 경로로 시작합니다.
        strcat(filename, uri); // 요청 URI를 뒤에 붙여 실제 파일 경로를 만듭니다.
        if (uri[strlen(uri)-1] == '/') // URI가 '/'로 끝나면 디렉터리 요청이라는 뜻입니다.
            strcat(filename, "home.html"); // Tiny의 기본 페이지는 home.html입니다.
        return 1; // 정적 콘텐츠임을 호출자에게 알립니다.
    }
    else { // cgi-bin이 들어 있으면 CGI 실행 요청으로 처리합니다.
        ptr = index(uri, '?'); // '?'를 찾아 프로그램 경로와 인자 부분을 나눕니다.
        if (ptr) { // 인자가 실제로 있는 경우
            strcpy(cgiargs, ptr+1); // '?' 뒤 문자열을 CGI 인자로 저장합니다.
            *ptr = '\0'; // URI를 여기서 끊어 실행 파일 경로만 남깁니다.
        }
        else
            strcpy(cgiargs, ""); // 인자가 없으면 빈 문자열로 둡니다.
        strcpy(filename, "."); // 현재 디렉터리 기준으로 실행 파일 경로를 만듭니다.
        strcat(filename, uri); // /cgi-bin/adder 같은 경로를 붙입니다.
        return 0; // 동적 콘텐츠임을 호출자에게 알립니다.
    }
}

// 오류 응답 처리
void clienterror(int fd, char *cause, char *errnum,
                char *shortmsg, char *longmsg)
{
    char buf[MAXLINE], body[MAXBUF]; // buf는 헤더, body는 HTML 본문을 만드는 데 사용합니다.

    // HTTP 응답 body 빌드
    snprintf(body, sizeof(body), // 오류 설명이 담긴 간단한 HTML 본문을 한 번에 만듭니다.
             "<html><title>Tiny Error</title>"
             "<body bgcolor=\"ffffff\">\r\n"
             "%s: %s\r\n"
             "<p>%s: %s\r\n"
             "<hr><em>The Tiny Web server</em>\r\n",
             errnum, shortmsg, longmsg, cause);

    // HTTP 응답 프린트
    sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg); // 상태줄을 만들어 먼저 보냅니다.
    Rio_writen(fd, buf, strlen(buf)); // 상태줄을 소켓으로 전송합니다.
    sprintf(buf, "Content-type: text/html\r\n"); // 본문이 HTML이라는 헤더를 보냅니다.
    Rio_writen(fd, buf, strlen(buf)); // 헤더 줄을 전송합니다.
    sprintf(buf, "Content-length: %d\r\n\r\n", (int)strlen(body)); // 본문 길이와 헤더 끝의 빈 줄을 전송합니다.
    Rio_writen(fd, buf, strlen(buf)); // Content-length 헤더를 전송합니다.
    Rio_writen(fd, body, strlen(body)); // 마지막으로 실제 HTML 오류 본문을 보냅니다.
}

// 정적 파일 제공
void serve_static(int fd, char *filename, int filesize)
{
    int srcfd; // 정적 파일을 읽기 위해 열 파일 디스크립터입니다.
    int len; // 응답 헤더를 버퍼에 이어 붙일 때 현재 길이를 추적합니다.
    char *srcp, filetype[MAXLINE], buf[MAXBUF]; // srcp는 mmap 결과 주소, filetype은 MIME 타입, buf는 응답 헤더 버퍼입니다.

    // 응답 헤더를 클라이언트에게 전송
    get_filetype(filename, filetype); // 파일 이름을 보고 브라우저가 이해할 MIME 타입을 정합니다.
    len = snprintf(buf, sizeof(buf), "HTTP/1.0 200 OK\r\n"); // 상태줄을 먼저 버퍼에 기록합니다.
    len += snprintf(buf + len, sizeof(buf) - len, // 서버 식별 헤더를 이어 붙입니다.
                    "Server: Tiny Web Server\r\n");
    len += snprintf(buf + len, sizeof(buf) - len, "Connection: close\r\n"); // 응답 후 연결을 닫겠다고 알립니다.
    len += snprintf(buf + len, sizeof(buf) - len, // 본문 길이를 Content-length에 적습니다.
                    "Content-length: %d\r\n", filesize);
    len += snprintf(buf + len, sizeof(buf) - len, // MIME 타입과 헤더 끝의 빈 줄을 붙입니다.
                    "Content-type: %s\r\n\r\n", filetype);
    Rio_writen(fd, buf, strlen(buf)); // 조립된 응답 헤더 전체를 한 번에 전송합니다.
    printf("Response headers:\n"); // 학습용으로 서버가 만든 응답 헤더를 출력합니다.
    printf("%s", buf); // 실제 전송된 헤더를 그대로 보여줍니다.

    // 클라이언트에게 응답 body를 전송
    srcfd = Open(filename, O_RDONLY, 0); // 정적 파일을 읽기 전용으로 엽니다.
    srcp = Mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0); // 파일 내용을 메모리 영역에 매핑해 복사 없이 접근합니다.
    Close(srcfd); // 매핑이 끝나면 파일 디스크립터는 더 이상 필요 없습니다.
    Rio_writen(fd, srcp, filesize); // 매핑된 파일 내용을 그대로 클라이언트에 씁니다.
    Munmap(srcp, filesize); // 사용이 끝난 매핑 영역을 해제합니다.
}

// 파일 이름으로부터 파일 타입을 얻음
void get_filetype(char *filename, char *filetype)
{
    if (strstr(filename, ".html")) // HTML 파일이면 브라우저가 문서로 렌더링하게 합니다.
        strcpy(filetype, "text/html");
    else if (strstr(filename, ".gif")) // GIF 이미지 MIME 타입입니다.
        strcpy(filetype, "image/gif");
    else if (strstr(filename, ".png")) // PNG 이미지 MIME 타입입니다.
        strcpy(filetype, "image/png");
    else if (strstr(filename, ".jpg")) // JPG 이미지 MIME 타입입니다.
        strcpy(filetype, "image/jpg");
    else
        strcpy(filetype, "text/plain"); // 그 외 파일은 평문으로 취급합니다.
}

// 동적 컨텐츠 제공
void serve_dynamic(int fd, char *filename, char *cgiargs)
{
    char buf[MAXLINE], *emptylist[] = { NULL }; // emptylist는 CGI 프로그램에 넘길 argv 배열입니다.

    // HTTP 응답의 첫 번째 부분을 반환
    sprintf(buf, "HTTP/1.0 200 Ok\r\n"); // 상태줄을 먼저 클라이언트에게 보냅니다.
    Rio_writen(fd, buf, strlen(buf)); // 상태줄 전송
    sprintf(buf, "Server: Tiny Web Server\r\n"); // 서버 이름 헤더를 보냅니다.
    Rio_writen(fd, buf, strlen(buf)); // 헤더 전송

    if (Fork() == 0) { // 자식 프로세스가 실제 CGI 프로그램을 실행합니다.
        // 실제 서버는 모든 CGI 변수를 여기에 세팅
        setenv("QUERY_STRING", cgiargs, 1); // adder 같은 CGI 프로그램이 환경변수로 인자를 읽게 합니다.
        Dup2(fd, STDOUT_FILENO); // 자식의 표준 출력을 클라이언트 소켓으로 바꿉니다.
        Execve(filename, emptylist, environ); // CGI 실행 파일로 프로세스 이미지를 교체합니다.
    }
    wait(NULL); // 부모는 자식 종료를 회수해 좀비 프로세스가 남지 않게 합니다.
}
