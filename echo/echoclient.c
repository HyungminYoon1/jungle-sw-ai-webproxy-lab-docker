#include "csapp.h"

int main(int argc, char **argv)
{
    int clientfd;
    char *host, *port, buf[MAXLINE];
    rio_t rio;

    /* 실행 인자로 서버 호스트와 포트를 받습니다. */
    if (argc != 3)
    {
        fprintf(stderr, "usage: %s <host> <port>\n", argv[0]); // argv[0] = "./echoclient"
        exit(0);
    }
    host = argv[1]; // argv[1] = "localhost"
    port = argv[2]; // argv[2] = "5000"

    /* 서버에 연결한 뒤 읽기 버퍼를 초기화합니다. */
    clientfd = Open_clientfd(host, port);
    Rio_readinitb(&rio, clientfd);

    /* 표준 입력의 한 줄을 서버로 보내고, echo 응답을 다시 출력합니다. */
    while (Fgets(buf, MAXLINE, stdin) != NULL)
    {
        /*
        * 사용자가 입력한 한 줄을 서버에 그대로 전송합니다.
        * 이때 줄바꿈 문자도 함께 전송되므로 서버는 한 줄 단위로 읽을 수 있습니다.
        */
        Rio_writen(clientfd, buf, strlen(buf));

        /*
        * 서버가 다시 돌려준 echo 응답을 한 줄 읽어 같은 버퍼에 저장합니다.
        * echo 서버이므로 정상 동작 시 방금 보낸 내용과 같은 문자열이 들어옵니다.
        */
        Rio_readlineb(&rio, buf, MAXLINE);

        /* 서버가 돌려준 응답을 표준 출력으로 보여줍니다. */
        Fputs(buf, stdout);
    }

    Close(clientfd);
    exit(0);
}
