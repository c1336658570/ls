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
#include "jjson.hpp"
#include "redis1.hpp"
using namespace std;

#define OPEN_MAX 5000

class serv1
{
public:
    //建立连接，执行操作
    void connectClient()
    {
        int nready, i, clnt_fd, sock;
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
                    tep.events = EPOLLIN;
                    tep.data.fd = clnt_fd;
                    ret = epoll_ctl(efd, EPOLL_CTL_ADD, clnt_fd, &tep);
                    if (ret == -1)
                    {
                        ssock::perr_exit("epoll_ctl error");
                    }
                }
                else
                { //不是监听套间字
                    char buf[BUFSIZ];
                    sock = ep[i].data.fd;
                    uint32_t len;
                    ret = ssock::Readn(sock, (void *)&len, sizeof(len));
                    if (ret == 0)
                    {
                        ret = epoll_ctl(efd, EPOLL_CTL_DEL, sock, NULL); //将该文件描述符从红黑树摘除
                        if (ret == -1)
                        {
                            ssock::perr_exit("epoll_ctr error");
                        }
                        close(sock); //关闭与该客户端的链接
                    }
                    len = ntohl(len);
                    ret = ssock::Readn(sock, buf, len);
                    if (ret == 0)
                    {
                        ret = epoll_ctl(efd, EPOLL_CTL_DEL, sock, NULL); //将该文件描述符从红黑树摘除
                        if (ret == -1)
                        {
                            ssock::perr_exit("epoll_ctr error");
                        }
                        close(sock); //关闭与该客户端的链接
                        continue;
                    }
                    jn = json::parse(buf);
                    u.From_Json(jn, u);
                    switch (u.getFlag())
                    {
                    case 1:
                        login(sock);
                        break;
                    case 2:
                        //注册后，对端套间字会关闭，将其从红黑书
                        reg(sock);
                        ret = epoll_ctl(efd, EPOLL_CTL_DEL, sock, NULL); //将该文件描述符从红黑树摘除
                        if (ret == -1)
                        {
                            ssock::perr_exit("epoll_ctr error");
                        }
                        close(sock); //关闭与该客户端的链接
                        break;
                    case 3:
                        retrieve(sock);
                        break;
                    }
                }
            }
        }
    }

    //从数据库读数据并与登陆输入的数据比较
    void login(int clnt_sock)
    {
        //打开数据库
        redisContext *c = Redis::RedisConnect("127.0.0.1", 6379);
        //查找与Number同名的哈希表，在map中查找密码，如果秘密相同返回该用户所有信息，
        //密码不同返回No
        redisReply *r = Redis::getValue(c, u.getNumber());
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
        if (u2.getPasswd() == u.getPasswd())
        {
            len = r->len + 1;
            len = htonl(len);
            ssock::Writen(clnt_sock, (void *)&len, sizeof(len));
            ssock::Writen(clnt_sock, r->str, r->len + 1);

            //登录后将该用户插入到map容器中
            friends.insert(pair<string, int>(u.getNumber(), clnt_sock));
        }
        else
        {
            len = 2;
            len = htonl(len);
            ssock::Writen(clnt_sock, (void *)&len, sizeof(len));
            ssock::Writen(clnt_sock, "No", 2);
        }

        freeReplyObject(r);
        redisFree(c);
    }

    //注册
    void reg(int clnt_sock)
    {
        uint32_t len;
        redisContext *c = Redis::RedisConnect("127.0.0.1", 6379);

        redisReply *r = Redis::existsValue(c, u.getNumber());
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
            r = Redis::setValue(c, u.getNumber(), jn.dump());
            len = 3;
            len = htonl(len);
            ssock::Writen(clnt_sock, (void *)&len, sizeof(len));
            ssock::Writen(clnt_sock, "Yes", 3);
        }
        else
        {
            len = 2;
            len = htonl(len);
            ssock::Writen(clnt_sock, (void *)&len, sizeof(len));
            ssock::Writen(clnt_sock, "No", 2);
        }
        redisFree(c);
    }

    //找回密码
    void retrieve(int clnt_sock)
    {
        //打开数据库
        redisContext *c = Redis::RedisConnect("127.0.0.1", 6379);
        //查找与Number同名的哈希表，在map中查找密码，如果秘密相同返回该用户所有信息，
        //密码不同返回No

        redisReply *r = Redis::getValue(c, u.getNumber());
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

        freeReplyObject(r);
        redisFree(c);
    }

private:
    User u;
    json jn;
    int serv_fd;
    int efd;
    struct epoll_event tep, ep[OPEN_MAX];
    map<string, int> friends;
};

#endif