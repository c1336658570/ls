#include "servfriend.hpp"
#include "servlogin.hpp"

int main(int argc, char *argv[])
{
    c = Redis::RedisConnect("127.0.0.1", 6379);
    ThreadPool pool(10);

    account s;
    gay g;
    json jn;

    int serv_fd, sock;
    int efd;
    struct epoll_event tep, ep[OPEN_MAX];
    int nready, i, clnt_fd;
    int ret;

    if (argc != 3)
    {
        printf("Usage : %s <IP> <port>\n", argv[0]);
        exit(-1);
    }

    serv_fd = ssock::Socket();
    int opt = 1;
    setsockopt(serv_fd, SOL_SOCKET, SO_REUSEADDR, (void *)&opt, sizeof(opt));
    ssock::Bind(serv_fd, atoi(argv[2]), argv[1]);
    ssock::Listen(serv_fd, 0);

    efd = epoll_create(OPEN_MAX);
    if (efd == -1)
    {
        ssock::perr_exit("epoll_create error");
    }
    setsp();
    s.setEfd(efd);
    g.setEfd(efd);

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

                int permission = fcntl(clnt_fd, F_GETFL);
                permission |= O_NONBLOCK;
                fcntl(clnt_fd, F_SETFL, permission);

                tep.events = EPOLLIN | EPOLLET;
                tep.data.fd = clnt_fd;
                ret = epoll_ctl(efd, EPOLL_CTL_ADD, clnt_fd, &tep);
                if (ret == -1)
                {
                    ssock::perr_exit("epoll_ctl error");
                }
            }
            else
            { //不是监听套间字
                sock = ep[i].data.fd;
                uint32_t flag = 0;

                ret = ssock::ReadMsg(sock, (void *)&flag, sizeof(flag));
                if (ret == 0)
                {
                    ret = epoll_ctl(efd, EPOLL_CTL_DEL, ep[i].data.fd, NULL); //将该文件描述符从红黑树摘除
                    if (ret == -1)
                    {
                        ssock::perr_exit("epoll_ctr error");
                    }
                    qqqqquit(sock); //将其从在线用户中删除
                    close(sock);    //关闭与该客户端的链接
                    cout << sock << "关闭";
                    continue;
                }
                flag = ntohl(flag);

                ret = epoll_ctl(efd, EPOLL_CTL_DEL, ep[i].data.fd, NULL); //将该文件描述符从红黑树摘除
                if (ret == -1)
                {
                    ssock::perr_exit("epoll_ctr error");
                }

                if (flag == 100)
                {
                    pthread_mutex_lock(&(g.getMutex()));
                    pthread_t tid;
                    g.getpChat().setServ_fd(sock);
                    g.getpChat().setFlag(flag);
                    pthread_create(&tid, NULL, continue_send, &g);
                    pthread_detach(tid);
                }

                if (flag >= 1 && flag <= 3)
                {
                    pthread_mutex_lock(&s.getMutex());
                    s.getUser().setServ_fd(sock);
                    s.getUser().setFlag(flag);

                    task.arg = &s;
                    task.function = startlogin;
                    pool.addTask(task);
                }
                else if (flag >= 9 && flag <= 36)
                {
                    pthread_mutex_lock(&(g.getMutex()));
                    g.getpChat().setServ_fd(sock);
                    g.getpChat().setFlag(flag);

                    task.arg = &g;
                    task.function = startpchat;
                    pool.addTask(task);
                }
            }
        }
    }

    return 0;
}