/*
 * adder_11_10.c - HTML form용 CGI 덧셈 프로그램
 */
#include "csapp.h"

int main(void)
{
    char *buf, *p, *v1, *v2;
    char arg1[MAXLINE], arg2[MAXLINE], content[MAXLINE];
    int n1 = 0, n2 = 0;

    /* num1=...&num2=... 형식의 쿼리 문자열을 파싱 */
    if ((buf = getenv("QUERY_STRING")) != NULL)
    {
        p = strchr(buf, '&');
        if (p != NULL)
        {
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
    }

    /* 응답 본문 생성 */
    sprintf(content, "QUERY_STRING=%s\r\n<p>", buf);
    sprintf(content + strlen(content), "Welcome to add.com: ");
    sprintf(content + strlen(content), "THE Internet addition portal.\r\n<p>");
    sprintf(content + strlen(content), "The answer is: %d + %d = %d\r\n<p>",
            n1, n2, n1 + n2);
    sprintf(content + strlen(content), "Thanks for visiting!\r\n");

    /* HTTP 응답 생성 */
    printf("Connection: close\r\n");
    printf("Content-length: %d\r\n", (int)strlen(content));
    printf("Content-type: text/html\r\n");
    printf("\r\n");
    printf("%s", content);
    fflush(stdout);

    exit(0);
}
