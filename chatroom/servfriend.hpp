#ifndef SERVFRIEND_H
#define SERVFRIEND_H
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
    void setEfd(const int &e); //设置监听树根
    void signout();            // 9退出登录
    void addFriend();          // 10 添加好友

    privateChat &getpChat();     //得到pChat对象
    pthread_mutex_t &getMutex(); //获得锁
    void talkwithfriends();      // 17和朋友聊天

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
    if (g.getpChat().getFlag() == 9) // 9退出登录
    {
        g.signout();
    }
    else if ((g.getpChat().getFlag() == 10)) //添加好友
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
    else if (g.getpChat().getFlag() == 17) // 17和好友聊天
    {
        g.talkwithfriends();
    }
}

void gay::setEfd(const int &e)
{
    efd = e;
}

void gay::signout() // 9退出登录
{
    int clnt_sock = pChat.getServ_fd();
    char buf[BUFSIZ];
    json jn;

    ssock::ReadMsg(clnt_sock, buf, BUFSIZ);
    jn = json::parse(buf);

    pChat.From_Json(jn, pChat);  //将会修改pChat中本来的文件描述符
    pChat.setServ_fd(clnt_sock); //将文件描述符改回去

    redisContext *c = Redis::RedisConnect("127.0.0.1", 6379);
    redisReply *r = Redis::hashdel(c, "Onlineuser", pChat.getNumber());
    if (r == NULL)
    {
        printf("Execut getValue failure\n");
        redisFree(c);
        return;
    }
    if (r->integer == 0)
    {
        cout << "下线失败" << endl;
    }

    //退出操作，执行完不需要将文件描述符挂上监听红黑树，将文件描述符关闭
    close(pChat.getServ_fd());

    freeReplyObject(r);
    redisFree(c);
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
    friends fri; //定义一个friends类，将获取到的朋友信息进行解析

    redisContext *c = Redis::RedisConnect("127.0.0.1", 6379);
    redisReply *r = Redis::hsetexist(c, pChat.getNumber() + "friend", pChat.getFriendUid()); //判断好友是否已经在好友列表
    if (r == NULL)
    {
        printf("Execut getValue failure\n");
        redisFree(c);
        return;
    }
    if (r->integer == 1)
    {
        printf("已经是好友\n");
        freeReplyObject(r);
        ssock::SendMsg(clnt_sock, "Has user", strlen("Has user") + 1); //添加失败，已经是好友
        redisFree(c);
        return;
    }
    freeReplyObject(r);
    r = Redis::hsetexist(c, "account", pChat.getFriendUid()); //判断账号列表是否有这个人
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
        fri.setflag(1);                         //设置朋友的权限
        fri.setfriendUid(pChat.getFriendUid()); //设置朋友的uid
        fri.To_Json(jn, fri);                   //将fri转换为json

        //将转换后的字符串写入数据库
        r = Redis::hsetValue(c, pChat.getNumber() + "friend", fri.getfriendUid(), jn.dump()); // 1表示正常，0表示屏蔽
        freeReplyObject(r);
    }
    redisFree(c);

    //执行完添加后将文件描述符挂上监听红黑树
    ep.events = EPOLLIN | EPOLLET;
    ep.data.fd = clnt_sock;
    int ret = epoll_ctl(efd, EPOLL_CTL_ADD, clnt_sock, &ep);
    if (ret == -1)
    {
        ssock::perr_exit("epoll_ctr error");
    }
}

privateChat &gay::getpChat()
{
    return pChat;
}

pthread_mutex_t &gay::getMutex() //获得锁
{
    return mutex;
}

void gay::talkwithfriends()
{
    struct epoll_event ep;
    int clnt_sock = pChat.getServ_fd();
    char buf[BUFSIZ];
    json jn;
}

#endif