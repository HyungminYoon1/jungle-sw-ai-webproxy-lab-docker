####################################################################
# CS:APP 프록시 실습
#
# 학생용 소스 파일
####################################################################

이 디렉터리에는 CS:APP Proxy Lab을 수행하는 데 필요한 파일들이 들어 있습니다.

proxy.c
csapp.h
csapp.c
    이 파일들은 시작용 파일입니다. `csapp.c`와 `csapp.h`는
    교재에 설명되어 있습니다.

    이 파일들은 자유롭게 수정해도 됩니다.
    또한 제출에 필요한 추가 파일을 만들어 함께 제출해도 됩니다.

    프록시 또는 tiny 서버에 사용할 고유 포트 번호를 만들려면
    `port-for-user.pl` 또는 `free-port.sh`를 사용하세요.

Makefile
    프록시 프로그램을 빌드하는 Makefile입니다.
    해답을 빌드하려면 `make`를 실행하고,
    새로 빌드하려면 `make clean` 후 `make`를 실행하세요.

    제출할 tar 파일을 만들려면 `make handin`을 실행하세요.
    이 Makefile은 자유롭게 수정할 수 있습니다.
    강사는 여러분의 Makefile을 사용해 소스 코드로부터 프록시를 빌드합니다.

port-for-user.pl
    특정 사용자에 대해 임의의 포트를 생성합니다.
    사용법: `./port-for-user.pl <userID>`

free-port.sh
    프록시 또는 tiny에서 사용할 수 있는 비어 있는 TCP 포트를 찾아주는 편리한 스크립트입니다.
    사용법: `./free-port.sh`

driver.sh
    Basic, Concurrency, Cache를 채점하는 자동 채점 스크립트입니다.
    사용법: `./driver.sh`

nop-server.py
    자동 채점을 위한 보조 프로그램입니다.

tiny
    CS:APP 교재에 나오는 Tiny 웹 서버입니다.
