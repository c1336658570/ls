#pragma once
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

class gay
{
public:
    gay();
    ~gay();
    void setEfd(const int &e);   //设置监听树根
    void addFriend();            //添加好友
    privateChat &getpChat();     //得到pChat对象
    pthread_mutex_t &getMutex(); //获得锁

private:
    pthread_mutex_t mutex;
    int efd;
    privateChat pChat;
};

gay::gay()
{
    pthread_mutex_init(&mutex, NULL);
}

gay::~gay()
{
    pthread_mutex_destroy(&mutex);
}

void startpchat(void *arg)
{
    gay g = *((gay *)arg);
    pthread_mutex_unlock(&(((gay *)arg)->getMutex()));

    cout << g.getpChat().getFlag() << endl;
    if ((g.getpChat().getFlag() == 10))
    {
        g.addFriend();
    }
    else if ((g.getpChat().getFlag() == 11))
    {
    }
    else if ((g.getpChat().getFlag() == 12))
    {
    }
    else if ((g.getpChat().getFlag() == 13))
    {
    }
}

void gay::setEfd(const int &e)
{
    efd = e;
}

void gay::addFriend()
{
    struct epoll_event ep;
    int clnt_sock = pChat.getServ_fd();
    char buf[BUFSIZ];
    json jn;

    ssock::ReadMsg(clnt_sock, buf, BUFSIZ);
    jn = json::parse(buf);

    pChat.From_Json(jn, pChat); //将会修改pChat中本来的文件描述符
    pChat.setServ_fd(clnt_sock);

    redisContext *c = Redis::RedisConnect("127.0.0.1", 6379);
    redisReply *r = Redis::hsetexist(c, "account", pChat.getFriendUid());
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
        ssock::SendMsg(clnt_sock, "No user", strlen("No user") + 1); //添加失败，数据库没有该用户
    }
    else
    {
        ssock::SendMsg(clnt_sock, "Added successfully", strlen("Added successfully") + 1);
        freeReplyObject(r);
        r = Redis::hsetValue(c, pChat.getNumber() + "friend", pChat.getFriendUid(), "1"); // 1表示正常，0表示屏蔽
        freeReplyObject(r);
    }
    redisFree(c);

    //执行完添加后将文件描述符挂上监听红黑树
    ep.events = EPOLLIN | EPOLLET;
    ep.data.fd = clnt_sock;
    int ret = epoll_ctl(efd, EPOLL_CTL_ADD, clnt_sock, &ep);
}

privateChat &gay::getpChat()
{
    return pChat;
}

pthread_mutex_t &gay::getMutex() //获得锁
{
    return mutex;
}