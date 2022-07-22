/*
g++ -o account account.cc account.hpp ssock.cpp -lhiredis
*/
#ifndef SERV_H
#define SERV_H
#include <iostream>
#include <cstring>
#include <string>
#include <set>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "ssock.hpp"
#include "redis1.hpp"
#include <hiredis/hiredis.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <fcntl.h>
#include "message.hpp"
#include "redis1.hpp"
#include "threadpool.hpp"
using namespace std;

#define OPEN_MAX 5000

void startlogin(void *arg);
void startpchat(void *arg);

class account
{
public:
    account();
    ~account();
    bool login();              //从数据库读数据并与登陆输入的数据比较
    void reg();                //注册
    void retrieve();           //找回密码
    void setEfd(const int &e); //设置监听红黑树根节点
    User &getUser();           //获得u这个对象
    pthread_mutex_t &getMutex();

private:
    pthread_mutex_t mutex; //定义锁
    int efd;               //监听红黑书根节点，用于将文件描述符挂上树
    User u;                //用户信息
};

account::account()
{
    pthread_mutex_init(&mutex, NULL);
}
account::~account()
{
    pthread_mutex_destroy(&mutex);
}

pthread_mutex_t &account::getMutex()
{
    return mutex;
}

void startlogin(void *arg)
{
    account s = *((account *)arg);
    pthread_mutex_unlock(&(((account *)arg)->getMutex()));
    if ((s.getUser()).getFlag() == 1)
    {
        s.login();
    }
    else if ((s.getUser()).getFlag() == 2)
    {
        s.reg();
    }
    else if ((s.getUser()).getFlag() == 3)
    {
        s.retrieve();
    }
}

//从数据库读数据并与登陆输入的数据比较
bool account::login()
{
    struct epoll_event ep;
    int clnt_sock = u.getServ_fd();
    json jn;
    char buf[BUFSIZ];

    cout << u.getServ_fd();
    ssock::ReadMsg(clnt_sock, buf, sizeof(buf));
    cout << buf << endl;

    jn = json::parse(buf);
    u.From_Json(jn, u); //将会修改U中本来的内容，导致u中的文件描述符改变
    u.setServ_fd(clnt_sock);

    //打开数据库
    redisContext *c = Redis::RedisConnect("127.0.0.1", 6379);
    //查找与Number同名的哈希表，在map中查找密码，如果秘密相同返回该用户所有信息，
    //密码不同返回No
    redisReply *r = Redis::hgethash(c, "account", u.getNumber());
    if (r == NULL)
    {
        printf("Execut getValue failure\n");
        redisFree(c);
        return false;
    }
    if (r->type != REDIS_REPLY_STRING)
    {
        printf("Execut getValue failure\n");
        freeReplyObject(r);
        redisFree(c);
        ssock::SendMsg(clnt_sock, "No", 2);
        close(clnt_sock);
        return false;
    }
    json jn2 = json::parse(r->str);
    User u2;
    u.From_Json(jn2, u2);
    if (u2.getPasswd() == u.getPasswd())
    {
        ssock::SendMsg(clnt_sock, r->str, r->len + 1);
        freeReplyObject(r);
        redisFree(c);

        //执行完登录后将文件描述符挂上监听红黑树
        ep.events = EPOLLIN | EPOLLET;
        ep.data.fd = clnt_sock;
        int ret = epoll_ctl(efd, EPOLL_CTL_ADD, clnt_sock, &ep);

        return true;
    }
    else
    {
        ssock::SendMsg(clnt_sock, "No", 2);
        freeReplyObject(r);
        redisFree(c);
        close(u.getServ_fd());
        return false;
    }
}

//注册
void account::reg()
{
    int clnt_sock = u.getServ_fd();
    json jn;
    char buf[BUFSIZ];
    ssock::ReadMsg(clnt_sock, buf, sizeof(buf));
    jn = json::parse(buf);
    u.From_Json(jn, u); //将会修改u中本来的内容，导致u中的内容改变
    u.setServ_fd(clnt_sock);

    redisContext *c = Redis::RedisConnect("127.0.0.1", 6379);

    redisReply *r = Redis::hsetexist(c, "account", u.getNumber());
    if (r == NULL)
    {
        printf("Execut getValue failure\n");
        redisFree(c);
        return;
    }
    if (r->integer == 0)
    {
        printf("该账号数据库中没有\n");
        freeReplyObject(r);
        r = Redis::hsetValue(c, "account", u.getNumber(), jn.dump());
        freeReplyObject(r);
        ssock::SendMsg(clnt_sock, "Yes", 3);
    }
    else
    {
        freeReplyObject(r);
        ssock::SendMsg(clnt_sock, "No", 2);
    }
    redisFree(c);
    close(u.getServ_fd());
}

//找回密码
void account::retrieve()
{
    int clnt_sock = u.getServ_fd();
    json jn;
    char buf[BUFSIZ];
    ssock::ReadMsg(clnt_sock, buf, sizeof(buf));
    jn = json::parse(buf);
    u.From_Json(jn, u);
    u.setServ_fd(clnt_sock); //将会修改u中本来的内容，导致U中的文件描述符被改变

    //打开数据库
    redisContext *c = Redis::RedisConnect("127.0.0.1", 6379);
    //查找与Number同名的哈希表，在map中查找密码，如果秘密相同返回该用户所有信息，
    //密码不同返回No

    redisReply *r = Redis::hgethash(c, "account", u.getNumber());
    if (r == NULL)
    {
        printf("Execut getValue failure\n");
        redisFree(c);
        return;
    }
    if (r->type != REDIS_REPLY_STRING)
    {
        printf("Execut getValue failure\n");
        freeReplyObject(r);
        redisFree(c);
        ssock::SendMsg(clnt_sock, "No", 2);
        return;
    }
    json jn2 = json::parse(r->str);
    User u2;
    u.From_Json(jn2, u2);

    if ((u2.getKey() == u.getKey()) && (u2.getNumber() == u.getNumber()))
    {
        ssock::SendMsg(clnt_sock, r->str, r->len + 1);
    }
    else
    {
        ssock::SendMsg(clnt_sock, "No", 2);
    }
    close(u.getServ_fd());

    freeReplyObject(r);
    redisFree(c);
}

void account::setEfd(const int &e)
{
    efd = e;
}

//获得u这个对象
User &account::getUser()
{
    return u;
}



#endif