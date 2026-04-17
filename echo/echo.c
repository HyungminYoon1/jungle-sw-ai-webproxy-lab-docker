#include <stdio.h>

#include "csapp.h"
#include "echo.h"

void echo(int connfd)
{
  size_t n;
  char buf[MAXLINE];
  rio_t rio;

  Rio_readinitb(&rio, connfd);
  while ((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0)
  {
    // printf("server received %zu bytes\n", n);
    printf("server received %zu bytes: %s", n, buf);
    Rio_writen(connfd, buf, n);
  }
}
