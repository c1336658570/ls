#include <iostream>
#include <string>
#include "jjson.hpp"
#include "account.hpp"

int main(void)
{
    while (1)
    {
        account a;
        int flag = a.show_Menu(); // 1登陆，2注册，3找回密码，4退出
        switch (flag)
        {
        case 1:
            a.login();
            break;
        case 2:
            a.reg();
            break;
        case 3:
            break;
        case 4:
            a.quit();
            break;
        }
    }

    return 0;
}