#include <stdio.h>
#include <string.h>

#include "csapp.h"

int main(int argc, char **argv)
{
  int clientfd;
  char buf[MAXLINE];
  rio_t rio;

  if (argc != 3)
  {
    fprintf(stderr, "usage: %s <host> <port>\n", argv[0]);
    return 1;
  }

  clientfd = Open_clientfd(argv[1], argv[2]);
  Rio_readinitb(&rio, clientfd);

  while (Fgets(buf, MAXLINE, stdin) != NULL)
  {
    Rio_writen(clientfd, buf, strlen(buf));
    if (Rio_readlineb(&rio, buf, MAXLINE) == 0)
    {
      break;
    }
    Fputs(buf, stdout);
  }

  Close(clientfd);
  return 0;
}
