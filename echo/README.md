# Echo Server Learning Notes

이 디렉터리는 CSAPP 11장의 `echo` 서버/클라이언트 예제를 학습하고 기록하기 위한 보조 공간입니다.

`webproxy-lab/` 아래의 tiny/proxy 과제 본체와는 구분해서 관리합니다.

## 목적

- 소켓 생성, 연결, 바인드, 리슨, accept 흐름을 직접 익힙니다.
- 클라이언트와 서버가 문자열을 주고받는 가장 작은 형태의 네트워크 프로그램을 구현합니다.
- 이후 tiny 웹 서버와 proxy 서버 구현에 필요한 기초를 단계적으로 확인합니다.

## 현재 구성

- `echoclient.c`: 표준 입력을 서버로 보내고 응답을 출력하는 클라이언트
- `echoserveri.c`: 순차적(iterative) 에코 서버
- `echo.c`, `echo.h`: 공통 에코 처리 함수
- `Makefile`: 학습용 빌드 스크립트

## 빌드 방법

`echo/` 디렉터리에서 아래 명령으로 빌드합니다.

```bash
cd /workspaces/jungle-sw-ai-webproxy-lab-docker/echo
make
```

빌드가 완료되면 다음 실행 파일이 생성됩니다.

- `echoserveri`
- `echoclient`

빌드 산출물을 지우려면 아래 명령을 사용합니다.

```bash
make clean
```

## 실행 방법

1. 먼저 서버를 실행합니다.

```bash
./echoserveri 5000
```

2. 다른 터미널에서 클라이언트를 실행합니다.

```bash
./echoclient localhost 5000
```

3. 클라이언트 터미널에 문자열을 입력하면 서버가 같은 내용을 다시 돌려줍니다.

`echo.c`는 단독 실행 파일이 아니라 서버가 호출하는 공통 함수 파일이므로, 직접 실행하는 대상은 아닙니다.

## 기록하면 좋은 내용

- 서버 시작 순서와 주요 시스템 콜 정리
- 클라이언트 요청과 서버 응답 흐름
- `rio` 기반 입출력과 일반 입출력의 차이
- tiny 구현으로 넘어갈 때 연결되는 개념
- 직접 디버깅하며 확인한 점과 실수한 점

## 메모

- 이 디렉터리는 echo 서버에 대한 학습 예제를 남기기 위한 공간입니다.
- 본 과제 구현은 `webproxy-lab/` 아래에서 진행합니다.
- `webproxy-lab/csapp.c`, `webproxy-lab/csapp.h`를 재사용해 CSAPP 흐름과 연결되도록 구성했습니다.
