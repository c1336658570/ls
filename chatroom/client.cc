/*
g++ -o client client.cc client.hpp ssock.cpp -lhiredis
*/

#include <iostream>
#include <string>
#include "jjson.hpp"
#include "client.hpp"

int main(void)
{
    account a;
    a.show_Menu();

    return 0;
}