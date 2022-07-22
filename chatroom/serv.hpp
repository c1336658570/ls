/*
g++ -o serv serv.cc serv.hpp ssock.cpp -lhiredis
*/
#ifndef SERV_H
#define SERV_H
#include <iostream>
#include <cstring>
#include <string>
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

void start(void *arg);

class serv
{
public:
    void connectClient(); //建立连接，执行操作
    bool login();         //从数据库读数据并与登陆输入的数据比较
    void reg();           //注册
    void retrieve();      //找回密码
    User getUser();       //获得u这个对象

private:
    User u;
    json jn;
    int serv_fd, sock;
    int efd;
    struct epoll_event tep, ep[OPEN_MAX];
    map<string, int> friends; //保存在线用户的uid，和套间字。
    list<User> offline;       //保存离线的聊天记录，存放发送消息那个人的user信息，对方上线后在该list中查找
    list<string> groups;      //群id号，不能和用户id重复，群id号作为群索引，通过该索引找到该群的所有信息，通过哈希存储，里面包含了用户id和用户权限
};

void start(void *arg)
{
    serv *s = (serv *)arg;
    if ((s->getUser()).getFlag() == 1)
    {
        s->login();
    }
    else if ((s->getUser()).getFlag() == 2)
    {
        s->reg();
    }
    else if ((s->getUser()).getFlag() == 3)
    {
        s->retrieve();
    }
}

//建立连接，执行操作
void serv::connectClient()
{
    ThreadPool pool(10);
    int nready, i, clnt_fd;
    int ret;

    serv_fd = ssock::Socket();
    int opt = 1;
    setsockopt(serv_fd, SOL_SOCKET, SO_REUSEADDR, (void *)&opt, sizeof(opt));
    ssock::Bind(serv_fd, 9999, "127.0.0.1");
    ssock::Listen(serv_fd, 128);

    efd = epoll_create(OPEN_MAX);
    if (efd == -1)
    {
        ssock::perr_exit("epoll_create error");
    }

    tep.events = EPOLLIN;
    tep.data.fd = serv_fd;
    ret = epoll_ctl(efd, EPOLL_CTL_ADD, serv_fd, &tep);
    if (ret == -1)
    {
        ssock::perr_exit("epoll_ctl error");
    }

    Task task;
    while (1)
    {
        nready = epoll_wait(efd, ep, OPEN_MAX, -1);
        if (nready == -1)
        {
            ssock::perr_exit("epoll_wait error");
        }

        for (i = 0; i < nready; ++i)
        {
            if (!(ep[i].events & EPOLLIN))
            { //如果不是读事件，继续循环
                continue;
            }

            //是监听套间字
            if (ep[i].data.fd == serv_fd)
            {
                clnt_fd = ssock::Accept(serv_fd);
                int flag = fcntl(clnt_fd, F_GETFL);
                flag |= O_NONBLOCK;
                fcntl(clnt_fd, F_SETFL, flag);
                tep.events = EPOLLIN | EPOLLET;
                tep.data.fd = clnt_fd;
                ret = epoll_ctl(efd, EPOLL_CTL_ADD, clnt_fd, &tep);
                cout << "sock:" << clnt_fd << "connect" << endl;
                if (ret == -1)
                {
                    ssock::perr_exit("epoll_ctl error");
                }
            }
            else
            { //不是监听套间字
                char buf[BUFSIZ];
                sock = ep[i].data.fd;
                int flag = 0;

                ret = ssock::ReadMsg(sock, (void *)&flag, sizeof(flag));
                if (ret == 0)
                {
                    ret = epoll_ctl(efd, EPOLL_CTL_DEL, ep[i].data.fd, NULL); //将该文件描述符从红黑树摘除
                    if (ret == -1)
                    {
                        ssock::perr_exit("epoll_ctr error");
                    }
                    close(sock); //关闭与该客户端的链接
                    continue;
                }
                ssock::ReadMsg(sock, buf, sizeof(buf));
                if (flag >= 1 && flag <= 3)
                {
                    jn = json::parse(buf);
                    u.From_Json(jn, u);
                    if (u.getFlag() != 1)
                    {
                        ret = epoll_ctl(efd, EPOLL_CTL_DEL, ep[i].data.fd, NULL); //将该文件描述符从红黑树摘除
                        if (ret == -1)
                        {
                            ssock::perr_exit("epoll_ctr error");
                        }
                    }
                    task.arg = this;
                    task.function = start;
                    pool.addTask(task);
                }
            }
        }
    }
}

//从数据库读数据并与登陆输入的数据比较
bool serv::login()
{
    int clnt_sock = sock;
    //打开数据库
    redisContext *c = Redis::RedisConnect("127.0.0.1", 6379);
    //查找与Number同名的哈希表，在map中查找密码，如果秘密相同返回该用户所有信息，
    //密码不同返回No
    redisReply *r = Redis::hgethash(c, "account", u.getNumber());
    uint32_t len = 0;
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
        len = 2;
        len = htonl(len);
        ssock::Writen(clnt_sock, (void *)&len, sizeof(len));
        ssock::Writen(clnt_sock, "No", 2);
        return false;
    }
    json jn2 = json::parse(r->str);
    User u2;
    u.From_Json(jn2, u2);
    if (u2.getPasswd() == u.getPasswd())
    {
        len = r->len + 1;
        len = htonl(len);
        ssock::Writen(clnt_sock, (void *)&len, sizeof(len));
        ssock::Writen(clnt_sock, r->str, r->len + 1);

        //登录后将该用户插入到map容器中
        friends.insert(pair<string, int>(u.getNumber(), clnt_sock));
        freeReplyObject(r);
        redisFree(c);
        return true;
    }
    else
    {
        len = 2;
        len = htonl(len);
        ssock::Writen(clnt_sock, (void *)&len, sizeof(len));
        ssock::Writen(clnt_sock, "No", 2);
        freeReplyObject(r);
        redisFree(c);
        return false;
    }
}

//注册
void serv::reg()
{
    int clnt_sock = sock;
    uint32_t len;
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
        len = 3;
        len = htonl(len);
        ssock::Writen(clnt_sock, (void *)&len, sizeof(len));
        ssock::Writen(clnt_sock, "Yes", 3);
    }
    else
    {
        freeReplyObject(r);
        len = 2;
        len = htonl(len);
        ssock::Writen(clnt_sock, (void *)&len, sizeof(len));
        ssock::Writen(clnt_sock, "No", 2);
    }
    redisFree(c);
    close(sock);
}

//找回密码
void serv::retrieve()
{
    int clnt_sock = sock;
    //打开数据库
    redisContext *c = Redis::RedisConnect("127.0.0.1", 6379);
    //查找与Number同名的哈希表，在map中查找密码，如果秘密相同返回该用户所有信息，
    //密码不同返回No

    redisReply *r = Redis::hgethash(c, "account", u.getNumber());
    uint32_t len = 0;
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
        len = 2;
        len = htonl(len);
        ssock::Writen(clnt_sock, (void *)&len, sizeof(len));
        ssock::Writen(clnt_sock, "No", 2);
        return;
    }
    json jn2 = json::parse(r->str);
    User u2;
    u.From_Json(jn2, u2);

    if ((u2.getKey() == u.getKey()) && (u2.getNumber() == u.getNumber()))
    {
        len = r->len + 1;
        len = htonl(len);
        ssock::Writen(clnt_sock, (void *)&len, sizeof(len));
        ssock::Writen(clnt_sock, r->str, r->len + 1);
    }
    else
    {
        len = 2;
        len = htonl(len);
        ssock::Writen(clnt_sock, (void *)&len, sizeof(len));
        ssock::Writen(clnt_sock, "No", 2);
    }
    close(sock);

    freeReplyObject(r);
    redisFree(c);
}

//获得u这个对象
User serv::getUser()
{
    return u;
}

#endif