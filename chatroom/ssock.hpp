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
    static ssize_t ReadMsg(int fd, void *vptr, size_t n);
    static ssize_t SendMsg(int fd, const void *vptr, size_t n);
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
        if (errno == EINTR || errno == EWOULDBLOCK)
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

ssize_t ssock::Readn(int fd, void *buffer, size_t n)
{
    ssize_t numRead;
    size_t totRead;
    char *buf;

    buf = (char *)buffer;
    for (totRead = 0; totRead < n;)
    {
        numRead = read(fd, buf, n - totRead);

        if (numRead == 0)
            return totRead;
        if (numRead == -1)
        {
            if (errno == EINTR || errno == EAGAIN)
                continue;
            else
                return -1;
        }
        totRead += numRead;
        buf += numRead;
    }
    return totRead;
}

ssize_t ssock::Writen(int fd, const void *buffer, size_t n)
{
    ssize_t numWritten;
    size_t totWritten;
    const char *buf;

    buf = (char *)buffer;
    for (totWritten = 0; totWritten < n;)
    {
        numWritten = write(fd, buf, n - totWritten);

        if (numWritten <= 0)
        {
            if (numWritten == -1 && errno == EINTR)
                continue;
            else
                return -1;
        }
        totWritten += numWritten;
        buf += numWritten;
    }
    return totWritten;
}

ssize_t ssock::ReadMsg(int fd, void *vptr, size_t n) // n为缓冲区的大小
{
    int ret;
    uint32_t len = 0;
    ret = ssock::Readn(fd, (void *)&len, sizeof(len));
    if (ret == 0)
    {
        return 0;
    }
    else if (ret == -1)
    {
        return -1;
    }
    len = ntohl(len);
    if (len > n)
    {
        return -1;
    }
    ret = ssock::Readn(fd, vptr, len);
    if (ret == 0)
    {
        return 0;
    }
    else if (ret == -1)
    {
        return -1;
    }
    return ret;
}
ssize_t ssock::SendMsg(int fd, const void *vptr, size_t n) // n为要发送的字节大小
{
    int ret = 0;
    uint32_t len = n;
    len = htonl(len);
    Writen(fd, (void *)&len, sizeof(len));
    ret = Writen(fd, vptr, n);
    return ret;
}
#endif