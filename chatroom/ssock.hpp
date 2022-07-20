#ifndef SOCKET_H
#define SOCKET_H
#include <iostream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
using namespace std;

class ssock
{
public:
    static void perr_exit(const string s);
    static int Socket();
    static void Bind(int sockfd, u_int16_t port, const string ip = "127.1");
    static void Listen(int sockfd, int num = 128);
    static int Accept(int sockfd);
    static void Connect(int sockfd, uint16_t port, const string ip);
    static ssize_t Read(int fd, void *ptr, size_t nbytes);
    static ssize_t Write(int fd, const void *ptr, size_t nbytes);
    static ssize_t Readn(int fd, void *vptr, size_t n);
    static ssize_t Writen(int fd, const void *vptr, size_t n);
};

void ssock::perr_exit(const string s)
{
    perror(s.c_str());
    exit(-1);
}

int ssock::Socket()
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
    {
        ssock::perr_exit("socket error");
    }

    return fd;
}

void ssock::Bind(int sockfd, u_int16_t port, const string ip)
{
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &serv_addr.sin_addr);
    int ret = bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    if (ret < 0)
    {
        ssock::perr_exit("bind error");
    }
}

void ssock::Listen(int sockfd, int num)
{
    int ret = listen(sockfd, num);
    if (ret < 0)
    {
        ssock::perr_exit("listen error");
    }
}

int ssock::Accept(int sockfd)
{
    int fd = -1;
    do
    {
        fd = accept(sockfd, NULL, NULL);
        if (fd < 0)
        {
            if ((errno == ECONNABORTED) || (errno == EINTR))
            {
                continue;
            }
            else
            {
                ssock::perr_exit("accept error");
            }
        }
        else
        {
            break;
        }
    } while (1);

    return fd;
}

void ssock::Connect(int sockfd, uint16_t port, const string ip)
{
    struct sockaddr_in serv_addr;

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &serv_addr.sin_addr);
    int ret = connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    if (ret < 0)
    {
        perr_exit("connect error");
    }
}

ssize_t ssock::Read(int fd, void *ptr, size_t nbytes)
{
    ssize_t n;

again:
    if ((n = read(fd, ptr, nbytes)) == -1)
    {
        if (errno == EINTR)
            goto again;
        else
            return -1;
    }
    return n;
}

ssize_t ssock::Write(int fd, const void *ptr, size_t nbytes)
{
    ssize_t n;

again:
    if ((n = write(fd, ptr, nbytes)) == -1)
    {
        if (errno == EINTR)
            goto again;
        else
            return -1;
    }
    return n;
}

//参三: 应该读取的字节数
ssize_t ssock::Readn(int fd, void *vptr, size_t n)
{
    size_t nleft;  // usigned int 剩余未读取的字节数
    ssize_t nread; // int 实际读到的字节数
    char *ptr;

    ptr = (char *)vptr;
    nleft = n;

    while (nleft > 0)
    {
        if ((nread = read(fd, ptr, nleft)) < 0)
        {
            if (errno == EINTR)
                nread = 0;
            else
                return -1;
        }
        else if (nread == 0)
            break;

        nleft -= nread;
        ptr += nread;
    }
    return n - nleft;
}

ssize_t ssock::Writen(int fd, const void *vptr, size_t n)
{
    size_t nleft;
    ssize_t nwritten;
    const char *ptr;

    ptr = (const char *)vptr;
    nleft = n;
    while (nleft > 0)
    {
        if ((nwritten = write(fd, ptr, nleft)) <= 0)
        {
            if (nwritten < 0 && errno == EINTR)
                nwritten = 0;
            else
                return -1;
        }

        nleft -= nwritten;
        ptr += nwritten;
    }
    return n;
}
#endif