#include <iostream>
#include "redis1.hpp"
using namespace std;

int main(void)
{
    redisContext *c = Redis::RedisConnect("127.0.0.1", 6379);

    redisReply *r = Redis::setValue(c, "aaa", "abc");
    r = Redis::getValue(c, "aaa");
    if (r == NULL)
        printf("GET ");
    if (r->type != REDIS_REPLY_STRING)
    {
        printf("Execut getValue failure\n");
        freeReplyObject(r);
        redisFree(c);
    }
    printf("%s\n", r->str);

    r = Redis::existsValue(c, "aaa");
    if (r == NULL)
        printf("GET ");
    printf("%d", r->integer);
    return 0;
}