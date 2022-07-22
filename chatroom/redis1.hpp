#ifndef REDIS1_H
#define REDIS1_H

#include <hiredis/hiredis.h>
#include <string>
#include <cstring>
#include <iostream>
using namespace std;

class Redis
{
public:
    //打开数据库
    static redisContext *RedisConnect(const char *ip, int port)
    {
        redisContext *c = redisConnect(ip, port);
        if (c->err || c == NULL)
        {
            if (c)
            {
                cout << "Error:" << c->errstr << endl;
            }
            else
            {
                cout << "Can't allocate redis context" << endl;
            }
        }
        return c;
    }
    // set
    static redisReply *setValue(redisContext *c, const string &key, const string &value)
    {
        string cmd = "set " + key + " " + value;
        redisReply *r = (redisReply *)redisCommand(c, cmd.c_str());
        return r;
    }
    static redisReply *getValue(redisContext *c, const string &key)
    {
        string cmd = "get " + key;
        redisReply *r = (redisReply *)redisCommand(c, cmd.c_str());
        return r;
    }
    static redisReply *existsValue(redisContext *c, const string &key)
    {
        string cmd = "exists " + key;
        redisReply *r = (redisReply *)redisCommand(c, cmd.c_str());
        return r;
    }
    static redisReply *delKey(redisContext *c, const string &key, const string &value)
    {
        string cmd = "del " + key;
        redisReply *r = (redisReply *)redisCommand(c, cmd.c_str());
        return r;
    }
    static redisReply *hsetValue(redisContext *c, const string &key, const string &field, const string &value)
    {
        string cmd = "hset " + key + " " + field + " " + value;
        redisReply *r = (redisReply *)redisCommand(c, cmd.c_str());
        return r;
    }
    static redisReply *hsetexist(redisContext *c, const string &key, const string &field)
    {
        string cmd = "hexists " + key + " " + field;
        redisReply *r = (redisReply *)redisCommand(c, cmd.c_str());
        return r;
    }
    static redisReply *hgethash(redisContext *c, const string &key, const string &field)
    {
        string cmd = "hget " + key + " " + field;
        redisReply *r = (redisReply *)redisCommand(c, cmd.c_str());
        return r;
    }
    static redisReply *hashdel(redisContext *c, const string &key, const string &field)
    {
        string cmd = "hdel " + key + " " + field;
        redisReply *r = (redisReply *)redisCommand(c, cmd.c_str());
        return r;
    }
};

#endif