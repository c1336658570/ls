#ifndef ACCOUNT_H
#define ACCOUNT_H
#include <iostream>
#include <unistd.h>
#include <time.h>
#include "message.hpp"
#include "ssock.hpp"
#include "leveldb/db.h"
using namespace std;

void *chat_recv_friend(void *arg); //接受好友消息的线程

class clnt
{
public:
    //菜单
    void show_Menu1();       //登录账号
    void show_Meun2();       //登录后界面
    void show_Menu3();       //好友管理
    void show_Menu4();       //私聊菜单
    int getclntfd();         //获取客户端套间字
    void read_account();     //账号输入
    void login();            // 1登录
    void reg();              // 2注册
    void retrieve();         // 3找回密码
    void quit();             // 4退出
    void addFriend();        // 10添加好友
    void chat_send_friend(); //给好友发消息
    void signout();          // 9退出登录

private:
    uint32_t flag;     //读取用户输入，保存用户的选项，1登陆，2注册，3找回密码，4退出
    User u;            //用户信息，用来登录或注册
    privateChat pChat; //私聊类，用于发送信息
    int clnt_fd;       //客户端套间字
};

void clnt::show_Menu1()
{
    while (1)
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
            cin.ignore(INT32_MAX, '\n');
        }
        u.setFlag(flag);
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
}

//账号输入
void clnt::read_account()
{
    string passwd, uid;

    cout << "请输入你的账号，不要超过20个字符" << endl;
    while (!(cin >> uid) || uid.size() > 20)
    {
        cout << "输入有误或账号过长，请重新输入" << endl;
        cin.clear();
        cin.ignore(INT32_MAX, '\n');
    }

    cout << "请输入密码，不要超过20个字符" << endl;
    while (!(cin >> passwd) || passwd.size() > 20)
    {
        cout << "输入有误或密码过长，请重新输入" << endl;
        cin.clear();
        cin.ignore(INT32_MAX, '\n');
    }

    u.setNumber(uid);    //设置账号
    u.setPasswd(passwd); //设置密码
}

void clnt::login() //登录
{
    clnt_fd = ssock::Socket();
    ssock::Connect(clnt_fd, 9999, "127.0.0.1");

    read_account();
    flag = htonl(flag);
    ssock::SendMsg(clnt_fd, (void *)&flag, sizeof(flag));

    //向服务器写数据
    json jn;
    u.To_Json(jn, u);
    string u_jn = jn.dump();

    ssock::SendMsg(clnt_fd, u_jn.c_str(), strlen(u_jn.c_str()) + 1);

    char buf[BUFSIZ];
    //从服务器读，看账号和密码是否正确
    ssock::ReadMsg(clnt_fd, buf, sizeof(buf));

    if (strncmp(buf, "No", 2) == 0)
    {
        cout << "账号或密码错误" << endl;
    }
    else if (strncmp(buf, "has online", strlen("has online")) == 0)
    {
        cout << "你已经登录过了" << endl;
    }
    else
    {
        cout << "登录成功" << endl;

        jn = json::parse(buf);
        u.From_Json(jn, u);
        u.print();
        show_Meun2();
    }
}

//注册
void clnt::reg()
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
        cin.clear();
        cin.ignore(INT32_MAX, '\n');
    }
    u.setKey(key);   //设置key
    u.setName(name); //设置昵称

    int clnt_fd = ssock::Socket();
    ssock::Connect(clnt_fd, 9999, "127.0.0.1");

    flag = htonl(flag);
    ssock::SendMsg(clnt_fd, (void *)&flag, sizeof(flag));

    //向服务器写数据
    json jn;
    u.To_Json(jn, u);
    string u_jn = jn.dump();

    ssock::SendMsg(clnt_fd, u_jn.c_str(), strlen(u_jn.c_str()) + 1);

    char buf[10];
    //从服务器读，看是否注册成功

    ssock::ReadMsg(clnt_fd, buf, sizeof(buf));

    if (strncmp(buf, "No", 2) == 0)
    {
        cout << "注册失败，账号已经存在！" << endl;
    }
    else if (strncmp(buf, "Yes", 3) == 0)
    {
        cout << "注册成功！" << endl;
    }
    close(clnt_fd);
}

void clnt::retrieve() //找回密码
{
    string key;
    string uid;
    cout << "请输入你的账号，不要超过20个字符" << endl;
    while (!(cin >> uid) || uid.size() > 20)
    {
        cout << "输入有误或账号过长，请重新输入" << endl;
        cin.clear();
        cin.ignore(INT32_MAX, '\n');
    }
    cout << "请输入你的密匙，不要超过20个字符" << endl;
    while (!(cin >> key) || key.size() > 20)
    {
        cout << "输入有误或密码过长，请重新输入" << endl;
        cin.clear();
        cin.ignore(INT32_MAX, '\n');
    }
    u.setNumber(uid);
    u.setKey(key);

    int clnt_fd = ssock::Socket();
    ssock::Connect(clnt_fd, 9999, "127.0.0.1");

    flag = htonl(flag);
    ssock::SendMsg(clnt_fd, (void *)&flag, sizeof(flag));

    //向服务器写数据
    json jn;
    u.To_Json(jn, u);
    string u_jn = jn.dump();

    ssock::SendMsg(clnt_fd, u_jn.c_str(), strlen(u_jn.c_str()) + 1);

    char buf[BUFSIZ];
    //从服务器读，看密码是多少

    ssock::ReadMsg(clnt_fd, buf, sizeof(buf));
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

void clnt::quit() //退出
{
    exit(0);
}

int clnt::getclntfd() //获取客户端套间字
{
    return clnt_fd;
}

void clnt::show_Meun2() // 5好友管理，6私聊，7群管理，8群聊，9退出账号
{
    while (1)
    {
        flag = 0;
        cout << "请输入你要执行的操作" << endl; // 5好友管理，6私聊，7群管理，8群聊，9退出账号
        cout << "5、好友管理" << endl;
        cout << "6、私聊" << endl;
        cout << "7、群管理" << endl;
        cout << "8、群聊" << endl;
        cout << "9、退出登录" << endl;
        while (!(cin >> flag) || flag < 5 || flag > 9)
        {
            cout << "输入有误" << endl;
            cin.clear();
            cin.ignore(INT32_MAX, '\n');
        }
        switch (flag)
        {
        case 5:
            clnt::show_Menu3();
            break;
        case 6:
            clnt::show_Menu4();
            break;
        case 7:
            break;
        case 8:
            break;
        case 9:
            signout();
            break;
        }
    }
}

void clnt::signout() // 9退出登录
{
    json jn;

    pChat.setNumber(u.getNumber()); //设置自己的uid
    pChat.setName(u.getName());     //设置自己的姓名
    pChat.setFlag(flag);            //设置自己的操作

    pChat.To_Json(jn, pChat); //将类转为序列

    flag = htonl(flag);
    ssock::SendMsg(clnt_fd, (void *)&flag, sizeof(flag));
    ssock::SendMsg(clnt_fd, jn.dump().c_str(), strlen(jn.dump().c_str()) + 1);
    close(clnt_fd);
    exit(0);
}

void clnt::show_Menu3() //好友管理
{
    while (1)
    {
        flag = 0;
        cout << "请输入要执行的操作" << endl; // 10、添加好友，11删除好友、12、查询好友、13显示好友在线状态、14屏蔽好友消息、15返回上一层
        cout << "10、添加好友" << endl;
        cout << "11、删除好友" << endl;
        cout << "12、查询好友" << endl;
        cout << "13、显示好友在线状态" << endl;
        cout << "14、屏蔽好友消息" << endl;
        cout << "15、返回上一层" << endl;
        while (!(cin >> flag) || flag < 10 || flag > 15)
        {
            cout << "输入有误" << endl;
            cin.clear();
            cin.ignore(INT32_MAX, '\n');
        }
        pChat.setFlag(flag);
        if (flag == 15)
        {
            break;
        }
        switch (flag)
        {
        case 10:
            addFriend();
            break;
        case 11:
            break;
        case 12:
            break;
        case 13:
            break;
        case 14:
            break;
        }
    }
}

//添加好友
void clnt::addFriend()
{
    char buf[BUFSIZ];
    json jn;
    string friendUid;
    pChat.setNumber(u.getNumber()); //设置自己的uid
    pChat.setName(u.getName());     //设置自己的姓名

    cout << "请输入你要添加的好友的uid" << endl;
    cin >> friendUid; //输入好友uid
    if (pChat.getNumber() == friendUid)
    {
        cout << "不可以添加自己" << endl;
        return;
    }
    pChat.setFriendUid(friendUid); //设置好友uid
    pChat.setTimeNow();            //设置时间

    pChat.To_Json(jn, pChat); //将类转为序列

    flag = htonl(flag);
    ssock::SendMsg(clnt_fd, (void *)&flag, sizeof(flag));
    ssock::SendMsg(clnt_fd, jn.dump().c_str(), strlen(jn.dump().c_str()) + 1); //将序列发给服务器
    ssock::ReadMsg(clnt_fd, buf, sizeof(buf));                                 //从服务器接受结果
    if (strcmp(buf, "No user") == 0)
    {
        cout << "你添加的好友不存在" << endl;
    }
    else if (strcmp(buf, "Has user") == 0)
    {
        cout << "你添加的好友已经存在" << endl;
    }
    else
    {
        cout << "添加成功" << endl;
    }
}

void clnt::show_Menu4() //私聊
{
    while (1)
    {
        flag = 0;
        cout << "请输入要执行的操作" << endl;
        cout << "16、查看历史聊天记录" << endl;
        cout << "17、和好友聊天" << endl;
        cout << "18、向好友发送文件" << endl;
        cout << "19、返回上一级" << endl;

        while (!(cin >> flag) || flag < 10 || flag > 15)
        {
            cout << "输入有误" << endl;
            cin.clear();
            cin.ignore(INT32_MAX, '\n');
        }
        pChat.setFlag(flag);
        if (flag == 19)
        {
            break;
        }
        switch (flag)
        {
        case 16:

            break;
        case 17:
            chat_send_friend();
            break;
        case 18:
            break;
        }
    }
}

void clnt::chat_send_friend()
{
    while (1)
    {
        pthread_t tid;
        char buf[BUFSIZ];
        json jn;
        string message;
        string friendUid;
        cout << "请输入你要发送的消息" << endl;
        cin >> message;
        cout << "请输入你要发送消息的uid" << endl;
        cin >> friendUid;

        pChat.setNumber(u.getNumber()); //设置自己的uid
        pChat.setName(u.getName());     //设置自己的姓名
        pChat.setFriendUid(friendUid);  //设置好友uid
        pChat.setTimeNow();             //设置时间
        pChat.setMessage(message);

        pthread_create(&tid, NULL, chat_recv_friend, &clnt_fd);
        pthread_detach(tid);

        if (pChat.getNumber() == friendUid)
        {
            cout << "不可以给自己发消息" << endl;
            return;
        }

        pChat.To_Json(jn, pChat); //将类转为序列

        flag = htonl(flag);
        ssock::SendMsg(clnt_fd, (void *)&flag, sizeof(flag));
        ssock::SendMsg(clnt_fd, jn.dump().c_str(), strlen(jn.dump().c_str()) + 1); //将序列发给服务器
        ssock::ReadMsg(clnt_fd, buf, sizeof(buf));
    }
}

void *chat_recv_friend(void *arg) //接受好友消息的线程
{
    json jn;
    privateChat pChat;

    char buf[BUFSIZ];
    int clnt_fd = *((int *)arg);
    while (1)
    {
        ssock::ReadMsg(clnt_fd, buf, sizeof(buf));
        jn = json::parse(buf);
        pChat.From_Json(jn, pChat);
        cout << pChat.getTimeNow() << endl
             << pChat.getName() << endl
             << pChat.getMessage() << endl;
    }
}

void continue_receive(void *arg)
{
}

#endif