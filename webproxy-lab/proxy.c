#include <stdio.h>

/* 권장 최대 캐시 및 객체 크기 */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

/* 이 긴 줄을 코드에 포함해도 스타일 점수에서 불이익은 없습니다 */
static const char *user_agent_hdr =
    "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 "
    "Firefox/10.0.3\r\n";

int main()
{
  printf("%s", user_agent_hdr);
  return 0;
}
