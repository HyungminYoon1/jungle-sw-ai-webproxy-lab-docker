# Web/Proxy Server Assignment

이 저장소는 정글 `Web/Proxy Server` 과제를 수행하기 위한 워크스페이스입니다.  
C 언어로 소켓 통신, HTTP 요청 처리, 정적/동적 콘텐츠 제공, 그리고 프록시 서버의 핵심 동작을 직접 구현하는 것이 목표입니다.

도커 및 Dev Container 환경설정은 `DOCKER_DEVCONTAINER_SETUP.md`를 참고하세요.

## 빠른 시작

VSCode에서 이 저장소를 연 뒤 Dev Container로 다시 열면 과제 수행에 필요한 C 개발 환경을 사용할 수 있습니다.

```bash
cd webproxy-lab/tiny
make

cd ..
make
```

자세한 Docker 및 Dev Container 설정 방법은 `DOCKER_DEVCONTAINER_SETUP.md`에 정리되어 있습니다.

## 과제 목적

이 과제의 핵심은 웹 통신의 흐름을 라이브러리 뒤에 가리지 않고 직접 구현해보는 데 있습니다.

- 소켓 기반 클라이언트/서버 통신을 이해합니다.
- HTTP 요청과 응답의 구조를 실제 코드로 다룹니다.
- 웹 서버가 정적 파일과 CGI 기반 동적 콘텐츠를 어떻게 처리하는지 익힙니다.
- 프록시 서버가 클라이언트와 원 서버 사이에서 어떤 역할을 하는지 구현으로 확인합니다.
- 순차 처리, 병렬 처리, 캐시까지 확장하며 네트워크 프로그램의 구조를 단계적으로 학습합니다.

## 과제 내용

과제는 크게 두 단계로 진행됩니다.

### 1. Tiny 웹 서버 구현

주요 구현 파일은 다음과 같습니다.

- `webproxy-lab/tiny/tiny.c`
- `webproxy-lab/tiny/cgi-bin/adder.c`

CS:APP 11장의 tiny 웹 서버를 바탕으로, HTTP GET 요청을 받아 정적 콘텐츠와 동적 콘텐츠를 처리하는 서버를 완성합니다.

### 2. Proxy 서버 구현

주요 구현 파일은 다음과 같습니다.

- `webproxy-lab/proxy.c`

웹 브라우저의 요청을 받아 원 서버로 전달하고, 응답을 다시 클라이언트에 전달하는 프록시 서버를 구현합니다. 이후에는 동시성 처리와 캐시 기능까지 확장하는 것이 과제의 핵심 도전 과제입니다.

## 학습 및 구현 범위

- 소켓과 HTTP 기본 개념 이해
- tiny 웹 서버 구현
- 숙제 문제 풀이
- proxy 서버 구현
- 병렬 처리와 캐시 기능 검증

## 빌드 및 실행

Tiny 웹 서버는 `webproxy-lab/tiny` 디렉터리에서 빌드하고 실행합니다.

```bash
cd webproxy-lab/tiny
make
./tiny 8000
```

브라우저나 `curl`로 정적/동적 콘텐츠를 확인할 수 있습니다.

```bash
curl http://localhost:8000/
curl "http://localhost:8000/cgi-bin/adder?1&2"
```

Proxy 서버는 `webproxy-lab` 디렉터리에서 빌드하고 실행합니다.

```bash
cd webproxy-lab
make
./proxy 4500
```

포트가 이미 사용 중이면 `webproxy-lab/free-port.sh`로 사용 가능한 포트를 확인하거나, VSCode 디버깅 설정의 포트를 변경합니다.

## 검증

변경 후에는 과제 디렉터리에서 단계별로 빌드와 드라이버를 실행해 확인합니다.

```bash
cd webproxy-lab/tiny
make

cd ..
make
./driver.sh
```

`./driver.sh`는 proxy 서버의 기본 동작, 동시성, 캐시 기능을 검증하는 과제용 드라이버입니다.

## 과제의 의의

이 과제는 단순히 웹 서버를 하나 만드는 실습이 아니라, 네트워크 프로그래밍의 기본기를 직접 손으로 구현하며 체득하는 과정입니다.

- 운영체제와 네트워크가 만나는 지점을 C 코드 수준에서 이해할 수 있습니다.
- 추상화된 프레임워크 없이도 서버가 동작하는 원리를 설명할 수 있게 됩니다.
- 이후 멀티스레드 서버, API 서버, 웹 인프라 학습으로 확장할 수 있는 기반을 다질 수 있습니다.

## 참고 문서

- 프록시 과제 설명: `webproxy-lab/README.md`
- Tiny 웹 서버 설명: `webproxy-lab/tiny/README`
