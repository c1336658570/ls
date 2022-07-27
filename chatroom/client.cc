/*
g++ -o client client.cc client.hpp ssock.cpp -lhiredis

客户端需要采用多线程，需要同时进行读写
*/

#include <iostream>
#include <string>
#include "message.hpp"
#include "client.hpp"

int main(int argc, char *argv[])
{
    clnt a;
    a.show_Menu1(argv);
    if (argc != 3)
    {
        printf("Usage : %s <IP> <port>\n", argv[0]);
        exit(-1);
    }

    return 0;
}