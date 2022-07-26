#ifndef ACCOUNT_H
#define ACCOUNT_H
#include <iostream>
#include <unistd.h>
#include <time.h>
#include "message.hpp"
#include "ssock.hpp"
#include "macro.h"
using namespace std;

void *chat_recv_friend(void *arg); //接受好友消息的线程
void *continue_receive(void *arg); //持续从服务器读数据的线程

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
    void signout();          // 9退出登录
    void addFriend();        // 10添加好友
    void inquireAdd();       // 11查看好友添加信息
    void delFriend();        // 12删除好友
    void findFriend();       // 13查询好友
    void onlineStatus();     // 14显示好友在线情况
    void blockFriend();      // 15屏蔽或解除屏蔽好友消息
    void history_message();  // 17历史聊天记录
    void chat_send_friend(); // 18给好友发消息

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
            if (cin.eof())
            {
                cout << "读到文件结束，函数返回" << endl;
                return;
            }
            cout << "输入有误" << endl;
            cin.clear();
            cin.ignore(INT32_MAX, '\n');
        }
        cin.ignore(INT32_MAX, '\n'); //清空cin缓冲
        u.setFlag(flag);
        switch (flag)
        {
        case LOGIN:
            login();
            break;
        case REGISTER:
            reg();
            break;
        case RETRIEVE:
            retrieve();
            break;
        case QUIT:
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
        if (cin.eof())
        {
            cout << "读到文件结束，函数返回" << endl;
            return;
        }
        cout << "输入有误或账号过长，请重新输入" << endl;
        cin.clear();
        cin.ignore(INT32_MAX, '\n');
    }
    cin.ignore(INT32_MAX, '\n'); //清空cin缓冲

    cout << "请输入密码，不要超过20个字符" << endl;
    while (!(cin >> passwd) || passwd.size() > 20)
    {
        if (cin.eof())
        {
            cout << "读到文件结束，函数返回" << endl;
            return;
        }
        cout << "输入有误或密码过长，请重新输入" << endl;
        cin.clear();
        cin.ignore(INT32_MAX, '\n');
    }
    cin.ignore(INT32_MAX, '\n'); //清空cin缓冲

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
        pthread_t tid;
        cout << "登录成功" << endl;

        jn = json::parse(buf);
        u.From_Json(jn, u);
        u.print();
        pChat.setNumber(u.getNumber());                               //在此处设置number，一直读消息的线程需要使用
        pthread_create(&tid, NULL, continue_receive, (void *)&pChat); //创建一个接受消息的线程
        pthread_detach(tid);                                          //设置线程分离
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
        if (cin.eof())
        {
            cout << "读到文件结束，函数返回" << endl;
            return;
        }
        cout << "输入有误或昵称过长，请重新输入" << endl;
        cin.clear(); //清除cin缓冲
        cin.ignore(INT32_MAX, '\n');
    }
    cin.ignore(INT32_MAX, '\n'); //清空cin缓冲

    cout << "请输入密匙，用来找回密码，不要超过20个字符" << endl;
    while (!(cin >> key) || key.size() > 20)
    {
        if (cin.eof())
        {
            cout << "读到文件结束，函数返回" << endl;
            return;
        }
        cout << "输入有误或密码过长，请重新输入" << endl;
        cin.clear();
        cin.ignore(INT32_MAX, '\n');
    }
    cin.ignore(INT32_MAX, '\n'); //清空cin缓冲

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
        if (cin.eof())
        {
            cout << "读到文件结束，函数返回" << endl;
            return;
        }
        cout << "输入有误或账号过长，请重新输入" << endl;
        cin.clear();
        cin.ignore(INT32_MAX, '\n');
    }
    cin.ignore(INT32_MAX, '\n'); //清空cin缓冲

    cout << "请输入你的密匙，不要超过20个字符" << endl;
    while (!(cin >> key) || key.size() > 20)
    {
        if (cin.eof())
        {
            cout << "读到文件结束，函数返回" << endl;
            return;
        }
        cout << "输入有误或密码过长，请重新输入" << endl;
        cin.clear();
        cin.ignore(INT32_MAX, '\n');
    }
    cin.ignore(INT32_MAX, '\n'); //清空cin缓冲

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
            if (cin.eof())
            {
                cout << "读到文件结束，函数返回" << endl;
                return;
            }
            cout << "输入有误" << endl;
            cin.clear();
            cin.ignore(INT32_MAX, '\n');
        }
        cin.ignore(INT32_MAX, '\n'); //清空cin缓冲

        switch (flag)
        {
        case SHOW_MENU3:
            clnt::show_Menu3();
            break;
        case SHOW_MENU4:
            clnt::show_Menu4();
            break;
        case SHOW_MENU5:
            break;
        case SHOW_MENU6:
            break;
        case SIGNOUT:
            signout();
            break;
        }
    }
}

void clnt::signout() // 9退出登录
{
    json jn;

    pChat.setFlag(flag);            //设置自己的操作
    pChat.setNumber(u.getNumber()); //设置自己的uid
    pChat.setName(u.getName());     //设置自己的姓名

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
        cout << "11、查看好友添加信息" << endl;
        cout << "12、删除好友" << endl;
        cout << "13、查询好友" << endl;
        cout << "14、显示好友在线状态" << endl;
        cout << "15、屏蔽好友或解除屏蔽" << endl;
        cout << "16、返回上一层" << endl;
        while (!(cin >> flag) || flag < 10 || flag > 16)
        {
            if (cin.eof())
            {
                cout << "读到文件结束，函数返回" << endl;
                return;
            }
            cout << "输入有误" << endl;
            cin.clear();
            cin.ignore(INT32_MAX, '\n');
        }
        cin.ignore(INT32_MAX, '\n'); //清空cin缓冲

        pChat.setFlag(flag); //设置操作
        if (flag == RETURNON1)
        {
            break;
        }
        switch (flag)
        {
        case ADDFRIEND:
            addFriend();
            break;
        case INQUIREADD:
            inquireAdd();
            break;
        case DELFRIEND:
            delFriend();
            break;
        case FINDFRIEND:
            findFriend();
            break;
        case ONLINESTATUS:
            onlineStatus();
            break;
        case BLOCKFRIEND:
            blockFriend();
            break;
        }
    }
}

// 10添加好友
void clnt::addFriend()
{
    char buf[BUFSIZ];
    json jn;
    string friendUid;
    pChat.setNumber(u.getNumber()); //设置自己的uid
    pChat.setName(u.getName());     //设置自己的姓名

    cout << "请输入你要添加的好友的uid，不要超过20个字符" << endl;
    //输入好友uid
    while (!(cin >> friendUid) || friendUid.size() > 20)
    {
        if (cin.eof())
        {
            cout << "读到文件结束，函数返回" << endl;
            return;
        }
        cout << "输入有误" << endl;
        cin.clear();
        cin.ignore(INT32_MAX, '\n');
    }
    cin.ignore(INT32_MAX, '\n'); //清空cin缓冲

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

// 11查询好友添加信息
void clnt::inquireAdd()
{
    char buf[BUFSIZ];
    json jn;

    flag = htonl(flag);
    ssock::SendMsg(clnt_fd, (void *)&flag, sizeof(flag));
}

// 12删除好友
void clnt::delFriend()
{
    char buf[BUFSIZ];
    json jn;
    string friendUid;
    pChat.setNumber(u.getNumber()); //设置自己的uid
    pChat.setName(u.getName());     //设置自己的姓名

    cout << "请输入你要删除的好友的uid，不要超过20个字符" << endl;
    //输入好友uid
    while (!(cin >> friendUid) || friendUid.size() > 20)
    {
        if (cin.eof())
        {
            cout << "读到文件结束，函数返回" << endl;
            return;
        }
        cout << "输入有误" << endl;
        cin.clear();
        cin.ignore(INT32_MAX, '\n');
    }
    cin.ignore(INT32_MAX, '\n'); //清空cin缓冲

    if (pChat.getNumber() == friendUid)
    {
        cout << "不可以删除自己" << endl;
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
        cout << "你删除的好友不存在" << endl;
    }
    else if (strcmp(buf, "del successfully") == 0)
    {
        cout << "删除成功" << endl;
    }
}

// 13查询好友
void clnt::findFriend()
{
    friends fri;
    json jn;
    char buf[BUFSIZ];

    flag = htonl(flag);
    ssock::SendMsg(clnt_fd, (void *)&flag, sizeof(flag));

    pChat.setNumber(u.getNumber()); //设置自己的uid
    pChat.To_Json(jn, pChat);
    ssock::SendMsg(clnt_fd, jn.dump().c_str(), strlen(jn.dump().c_str()) + 1);

    //从对端读消息
    while (1)
    {
        ssock::ReadMsg(clnt_fd, buf, sizeof(buf));
        if (strcmp(buf, "finish") == 0)
        {
            break;
        }
        jn = json::parse(buf);
        fri.From_Json(jn, fri);
        cout << "uid = " << fri.getfriendUid() << endl;
    }
}

// 14显示好友在线情况
void clnt::onlineStatus()
{
    json jn;
    char buf[BUFSIZ];
    friends fri;

    flag = htonl(flag);
    ssock::SendMsg(clnt_fd, (void *)&flag, sizeof(flag));

    pChat.setNumber(u.getNumber()); //设置自己的uid
    pChat.To_Json(jn, pChat);
    ssock::SendMsg(clnt_fd, jn.dump().c_str(), strlen(jn.dump().c_str()) + 1); //向对端发送消息

    //从对端读消息
    while (1)
    {
        ssock::ReadMsg(clnt_fd, buf, sizeof(buf));
        if (strcmp(buf, "finish") == 0)
        {
            break;
        }
        jn = json::parse(buf);
        fri.From_Json(jn, fri);
        if (fri.getflag() == 1 || fri.getflag() == 0)
        {
            cout << "uid = " << fri.getfriendUid() << "不在线" << endl;
        }
        else
        {
            cout << "uid = " << fri.getfriendUid() << "在线" << endl;
        }
    }
}

// 15屏蔽好友消息或戒除屏蔽
void clnt::blockFriend()
{
    json jn;
    string friendUid, message;
    char buf[BUFSIZ];

    cout << "1、屏蔽好友消息" << endl;
    cout << "2、解除屏蔽" << endl;
    while (!(cin >> message) || (message != "1" && message != "2"))
    {
        if (cin.eof())
        {
            cout << "读到文件结束，函数返回" << endl;
            return;
        }
        cout << "输入有误，清重新输入" << endl;
        cin.clear();
        cin.ignore(INT32_MAX, '\n');
    }
    cin.ignore(INT32_MAX, '\n'); //清空cin缓冲

    cout << "请输入好友的uid，不要超过20个字符" << endl;
    while (!(cin >> friendUid) || friendUid.size() > 20)
    {
        if (cin.eof())
        {
            cout << "读到文件结束，函数返回" << endl;
            return;
        }
        cout << "输入有误" << endl;
        cin.clear();
        cin.ignore(INT32_MAX, '\n');
    }
    cin.ignore(INT32_MAX, '\n'); //清空cin缓冲

    pChat.setNumber(u.getNumber()); //设置自己的uid
    pChat.setFriendUid(friendUid);  //设置好友的uid
    pChat.setMessage(message);      //设置操作

    pChat.To_Json(jn, pChat);

    flag = htonl(flag);
    ssock::SendMsg(clnt_fd, (void *)&flag, sizeof(flag));
    ssock::SendMsg(clnt_fd, jn.dump().c_str(), strlen(jn.dump().c_str()) + 1);

    ssock::ReadMsg(clnt_fd, buf, sizeof(buf));
    if (strcmp(buf, "No user") == 0)
    {
        cout << "没有该好友" << endl;
    }
    else if (strcmp(buf, "Block successfully") == 0)
    {
        cout << "屏蔽成功" << endl;
    }
    else if (strcmp(buf, "No block successfully") == 0)
    {
        cout << "解除屏蔽成功" << endl;
    }
}

void clnt::show_Menu4() // 18私聊
{
    while (1)
    {
        flag = 0;
        cout << "请输入要执行的操作" << endl;
        cout << "17、查看历史聊天记录" << endl;
        cout << "18、和好友聊天" << endl;
        cout << "19、向好友发送文件" << endl;
        cout << "20、返回上一级" << endl;

        while (!(cin >> flag) || flag < 17 || flag > 20)
        {
            if (cin.eof())
            {
                cout << "读到文件结束，函数返回" << endl;
                return;
            }
            cout << "输入有误" << endl;
            cin.clear();
            cin.ignore(INT32_MAX, '\n');
        }
        cin.ignore(INT32_MAX, '\n'); //清空cin缓冲

        pChat.setFlag(flag);
        if (flag == RETURNON2)
        {
            break;
        }
        switch (flag)
        {
        case HISTORY_MESSAGE: //历史聊天记录
            history_message();
            break;
        case CHAT_SEND_FRIEND:
            chat_send_friend(); //和好友聊天
            break;
        case SEND_FILE: //向好友发文件
            break;
        }
    }
}

// 17查看历史聊天记录
void clnt::history_message()
{
    int ret;
    string number;
    json jn;
    char buf[BUFSIZ];

    cout << "请输入你要查看uid，不要超过20个字符" << endl;
    while (!(cin >> number) || number.size() > 20)
    {
        if (cin.eof())
        {
            cout << "读到文件结束，函数返回" << endl;
            return;
        }
        cout << "输入有误或账号过长，请重新输入" << endl;
        cin.clear();
        cin.ignore(INT32_MAX, '\n');
    }
    cin.ignore(INT32_MAX, '\n'); //清空cin缓冲

    pChat.setNumber(u.getNumber()); //设置自己的uid
    pChat.setFriendUid(number);     //设置要查看的历史聊天记录的对方uid

    pChat.To_Json(jn, pChat);

    flag = htonl(flag);
    ssock::SendMsg(clnt_fd, (void *)&flag, sizeof(flag));
    ssock::SendMsg(clnt_fd, (void *)jn.dump().c_str(), strlen(jn.dump().c_str()) + 1);

    while (1)
    {
        ret = ssock::ReadMsg(clnt_fd, buf, sizeof(buf));
        if (ret == 0)
        {
            continue;
        }
        if (strcmp(buf, "finish") == 0)
        {
            break;
        }
        jn = json::parse(buf);
        pChat.From_Json(jn, pChat);
        cout << pChat.getTimeNow()
             << pChat.getNumber() << "："
             << pChat.getMessage() << endl;
    }
}

// 18聊天中发送消息的线程
void clnt::chat_send_friend()
{
    pthread_t tid;
    json jn;
    string message;
    string friendUid;

    cout << "请输入你要发送消息的uid，不要超过20个字符" << endl;
    while (!(cin >> friendUid) || friendUid.size() > 20)
    {
        if (cin.eof())
        {
            cout << "读到文件结束，函数返回" << endl;
            return;
        }
        cout << "输入有误" << endl;
        cin.clear();
        cin.ignore(INT32_MAX, '\n');
    }
    cin.ignore(INT32_MAX, '\n'); //清空cin缓冲

    if (u.getNumber() == friendUid)
    {
        cout << "不可以给自己发消息" << endl;
        return;
    }

    pChat.setFriendUid(friendUid);                                //设置好友的uid
    pChat.setNumber(u.getNumber());                               //设置自己的uid
    pChat.setName(u.getName());                                   //设置自己的姓名
    pChat.setServ_fd(clnt_fd);                                    //设置自己套间字，为了传给接受消息的线程，让接收消息的线程使用
    pthread_create(&tid, NULL, chat_recv_friend, (void *)&pChat); //创建一个接受消息的线程
    pthread_detach(tid);                                          //设置线程分离
    cout << "输入exit退出" << endl;

    flag = htonl(flag);
    ssock::SendMsg(clnt_fd, (void *)&flag, sizeof(flag)); //向服务器发送要执行的操作

    while (1)
    {
        cin >> message;
        cin.ignore(INT32_MAX, '\n'); //清空cin缓冲

        pChat.setTimeNow(); //设置时间
        pChat.setMessage(message);

        pChat.To_Json(jn, pChat); //将类转为序列

        ssock::SendMsg(clnt_fd, jn.dump().c_str(), strlen(jn.dump().c_str()) + 1); //将序列发给服务器
        if (strcmp(message.c_str(), "exit") == 0)
            break;
    }
}

void *chat_recv_friend(void *arg) //聊天中接受好友消息的线程
{
    json jn;
    privateChat pChat = *((privateChat *)arg);

    char buf[BUFSIZ];
    int clnt_fd = pChat.getServ_fd();
    string number = pChat.getFriendUid(); //确保消息是我要聊天的好友发送过来的，如果不是我要聊天的好友发送过来的，就将其添加到聊天的历史记录里面
    int ret;
    while (1)
    {
        ret = ssock::ReadMsg(clnt_fd, buf, sizeof(buf));
        if (ret == 0)
        {
            continue;
        }
        if (strcmp(buf, "No friend") == 0)
        {
            continue;
        }
        // cout << buf << endl;
        jn = json::parse(buf);
        pChat.From_Json(jn, pChat);
        if (strcmp(pChat.getMessage().c_str(), "exit") == 0)
        {
            break;
        }
        if (number == pChat.getNumber())
            cout << pChat.getTimeNow()
                 << pChat.getNumber() << endl
                 << pChat.getMessage() << endl;
        else
        {
            cout << "你收到了" << pChat.getNumber() << "的消息" << endl;
        }
    }

    return NULL;
}

//每隔1s向服务器发送100，然后读取信息
void *continue_receive(void *arg)
{
    privateChat &pChat = *((privateChat *)arg);

    char buf[BUFSIZ];
    json jn;
    privateChat pChat2;
    int clnt_fd = ssock::Socket();
    ssock::Connect(clnt_fd, 9999, "127.0.0.1");

    while (1)
    {
        int ret;
        uint32_t flag = 100;
        flag = htonl(flag);
        ret = ssock::SendMsg(clnt_fd, (void *)&flag, sizeof(flag));
        if (ret == -1)
        {
            close(clnt_fd);
            return NULL;
        }

        //将自己的uid发送给服务器
        ssock::SendMsg(clnt_fd, (void *)pChat.getNumber().c_str(), strlen(pChat.getNumber().c_str()) + 1);
        if (ret == -1)
        {
            close(clnt_fd);
            return NULL;
        }
        ret = ssock::ReadMsg(clnt_fd, buf, sizeof(buf)); //从服务器读数据
        if (ret == 0)
        {
            close(clnt_fd);
            return NULL;
        }
        if (strcmp(buf, "finish") == 0)
        {
            continue;
        }
        jn = json::parse(buf);
        pChat2.From_Json(jn, pChat2);
        if (pChat2.getMessage() == "exit")
        {
            pChat.setFlag(1);
            continue;
        }
        if (pChat.getFlag() != CHAT_SEND_FRIEND) // flag不等于18，即客户端没有进入聊天，其他人发的连天消息写入到一个列表里，客户端通过该线程读取
        {
            cout << "你收到了来自" << pChat2.getNumber() << "的一条消息，请在历史记录中查看" << endl;
        }
    }
}

#endif