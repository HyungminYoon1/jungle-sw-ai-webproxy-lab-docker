/*
 * adder_11_12.c - POST form용 CGI 덧셈 프로그램
 */
#include "csapp.h"

int main(void)
{
    char *lenstr, *p, *v1, *v2;
    char buf[MAXLINE], body[MAXLINE], arg1[MAXLINE], arg2[MAXLINE], content[MAXLINE];
    int n1 = 0, n2 = 0, content_length = 0;
    ssize_t nread;

    lenstr = getenv("CONTENT_LENGTH");
    if (lenstr != NULL)
        content_length = atoi(lenstr);

    if (content_length > 0 && content_length < MAXLINE) {
        nread = read(STDIN_FILENO, buf, content_length);
        if (nread < 0)
            exit(1);
        buf[nread] = '\0';
        strcpy(body, buf);

        p = strchr(buf, '&');
        if (p != NULL) {
            *p = '\0';
            strcpy(arg1, buf);
            strcpy(arg2, p + 1);

            v1 = strchr(arg1, '=');
            v2 = strchr(arg2, '=');
            if (v1 != NULL)
                n1 = atoi(v1 + 1);
            if (v2 != NULL)
                n2 = atoi(v2 + 1);
        }
    } else {
        strcpy(buf, "");
        strcpy(body, "");
    }

    sprintf(content, "POST_BODY=%s\r\n<p>", body);
    sprintf(content + strlen(content), "Welcome to add.com: ");
    sprintf(content + strlen(content), "THE Internet addition portal.\r\n<p>");
    sprintf(content + strlen(content), "The answer is: %d + %d = %d\r\n<p>",
            n1, n2, n1 + n2);
    sprintf(content + strlen(content), "Thanks for visiting!\r\n");

    printf("Connection: close\r\n");
    printf("Content-length: %d\r\n", (int)strlen(content));
    printf("Content-type: text/html\r\n");
    printf("\r\n");
    printf("%s", content);
    fflush(stdout);

    exit(0);
}
