#ifndef SERV_H
#define SERV_H
#include <iostream>
#include <cstring>
#include <string>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "ssock.h"
#include "redis1.hpp"
#include <hiredis/hiredis.h>
#include <sys/epoll.h>
#include "jjson.hpp"
using namespace std;

#define OPEN_MAX 5000

class serv1
{
public:
    //建立连接，执行操作
    void connectClient()
    {
        int nready, i, sock;

        serv_fd = ssock::Socket();
        ssock::Bind(serv_fd, 9999, "127.0.0.1");
        ssock::Listen(serv_fd, 128);

        efd = epoll_create(OPEN_MAX);
        if (efd == -1)
        {
            ssock::perr_exit("epoll_create error");
        }

        tep.events = EPOLLIN;
        tep.data.fd = serv_fd;
        int ret = epoll_ctl(efd, EPOLL_CTL_ADD, serv_fd, &tep);
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
                if (!ep[i].events & EPOLLIN)
                {
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
                }
            }
        }
    }

    //从数据库读数据并与登陆输入的数据比较
    void login(int clnt_sock)
    {
        uint32_t len = 0;
        char buf[BUFSIZ];
        ssock::Readn(clnt_fd, &len, sizeof(len));
        len = ntohl(len);
        ssock::Readn(clnt_fd, buf, len);
        json jn = json::parse(R"(buf)");
        u.From_Json(jn, u);

        //打开数据库
        redisContext *c = Redis::RedisConnect("127.0.0.1", 6379);
        // set
    }

private:
    User u;
    int clnt_fd, serv_fd;
    int efd;
    struct epoll_event tep, ep[OPEN_MAX];
};

#endif