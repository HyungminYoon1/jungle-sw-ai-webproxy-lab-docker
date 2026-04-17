#include "csapp.h"

void echo(int connfd);

int main(int argc, char **argv)
{
    int listenfd, connfd;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    char client_hostname[MAXLINE], client_port[MAXLINE];

    /* 서버 실행 시 포트 번호 하나를 인자로 받습니다. */
    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(0);
    }

    /*
    * 지정한 포트에서 연결 요청을 받을 리스닝 소켓을 생성합니다.
    * 이후부터 서버는 이 소켓을 통해 클라이언트 접속을 기다립니다.
    */
    listenfd = Open_listenfd(argv[1]);

    /*
    * 반복형(iterative) 서버이므로 한 번에 하나의 연결만 처리합니다.
    * 연결을 수락하고 echo()로 요청을 처리한 뒤, 소켓을 닫고
    * 다시 다음 연결을 기다리는 구조입니다.
    */
    while (1) {
        /* 클라이언트 주소 정보를 저장할 구조체 크기를 설정합니다. */
        clientlen = sizeof(struct sockaddr_storage);

        /*
        * 클라이언트의 연결 요청을 수락하면, 통신 전용 연결 소켓 connfd가 생성됩니다.
        * listenfd는 계속 다음 연결을 받기 위해 유지되고,
        * 실제 데이터 송수신은 connfd로 수행합니다.
        */
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);

        /*
        * 방금 연결한 클라이언트의 호스트 이름과 포트 번호를 사람이 읽기 쉬운
        * 문자열 형태로 변환합니다. 로그 출력에 사용합니다.
        */
        Getnameinfo((SA *)&clientaddr, clientlen, client_hostname, MAXLINE,
                    client_port, MAXLINE, 0);
        printf("connected to (%s, %s)\n", client_hostname, client_port);

        /* 연결된 클라이언트와 echo 통신을 수행합니다. */
        echo(connfd);

        /* 해당 클라이언트와의 통신이 끝났으므로 연결 소켓을 닫습니다. */
        Close(connfd);
    }
}
