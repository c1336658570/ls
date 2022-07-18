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

#endif