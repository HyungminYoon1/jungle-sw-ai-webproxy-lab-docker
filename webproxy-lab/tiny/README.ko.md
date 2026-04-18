# Tiny 웹 서버

Dave O'Hallaron  
Carnegie Mellon University

이 디렉터리는 Tiny 서버의 홈 디렉터리입니다. Tiny는 Carnegie Mellon University의  
`15-213: Intro to Computer Systems` 수업에서 사용하는 약 200줄 규모의 웹 서버입니다.

Tiny는 `GET` 메서드를 사용하여 다음 두 종류의 콘텐츠를 제공합니다.

- 정적 콘텐츠: 현재 디렉터리(`./`) 아래의 텍스트, HTML, GIF, JPG 파일
- 동적 콘텐츠: `./cgi-bin` 아래의 CGI 프로그램 실행 결과

기본 페이지는 `index.html`이 아니라 `home.html`입니다.  
이는 브라우저에서 디렉터리 내용을 함께 확인할 수 있도록 하기 위함입니다.

Tiny는 보안적으로 완전하지도 않고 기능이 충분히 완성된 서버도 아니지만,  
학생들이 실제 웹 서버가 어떻게 동작하는지 감을 잡을 수 있도록 만든 교육용 예제입니다.  
학습 목적에만 사용하세요.

이 코드는 Linux 2.2.20 커널 환경에서 `gcc 2.95.3`로 컴파일 및 실행되도록 작성되었습니다.

## Tiny 설치

깨끗한 디렉터리에서 아래 명령을 실행합니다.

```bash
tar xvf tiny.tar
```

## Tiny 실행

서버 머신에서 아래처럼 실행합니다.

```bash
tiny <port>
```

예:

```bash
tiny 8000
```

브라우저에서 Tiny에 접속하는 예시는 다음과 같습니다.

- 정적 콘텐츠: `http://<host>:8000`
- 동적 콘텐츠: `http://<host>:8000/cgi-bin/adder?1&2`

## 파일 구성

- `tiny.tar`: 이 디렉터리 전체를 담은 아카이브
- `tiny.c`: Tiny 서버 본체
- `Makefile`: `tiny.c`를 빌드하는 메이크파일
- `home.html`: 테스트용 HTML 페이지
- `godzilla.gif`: `home.html`에 포함된 이미지 파일
- `README`: 원문 안내 문서
- `cgi-bin/adder.c`: 두 수를 더하는 CGI 프로그램
- `cgi-bin/Makefile`: `adder.c`를 빌드하는 메이크파일
