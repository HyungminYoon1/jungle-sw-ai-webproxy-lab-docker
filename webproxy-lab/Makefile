# 프록시 실습용 Makefile
#
# handin 규칙을 제외하고 이 파일은 자유롭게 수정할 수 있습니다.
# 강사는 제출한 Makefile에서 "make"를 실행해
# 소스 코드로부터 proxy를 빌드합니다.

CC = gcc
CFLAGS = -g -Wall
LDFLAGS = -lpthread

all: proxy

csapp.o: csapp.c csapp.h
	$(CC) $(CFLAGS) -c csapp.c

proxy.o: proxy.c csapp.h
	$(CC) $(CFLAGS) -c proxy.c

proxy: proxy.o csapp.o
	$(CC) $(CFLAGS) proxy.o csapp.o -o proxy $(LDFLAGS)

# 제출 가능한 ../proxylab-handin.tar 타르 파일을 생성합니다.
# 이 규칙은 수정하지 마십시오!
handin:
	(make clean; cd ..; tar cvf $(USER)-proxylab-handin.tar proxylab-handout --exclude tiny --exclude nop-server.py --exclude proxy --exclude driver.sh --exclude port-for-user.pl --exclude free-port.sh --exclude ".*")

clean:
	rm -f *~ *.o proxy core *.tar *.zip *.gzip *.bzip *.gz

