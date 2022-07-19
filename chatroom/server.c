#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include "wrap.h"

#define SERV_PORT 8000
#define OPEN_MAX 5000
#define MAXLINE 8192

int main(void)
{
    int serv_sock, clnt_sock, sock;
    struct sockaddr_in serv_addr, clnt_addr;
    socklen_t clnt_addr_len;
    int i, n, num = 0;
    ssize_t nready, efd, ret;
    char buf[MAXLINE], str[INET_ADDRSTRLEN];

    serv_sock = Socket(AF_INET, SOCK_STREAM, 0);

    int opt = 1;
    setsockopt(serv_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)); //端口复用

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(SERV_PORT);
    Bind(serv_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    Listen(serv_sock, 20);

    efd = epoll_create(OPEN_MAX); //创建epoll模型, efd指向红黑树根节点
    if (efd == -1)
    {
        perr_exit("epoll_create error");
    }

    struct epoll_event tep, ep[OPEN_MAX]; // tep: epoll_ctl参数  ep[] : epoll_wait参数

    tep.events = EPOLLIN; //指定lfd的监听时间为"读"
    tep.data.fd = serv_sock;
    ret = epoll_ctl(efd, EPOLL_CTL_ADD, serv_sock, &tep); //将lfd及对应的结构体设置到树上,efd可找到该树
    if (ret == -1)
    {
        perr_exit("epoll_ctl error");
    }

    while (1)
    {
        // epoll为server阻塞监听事件, ep为struct epoll_event类型数组, OPEN_MAX为数组容量, -1表永久阻塞
        nready = epoll_wait(efd, ep, OPEN_MAX, -1);
        if (nready == -1)
        {
            perr_exit("epoll_wait error");
        }

        for (i = 0; i < nready; ++i)
        {
            if (!(ep[i].events & EPOLLIN))
            { //如果不是"读"事件, 继续循环
                continue;
            }

            if (ep[i].data.fd == serv_sock)
            { //判断满足事件的fd是不是lfd
                clnt_addr_len = sizeof(clnt_addr);
                clnt_sock = Accept(serv_sock, (struct sockaddr *)&clnt_addr, &clnt_addr_len);
                printf("received from %s at PORT %d\n",
                       inet_ntop(AF_INET, &clnt_addr.sin_addr, str, sizeof(str)),
                       ntohs(clnt_addr.sin_port));
                printf("cfd %d---client %d\n", clnt_sock, ++num);

                tep.events = EPOLLIN;
                tep.data.fd = clnt_sock;
                ret = epoll_ctl(efd, EPOLL_CTL_ADD, clnt_sock, &tep); //加入红黑树
                if (ret == -1)
                {
                    perr_exit("epoll_ctl error");
                }
            }
            else
            {
                // 不是lfd,
                sock = ep[i].data.fd;

                n = Read(sock, buf, MAXLINE);
                if (n == 0) //读到0,说明客户端关闭链接
                {
                    ret = epoll_ctl(efd, EPOLL_CTL_DEL, sock, NULL); //将该文件描述符从红黑树摘除
                    if (ret == -1)
                    {
                        perr_exit("epoll_ctr error");
                    }
                    close(sock); //关闭与该客户端的链接
                    printf("client[%d] closed connection\n", sock);
                }
                else if (n < 0)
                { //出错
                    perror("read n < 0 error: ");
                    ret = epoll_ctl(efd, EPOLL_CTL_DEL, sock, NULL); //摘除节点
                    Close(sock);
                }
                else
                { //实际读到了字节数
                    for (i = 0; i < n; i++)
                    {
                        buf[i] = toupper(buf[i]); //转大写,写回给客户端
                    }

                    Write(STDOUT_FILENO, buf, n);
                    Writen(sock, buf, n);
                }
            }
        }
    }

    close(serv_sock);
    close(efd);

    return 0;
}