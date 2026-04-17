CC = gcc
CPPFLAGS = -I../webproxy-lab
CFLAGS = -g -Wall
LDFLAGS = -lpthread

TARGETS = echoclient echoserveri

all: $(TARGETS)

csapp.o: ../webproxy-lab/csapp.c ../webproxy-lab/csapp.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -c ../webproxy-lab/csapp.c -o csapp.o

echo.o: echo.c echo.h ../webproxy-lab/csapp.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -c echo.c

echoclient.o: echoclient.c ../webproxy-lab/csapp.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -c echoclient.c

echoserveri.o: echoserveri.c echo.h ../webproxy-lab/csapp.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -c echoserveri.c

echoclient: echoclient.o csapp.o
	$(CC) $(CFLAGS) -o echoclient echoclient.o csapp.o $(LDFLAGS)

echoserveri: echoserveri.o echo.o csapp.o
	$(CC) $(CFLAGS) -o echoserveri echoserveri.o echo.o csapp.o $(LDFLAGS)

clean:
	rm -f *.o $(TARGETS)

.PHONY: all clean
