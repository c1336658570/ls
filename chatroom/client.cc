/*
g++ -o client client.cc client.hpp ssock.cpp -lhiredis

客户端需要采用多线程，需要同时进行读写
*/

#include <iostream>
#include <string>
#include "message.hpp"
#include "client.hpp"

int main(void)
{
    clnt a;
    a.show_Menu1();

    return 0;
}