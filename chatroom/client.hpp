#ifndef ACCOUNT_H
#define ACCOUNT_H
#include <iostream>
#include <unistd.h>
#include "jjson.hpp"
#include "ssock.hpp"
#include "leveldb/db.h"
using namespace std;

class account
{
public:
    //菜单
    void show_Menu()
    {
        cout << "请输入你要执行的操作" << endl;
        cout << "1、登陆" << endl;
        cout << "2、注册" << endl;
        cout << "3、找回密码" << endl;
        cout << "4、退出" << endl;
        while (!(cin >> flag) || flag < 1 || flag > 4)
        {
            cout << "输入有误" << endl;
            cin.clear();
        }
        u.setFlag(flag); //设置flag
        switch (flag)
        {
        case 1:
            login();
            break;
        case 2:
            reg();
            break;
        case 3:
            retrieve();
            break;
        case 4:
            quit();
            break;
        }
    }

    //账号输入
    void read_account()
    {
        string passwd, uid;

        cout << "请输入你的账号，不要超过20个字符" << endl;
        while (!(cin >> uid) || uid.size() > 20)
        {
            cout << "输入有误或账号过长，请重新输入" << endl;
            cin.clear(); //清除cin缓冲
        }

        cout << "请输入密码，不要超过20个字符" << endl;
        while (!(cin >> passwd) || passwd.size() > 20)
        {
            cout << "输入有误或密码过长，请重新输入" << endl;
            cin.clear(); //清除cin缓冲
        }

        u.setNumber(uid);    //设置账号
        u.setPasswd(passwd); //设置密码
    }

    //登录
    void login()
    {
        int i;

        int clnt_fd = ssock::Socket();
        ssock::Connect(clnt_fd, 9999, "127.0.0.1");

        for (i = 1; i <= 5; ++i)
        {
            cout << "你有5次登录机会，现在是第" << i << "次" << endl;
            read_account();

            //向服务器写数据
            json jn;
            u.To_Json(jn, u);
            string u_jn = jn.dump();
            uint32_t len = strlen(u_jn.c_str()) + 1;
            len = htonl(len);
            ssock::Writen(clnt_fd, (void *)&len, sizeof(len));
            ssock::Writen(clnt_fd, u_jn.c_str(), strlen(u_jn.c_str()) + 1);

            char buf[BUFSIZ];
            //从服务器读，看账号和密码是否正确
            ssock::Readn(clnt_fd, (void *)&len, sizeof(len));
            len = ntohl(len);
            ssock::Readn(clnt_fd, buf, len);
            if (strncmp(buf, "No", 2) == 0)
            {
                cout << "账号或密码错误" << endl;
            }
            else
            {
                cout << "登录成功" << endl;
                jn = json::parse(buf);
                u.From_Json(jn, u);
                u.setClnt_fd(clnt_fd);
                u.print();
                break;
            }
        }

        if (i == 5)
        {
            close(clnt_fd);
            cout << "登录失败，程序退出！\n";
            exit(-1);
        }
    }

    //注册
    void reg()
    {
        string key, name;

        read_account();

        cout << "请输入你的昵称，不要超过20个字符" << endl;
        while (!(cin >> name) || name.size() > 20)
        {
            cout << "输入有误或昵称过长，请重新输入" << endl;
            cin.clear(); //清除cin缓冲
        }

        cout << "请输入密匙，用来找回密码，不要超过20个字符" << endl;
        while (!(cin >> key) || key.size() > 20)
        {
            cout << "输入有误或密码过长，请重新输入" << endl;
            cin.clear(); //清除cin缓冲
        }
        u.setKey(key);   //设置key
        u.setName(name); //设置昵称

        int clnt_fd = ssock::Socket();
        ssock::Connect(clnt_fd, 9999, "127.0.0.1");

        //向服务器写数据
        json jn;
        u.To_Json(jn, u);
        string u_jn = jn.dump();
        uint32_t len = strlen(u_jn.c_str()) + 1;
        len = htonl(len);
        ssock::Writen(clnt_fd, (void *)&len, sizeof(len));
        ssock::Writen(clnt_fd, u_jn.c_str(), strlen(u_jn.c_str()) + 1);

        char buf[10];
        //从服务器读，看是否注册成功
        ssock::Readn(clnt_fd, (void *)&len, sizeof(len));
        len = ntohl(len);
        ssock::Readn(clnt_fd, buf, len);
        if (strncmp(buf, "No", 2) == 0)
        {
            cout << "注册失败，账号已经存在！" << endl;
        }
        else
        {
            cout << "注册成功！" << endl;
        }
        close(clnt_fd);
    }

    //找回密码
    void retrieve()
    {
        string key;
        string uid;
        cout << "请输入你的账号，不要超过20个字符" << endl;
        while (!(cin >> uid) || uid.size() > 20)
        {
            cout << "输入有误或账号过长，请重新输入" << endl;
            cin.clear(); //清除cin缓冲
        }
        cout << "请输入你的密匙，不要超过20个字符" << endl;
        while (!(cin >> key) || key.size() > 20)
        {
            cout << "输入有误或密码过长，请重新输入" << endl;
            cin.clear(); //清除cin缓冲
        }
        u.setNumber(uid);
        u.setKey(key);

        int clnt_fd = ssock::Socket();
        ssock::Connect(clnt_fd, 9999, "127.0.0.1");

        //向服务器写数据
        json jn;
        u.To_Json(jn, u);
        string u_jn = jn.dump();
        uint32_t len = strlen(u_jn.c_str()) + 1;
        len = htonl(len);
        ssock::Writen(clnt_fd, (void *)&len, sizeof(len));
        ssock::Writen(clnt_fd, u_jn.c_str(), strlen(u_jn.c_str()) + 1);

        char buf[BUFSIZ];
        //从服务器读，看密码是多少
        ssock::Readn(clnt_fd, (void *)&len, sizeof(len));
        len = ntohl(len);
        ssock::Readn(clnt_fd, buf, len);
        if (strncmp(buf, "No", 2) == 0)
        {
            cout << "找回失败，密匙错误或账号错误" << endl;
        }
        else
        {
            jn = json::parse(buf);
            u.From_Json(jn, u);
            cout << "找回成功，密码为：" << u.getPasswd() << endl;
        }
        close(clnt_fd);
    }

    //退出
    void quit()
    {
        exit(0);
    }

private:
    int flag = 0; //读取用户输入，保存用户的选项，1登陆，2注册，3找回密码，4退出
    User u;       //用户信息，用来登录或注册
};

#endif