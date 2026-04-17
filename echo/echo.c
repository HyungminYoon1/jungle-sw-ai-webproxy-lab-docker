#include "csapp.h"

void echo(int connfd)
{
    size_t n;
    char buf[MAXLINE];
    rio_t rio;

    /* 연결된 소켓을 rio 버퍼와 연결해 줄 단위 입력을 읽을 준비를 합니다. */
    Rio_readinitb(&rio, connfd);

    /*
     * 클라이언트가 보낸 데이터를 한 줄씩 읽습니다.
     * 읽은 줄이 있으면 그대로 다시 클라이언트에 돌려보내며,
     * 더 이상 읽을 데이터가 없으면 반복을 종료합니다.
     */
    while ((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0)
    {        
        // printf("server received %zu bytes\n", n);
        printf("server received %zu bytes: %s", n, buf); /* 현재는 학습용으로 서버가 받은 바이트 수와 내용을 함께 출력합니다. */

        /* 방금 읽은 데이터를 그대로 클라이언트에 다시 전송합니다. */
        Rio_writen(connfd, buf, n);
    }
}
