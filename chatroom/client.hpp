#ifndef ACCOUNT_H
#define ACCOUNT_H
#include <iostream>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/sendfile.h>
#include <signal.h>
#include "message.hpp"
#include "ssock.hpp"
#include "macro.h"
using namespace std;

void *chat_recv_friend(void *arg);                 //接受好友消息的线程
void *continue_receive(void *arg);                 //持续从服务器读数据的线程
void *pthread_send_file(void *arg);                //专门发文件的线程
unsigned long long htonll(unsigned long long val); //主机序转网络序
unsigned long long ntohll(unsigned long long val); //网络序转主机序
void *pthread_recv_file(void *arg);                //收文件的线程
void *chat_recv_group(void *arg);                  //聊天中接受群消息的线程

char **argv;

class clnt
{
public:
    //菜单
    void show_Menu1(char *arg[]); //登录账号
    void show_Meun2();            //登录后界面
    void show_Menu3();            //好友管理
    void show_Menu4();            //私聊菜单
    void show_Menu5();            //群管理菜单
    void show_Menu6();            //群聊菜单
    int getclntfd();              //获取客户端套间字
    void read_account();          //账号输入
    void login();                 // 1登录
    void reg();                   // 2注册
    void retrieve();              // 3找回密码
    void quit();                  // 4退出
    void signout();               // 9退出登录
    void addFriend();             // 10添加好友
    void inquireAdd();            // 11查看好友添加信息
    void delFriend();             // 12删除好友
    void findFriend();            // 13查询好友
    void onlineStatus();          // 14显示好友在线情况
    void blockFriend();           // 15屏蔽或解除屏蔽好友消息
    void history_message();       // 17历史聊天记录
    void chat_send_friend();      // 18给好友发消息
    void send_file();             // 19发送文件
    void recv_file();             // 20接收文件
    void creategroup();           // 22创建群
    void dissolvegroup();         // 23 解散群
    void joingroup();             // 24加入群
    void quitgroup();             // 25退出群
    void hasjoingroup();          // 26查看已加入的群组
    void groupmembers();          // 27查看群组成员
    void pullmanagepeople();      // 28设置管理员
    void kickmanagepeople();      // 29取消管理员
    void groupapplication();      // 30查看群组申请列表，进行同意或拒绝
    void kickpeople();            // 31踢人
    void history_groupmessage();  // 33查看群历史消息记录
    void chat_send_group();       // 34给群发消息
    void send_file_group();       // 35给群发文件
    void recv_file_group();       // 36接收群文件

private:
    uint32_t flag;     //读取用户输入，保存用户的选项，1登陆，2注册，3找回密码，4退出
    User u;            //用户信息，用来登录或注册
    privateChat pChat; //私聊类，用于发送信息
    int clnt_fd;       //客户端套间字
};

void clnt::show_Menu1(char *arg[])
{
    argv = arg;
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

void setblocksignal()
{
    sigset_t set;

    sigemptyset(&set);
    sigaddset(&set, SIGPIPE);
    sigaddset(&set, SIGBUS);
    sigprocmask(SIG_BLOCK, &set, NULL);
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
    ssock::Connect(clnt_fd, atoi(argv[2]), argv[1]);

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
        setblocksignal();
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
    ssock::Connect(clnt_fd, atoi(argv[2]), argv[1]);

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
    ssock::Connect(clnt_fd, atoi(argv[2]), argv[1]);

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
            show_Menu3();
            break;
        case SHOW_MENU4:
            show_Menu4();
            break;
        case SHOW_MENU5:
            show_Menu5();
            break;
        case SHOW_MENU6:
            show_Menu6();
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
        cout << "等待对方同意" << endl;
    }
}

// 11查询好友添加信息
void clnt::inquireAdd()
{
    char buf[BUFSIZ];
    string message;
    json jn;
    privateChat pChat2;

    flag = htonl(flag);
    ssock::SendMsg(clnt_fd, (void *)&flag, sizeof(flag));
    ssock::SendMsg(clnt_fd, (void *)u.getNumber().c_str(), strlen(u.getNumber().c_str()) + 1);
    while (1)
    {
        ssock::ReadMsg(clnt_fd, (void *)buf, sizeof(buf));
        if (strcmp(buf, "finish") == 0)
        {
            break;
        }
        jn = json::parse(buf);
        pChat2.From_Json(jn, pChat2);

        cout << "你收到了" << pChat2.getNumber() << "的好友申请" << endl;
        cout << "Yes同意/No拒绝" << endl;
        while (!(cin >> message) || (message != "Yes" && message != "No"))
        {
            if (cin.eof())
            {
                cout << "读到文件结束，函数返回" << endl;
                return;
            }
            cout << "输入有误，请重新输入" << endl;
            cin.clear();
            cin.ignore(INT32_MAX, '\n');
        }
        cin.ignore(INT32_MAX, '\n'); //清空cin缓冲
        ssock::SendMsg(clnt_fd, (void *)message.c_str(), strlen(message.c_str()) + 1);
        if (message == "Yes")
        {
            cout << "添加成功" << endl;
        }
        else
        {
            cout << "添加失败" << endl;
        }
    }
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
    onlineUser onlineU;
    friends fri;

    flag = htonl(flag);
    ssock::SendMsg(clnt_fd, (void *)&flag, sizeof(flag));

    pChat.setNumber(u.getNumber()); //设置自己的uid
    pChat.To_Json(jn, pChat);
    ssock::SendMsg(clnt_fd, jn.dump().c_str(), strlen(jn.dump().c_str()) + 1); //向对端发送消息

    //从对端读消息
    while (1)
    {
        ssock::ReadMsg(clnt_fd, (void *)&flag, sizeof(flag));
        ssock::ReadMsg(clnt_fd, buf, sizeof(buf));
        if (strcmp(buf, "finish") == 0)
        {
            break;
        }
        jn = json::parse(buf);
        if (flag == 0)
        {
            fri.From_Json(jn, fri);
            cout << "uid = " << fri.getfriendUid() << "不在线" << endl;
        }
        else
        {

            onlineU.From_Json(jn, onlineU);
            cout << "uid = " << onlineU.getUid() << "在线" << endl;
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

void clnt::show_Menu4()
{
    while (1)
    {
        flag = 0;
        cout << "请输入要执行的操作" << endl;
        cout << "17、查看历史聊天记录" << endl;
        cout << "18、和好友聊天" << endl;
        cout << "19、向好友发送文件" << endl;
        cout << "20、接收好友文件" << endl;
        cout << "21、返回上一级" << endl;

        while (!(cin >> flag) || flag < 17 || flag > 21)
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
            send_file();
            break;
        case RECV_FILE: //接收文件
            recv_file();
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
    privateChat pChat2;

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
        pChat2.From_Json(jn, pChat2);
        cout << pChat2.getTimeNow()
             << pChat2.getNumber() << "："
             << pChat2.getMessage() << endl;
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
        if (cin.eof())
        {
            cout << "读到文件结尾，函数返回" << endl;
            break;
        }

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
    privateChat pChat2;
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
        jn = json::parse(buf);
        pChat2.From_Json(jn, pChat2);
        if (strcmp(pChat2.getMessage().c_str(), "exit") == 0)
        {
            break;
        }
        if (number == pChat2.getNumber())
            cout << pChat2.getTimeNow()
                 << pChat2.getNumber() << endl
                 << pChat2.getMessage() << endl;
    }

    return NULL;
}

// 19发送文件
void clnt::send_file()
{
    string friendUid;
    json jn;
    char buf[BUFSIZ];

    cout << "请输入你要发送文件的好友uid，不要超过20个字符" << endl;
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
        cout << "不可以给自己发文件" << endl;
        return;
    }

    pChat.setFriendUid(friendUid);  //设置好友的uid
    pChat.setNumber(u.getNumber()); //设置自己的uid
    pChat.setName(u.getName());     //设置自己的姓名

    pChat.setTimeNow(); //设置时间

    string file_name;
    cout << "请输入你要发送的文件路径和文件名" << endl;
    while (!(cin >> file_name))
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
    pChat.setMessage(file_name); //设置文件名

    int filefd = open(file_name.c_str(), O_RDONLY);
    if (filefd == -1)
    {
        cout << "文件名或文件路径错误" << endl;
        return;
    }

    flag = htonl(flag);
    ssock::SendMsg(clnt_fd, (void *)&flag, sizeof(flag));
    pChat.To_Json(jn, pChat);                                                  //将类转为序列
    ssock::SendMsg(clnt_fd, jn.dump().c_str(), strlen(jn.dump().c_str()) + 1); //将序列发给服务器

    ssock::ReadMsg(clnt_fd, buf, sizeof(buf));
    if (strcmp(buf, "No friend") == 0)
    {
        cout << "没有该好友" << endl;
        return;
    }
    ssock::ReadMsg(clnt_fd, buf, sizeof(buf));
    if (strcmp(buf, "Has block") == 0)
    {
        return;
    }

    file f;
    f.clnt_sock = clnt_fd;
    f.filefd = filefd;
    pthread_t tid;
    pthread_create(&tid, NULL, pthread_send_file, &f);
    pthread_detach(tid);
}

//发文件的线程
void *pthread_send_file(void *arg)
{
    file *f = (file *)arg;

    __off_t size;
    struct stat file_stat;
    //为了获取文件大小
    fstat(f->filefd, &file_stat);
    size = file_stat.st_size;
    size = htonll(size);
    ssock::SendMsg(f->clnt_sock, (void *)&size, sizeof(size));

    int ret;
    while ((ret = sendfile(f->clnt_sock, f->filefd, NULL, file_stat.st_size)) != 0)
    {
    }
    cout << "发送完毕" << endl;

    close(f->filefd);

    return NULL;
}

// 20接收文件
void clnt::recv_file()
{
    privateChat pChat2;
    json jn;
    char buf[BUFSIZ];
    string message;

    pChat.setNumber(u.getNumber()); //设置自己的uid
    pChat.setName(u.getName());     //设置自己的姓名
    pChat.To_Json(jn, pChat);

    flag = htonl(flag);
    ssock::SendMsg(clnt_fd, (void *)&flag, sizeof(flag));
    ssock::SendMsg(clnt_fd, (void *)jn.dump().c_str(), strlen(jn.dump().c_str()) + 1);

    while (1)
    {
        ssock::ReadMsg(clnt_fd, (void *)buf, sizeof(buf));
        if (strcmp(buf, "finish") == 0)
        {
            break;
        }
        cout << buf << endl;
        jn = json::parse(buf);
        pChat2.From_Json(jn, pChat2);

        string filename(pChat2.getMessage()); //文件名及其文件路径
        cout << filename << endl;
        auto f = filename.rfind('/');
        filename.erase(0, f + 1); //删除文件前缀，只保留文件名
        cout << filename << endl;

        cout << "你收到了" << pChat2.getNumber() << "的文件" << filename << endl;
        cout << "Yes同意接收/No拒绝接收" << endl;
        while (!(cin >> message) || (message != "Yes" && message != "No"))
        {
            if (cin.eof())
            {
                cout << "读到文件结束，函数返回" << endl;
                return;
            }
            cout << "输入有误，请重新输入" << endl;
            cin.clear();
            cin.ignore(INT32_MAX, '\n');
        }
        cin.ignore(INT32_MAX, '\n'); //清空cin缓冲
        ssock::SendMsg(clnt_fd, (void *)message.c_str(), strlen(message.c_str()) + 1);
        if (message == "Yes")
        {
            cout << "接收成功" << endl;
            FILE *fp = fopen(filename.c_str(), "w");
            int n;
            __off_t size;
            ssock::ReadMsg(clnt_fd, (void *)&size, sizeof(size));
            size = ntohll(size);
            while (size > 0)
            {
                if (sizeof(buf) < size)
                {
                    n = ssock::Readn(clnt_fd, buf, sizeof(buf));
                }
                else
                {
                    n = ssock::Readn(clnt_fd, buf, size);
                }
                if (n < 0)
                {
                    continue;
                }
                cout << "size = " << size << endl;
                size -= n;
                fwrite(buf, n, 1, fp);
            }

            fclose(fp);
        }
        else
        {
            cout << "接收失败" << endl;
        }
    }
}

void clnt::show_Menu5()
{
    while (1)
    {
        flag = 0;
        cout << "请输入要执行的操作" << endl;
        cout << "22、创建群" << endl;
        cout << "23、解散群" << endl;
        cout << "24、加入群" << endl;
        cout << "25、退出群" << endl;
        cout << "26、查看已加入的群组" << endl;
        cout << "27、查看群组成员" << endl;
        cout << "28、设置管理员" << endl;
        cout << "29、取消管理员" << endl;
        cout << "30、查看群组成员申请列表" << endl;
        cout << "31、从群组中移除用户" << endl;
        cout << "32、返回上一层" << endl;

        while (!(cin >> flag) || flag < 22 || flag > 32)
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
        if (flag == RETURNON3) // 32返回上一层
        {
            break;
        }
        switch (flag)
        {
        case CREATEGROUP: // 22创建群
            creategroup();
            break;
        case DISSOLVEGROUP: // 23 解散群
            dissolvegroup();
            break;
        case JOINGROUP: // 24加入群
            joingroup();
            break;
        case QUITGROUP: // 25退出群
            quitgroup();
            break;
        case HASJOINGROUP: // 26查看已加入的群组
            hasjoingroup();
            break;
        case GROUPMEMBERS: // 27查看群组成员
            groupmembers();
            break;
        case PULLMANAGEPEOPLE: // 28设置管理员
            pullmanagepeople();
            break;
        case KICKMANAGEPEOPLE: // 29取消管理员
            kickmanagepeople();
            break;
        case GROUPAPPLICATION: // 30、查看群组成员申请列表
            groupapplication();
            break;
        case KICKPEOPLE: // 31、从群组中移除用户
            kickpeople();
            break;
        }
    }
}

void clnt::creategroup() // 22创建群
{
    json jn;
    string groupid;
    cout << "请输入你要创建的群号，请不要超过20个字符，也不要包含空格" << endl;
    while (!(cin >> groupid) || groupid.size() > 20)
    {
        if (cin.eof())
        {
            cout << "读到文件结束，函数返回" << endl;
            return;
        }
        cout << "输入有误，请重新输入" << endl;
        cin.clear();
        cin.ignore(INT32_MAX, '\n');
    }
    cin.ignore(INT32_MAX, '\n'); //清空cin缓冲

    pChat.setNumber(u.getNumber()); //设置自己的uid
    pChat.setName(u.getName());     //设置自己的姓名
    pChat.setFriendUid(groupid);    //设置群号
    pChat.To_Json(jn, pChat);

    flag = htonl(flag); //发送要进行的操作
    ssock::SendMsg(clnt_fd, (void *)&flag, sizeof(flag));
    ssock::SendMsg(clnt_fd, jn.dump().c_str(), strlen(jn.dump().c_str()) + 1);

    char buf[10];
    //从服务器读，看是否注册群成功

    ssock::ReadMsg(clnt_fd, buf, sizeof(buf));

    if (strcmp(buf, "No") == 0)
    {
        cout << "创建失败，群已经存在！" << endl;
    }
    else if (strcmp(buf, "Yes") == 0)
    {
        cout << "创建成功！" << endl;
    }
}

void clnt::dissolvegroup() // 23解散群
{
    json jn;
    string groupid;
    cout << "请输入你要解散的群号，请不要超过20个字符，也不要包含空格" << endl;
    while (!(cin >> groupid) || groupid.size() > 20)
    {
        if (cin.eof())
        {
            cout << "读到文件结束，函数返回" << endl;
            return;
        }
        cout << "输入有误，请重新输入" << endl;
        cin.clear();
        cin.ignore(INT32_MAX, '\n');
    }
    cin.ignore(INT32_MAX, '\n'); //清空cin缓冲

    pChat.setNumber(u.getNumber()); //设置自己的uid
    pChat.setName(u.getName());     //设置自己的姓名
    pChat.setFriendUid(groupid);    //设置群号
    pChat.To_Json(jn, pChat);

    flag = htonl(flag); //发送要进行的操作
    ssock::SendMsg(clnt_fd, (void *)&flag, sizeof(flag));
    ssock::SendMsg(clnt_fd, jn.dump().c_str(), strlen(jn.dump().c_str()) + 1);

    char buf[50];
    //从服务器读，看是否解散群成功

    ssock::ReadMsg(clnt_fd, buf, sizeof(buf));

    if (strcmp(buf, "No group") == 0)
    {
        cout << "解散失败，没有该群" << endl;
    }
    else if (strcmp(buf, "you are not a member of this group") == 0)
    {
        cout << "你不是该群成员" << endl;
    }
    else if (strcmp(buf, "not the group owner") == 0)
    {
        cout << "你不是该群群主" << endl;
    }
    else if (strcmp(buf, "Yes") == 0)
    {
        cout << "解散成功" << endl;
    }
}

// 24加入群
void clnt::joingroup()
{
    json jn;
    string groupid;
    cout << "请输入你要加入的群号，请不要超过20个字符，也不要包含空格" << endl;
    while (!(cin >> groupid) || groupid.size() > 20)
    {
        if (cin.eof())
        {
            cout << "读到文件结束，函数返回" << endl;
            return;
        }
        cout << "输入有误，请重新输入" << endl;
        cin.clear();
        cin.ignore(INT32_MAX, '\n');
    }
    cin.ignore(INT32_MAX, '\n'); //清空cin缓冲

    pChat.setNumber(u.getNumber()); //设置自己的uid
    pChat.setName(u.getName());     //设置自己的姓名
    pChat.setFriendUid(groupid);    //设置群号
    pChat.To_Json(jn, pChat);

    flag = htonl(flag); //发送要进行的操作
    ssock::SendMsg(clnt_fd, (void *)&flag, sizeof(flag));
    ssock::SendMsg(clnt_fd, jn.dump().c_str(), strlen(jn.dump().c_str()) + 1);

    char buf[50];
    //从服务器读，看是否解散群成功

    ssock::ReadMsg(clnt_fd, buf, sizeof(buf));

    if (strcmp(buf, "No group") == 0)
    {
        cout << "加入失败，没有该群" << endl;
    }
    else if (strcmp(buf, "you are already a member of this group") == 0)
    {
        cout << "你已经是该群成员" << endl;
    }
    else if (strcmp(buf, "Yes") == 0)
    {
        cout << "申请成功，等待管理员审核" << endl;
    }
}

// 25退出群
void clnt::quitgroup()
{
    json jn;
    string groupid;
    cout << "请输入你要退出的群号，请不要超过20个字符，也不要包含空格" << endl;
    while (!(cin >> groupid) || groupid.size() > 20)
    {
        if (cin.eof())
        {
            cout << "读到文件结束，函数返回" << endl;
            return;
        }
        cout << "输入有误，请重新输入" << endl;
        cin.clear();
        cin.ignore(INT32_MAX, '\n');
    }
    cin.ignore(INT32_MAX, '\n'); //清空cin缓冲

    pChat.setNumber(u.getNumber()); //设置自己的uid
    pChat.setName(u.getName());     //设置自己的姓名
    pChat.setFriendUid(groupid);    //设置群号
    pChat.To_Json(jn, pChat);

    flag = htonl(flag); //发送要进行的操作
    ssock::SendMsg(clnt_fd, (void *)&flag, sizeof(flag));
    ssock::SendMsg(clnt_fd, jn.dump().c_str(), strlen(jn.dump().c_str()) + 1);

    char buf[50];
    //从服务器读，看是否解散群成功

    ssock::ReadMsg(clnt_fd, buf, sizeof(buf));

    if (strcmp(buf, "No group") == 0)
    {
        cout << "退出失败，没有该群" << endl;
    }
    else if (strcmp(buf, "you are not a member of this group") == 0)
    {
        cout << "你不是该群成员" << endl;
    }
    else if (strcmp(buf, "you are the leader") == 0)
    {
        cout << "你是群主，不能退群" << endl;
    }
    else if (strcmp(buf, "Yes") == 0)
    {
        cout << "退出成功" << endl;
    }
}

// 26查看已加入的群组
void clnt::hasjoingroup()
{
    json jn;
    groups grps;

    pChat.setNumber(u.getNumber()); //设置自己的uid
    pChat.setName(u.getName());     //设置自己的姓名
    pChat.To_Json(jn, pChat);

    flag = htonl(flag); //发送要进行的操作
    ssock::SendMsg(clnt_fd, (void *)&flag, sizeof(flag));
    ssock::SendMsg(clnt_fd, jn.dump().c_str(), strlen(jn.dump().c_str()) + 1);

    char buf[BUFSIZ];
    //从服务器读自己加入的群
    while (1)
    {
        ssock::ReadMsg(clnt_fd, buf, sizeof(buf));
        if (strcmp(buf, "finish") == 0)
        {
            break;
        }
        jn = json::parse(buf);
        grps.From_Json(jn, grps);
        if (grps.getflag() == 0)
            cout << "groupid：" << grps.getgroup_number() << "，权限："
                 << "普通成员" << endl;
        if (grps.getflag() == 1)
            cout << "groupid：" << grps.getgroup_number() << "，权限："
                 << "管理员" << endl;
        if (grps.getflag() == 2)
            cout << "groupid：" << grps.getgroup_number() << "，权限："
                 << "群主" << endl;
    }
}

// 27查看群组成员
void clnt::groupmembers()
{
    json jn;
    string groupid;
    groups grps;

    cout << "请输入你要查看的群号，请不要超过20个字符，也不要包含空格" << endl;
    while (!(cin >> groupid) || groupid.size() > 20)
    {
        if (cin.eof())
        {
            cout << "读到文件结束，函数返回" << endl;
            return;
        }
        cout << "输入有误，请重新输入" << endl;
        cin.clear();
        cin.ignore(INT32_MAX, '\n');
    }
    cin.ignore(INT32_MAX, '\n'); //清空cin缓冲

    pChat.setNumber(u.getNumber()); //设置自己的uid
    pChat.setName(u.getName());     //设置自己的姓名
    pChat.setFriendUid(groupid);    //设置群号
    pChat.To_Json(jn, pChat);

    flag = htonl(flag); //发送要进行的操作
    ssock::SendMsg(clnt_fd, (void *)&flag, sizeof(flag));
    ssock::SendMsg(clnt_fd, jn.dump().c_str(), strlen(jn.dump().c_str()) + 1);

    char buf[BUFSIZ];

    ssock::ReadMsg(clnt_fd, buf, sizeof(buf));
    if (strcmp(buf, "No group") == 0)
    {
        cout << "该群不存在" << endl;
        return;
    }
    else if (strcmp(buf, "you are not a member of this group") == 0)
    {
        cout << "你不是该群成员" << endl;
        return;
    }
    //从服务器读群成员
    while (1)
    {
        ssock::ReadMsg(clnt_fd, buf, sizeof(buf));
        if (strcmp(buf, "finish") == 0)
        {
            break;
        }

        jn = json::parse(buf);
        grps.From_Json(jn, grps);
        if (grps.getflag() == 0)
            cout << "uid：" << grps.getuser_number() << "，权限："
                 << "普通成员" << endl;
        if (grps.getflag() == 1)
            cout << "uid：" << grps.getuser_number() << "，权限："
                 << "管理员" << endl;
        if (grps.getflag() == 2)
            cout << "uid：" << grps.getuser_number() << "，权限："
                 << "群主" << endl;
    }
}

void clnt::pullmanagepeople() // 28设置管理员
{
    json jn;
    string groupid, uid;
    groups grps;

    cout << "请输入你要设置管理员的群号，请不要超过20个字符，也不要包含空格" << endl;
    while (!(cin >> groupid) || groupid.size() > 20)
    {
        if (cin.eof())
        {
            cout << "读到文件结束，函数返回" << endl;
            return;
        }
        cout << "输入有误，请重新输入" << endl;
        cin.clear();
        cin.ignore(INT32_MAX, '\n');
    }
    cin.ignore(INT32_MAX, '\n'); //清空cin缓冲

    cout << "请输入你要设置的管理员id，请不要超过20个字符，也不要包含空格" << endl;
    while (!(cin >> uid) || uid.size() > 20)
    {
        if (cin.eof())
        {
            cout << "读到文件结束，函数返回" << endl;
            return;
        }
        cout << "输入有误，请重新输入" << endl;
        cin.clear();
        cin.ignore(INT32_MAX, '\n');
    }
    cin.ignore(INT32_MAX, '\n'); //清空cin缓冲

    pChat.setNumber(u.getNumber()); //设置自己的uid
    pChat.setName(u.getName());     //设置自己的姓名
    pChat.setFriendUid(groupid);    //设置群号
    pChat.setMessage(uid);          //设置管理员id
    pChat.To_Json(jn, pChat);
    if (uid == u.getNumber())
    {
        cout << "不可以修改自己的权限" << endl;
        return;
    }

    flag = htonl(flag); //发送要进行的操作
    ssock::SendMsg(clnt_fd, (void *)&flag, sizeof(flag));
    ssock::SendMsg(clnt_fd, jn.dump().c_str(), strlen(jn.dump().c_str()) + 1);

    char buf[BUFSIZ];

    ssock::ReadMsg(clnt_fd, buf, sizeof(buf));
    if (strcmp(buf, "No group") == 0)
    {
        cout << "该群不存在" << endl;
    }
    else if (strcmp(buf, "you are not a member of this group") == 0)
    {
        cout << "你不是该群成员" << endl;
    }
    else if (strcmp(buf, "not the group owner") == 0)
    {
        cout << "你不是该群群主" << endl;
    }
    else if (strcmp(buf, "No user") == 0)
    {
        cout << "该账号未加入该群" << endl;
    }
    else if (strcmp(buf, "Yes") == 0)
    {
        cout << "设置成功" << endl;
    }
}

// 29取消管理员
void clnt::kickmanagepeople()
{
    json jn;
    string groupid, uid;
    groups grps;

    cout << "请输入你要取消管理员的群号，请不要超过20个字符，也不要包含空格" << endl;
    while (!(cin >> groupid) || groupid.size() > 20)
    {
        if (cin.eof())
        {
            cout << "读到文件结束，函数返回" << endl;
            return;
        }
        cout << "输入有误，请重新输入" << endl;
        cin.clear();
        cin.ignore(INT32_MAX, '\n');
    }
    cin.ignore(INT32_MAX, '\n'); //清空cin缓冲

    cout << "请输入你要取消的管理员id，请不要超过20个字符，也不要包含空格" << endl;
    while (!(cin >> uid) || uid.size() > 20)
    {
        if (cin.eof())
        {
            cout << "读到文件结束，函数返回" << endl;
            return;
        }
        cout << "输入有误，请重新输入" << endl;
        cin.clear();
        cin.ignore(INT32_MAX, '\n');
    }
    cin.ignore(INT32_MAX, '\n'); //清空cin缓冲

    pChat.setNumber(u.getNumber()); //设置自己的uid
    pChat.setName(u.getName());     //设置自己的姓名
    pChat.setFriendUid(groupid);    //设置群号
    pChat.setMessage(uid);          //设置管理员id
    pChat.To_Json(jn, pChat);
    if (uid == u.getNumber())
    {
        cout << "不可以修改自己的权限" << endl;
        return;
    }

    flag = htonl(flag); //发送要进行的操作
    ssock::SendMsg(clnt_fd, (void *)&flag, sizeof(flag));
    ssock::SendMsg(clnt_fd, jn.dump().c_str(), strlen(jn.dump().c_str()) + 1);

    char buf[BUFSIZ];

    ssock::ReadMsg(clnt_fd, buf, sizeof(buf));
    if (strcmp(buf, "No group") == 0)
    {
        cout << "该群不存在" << endl;
    }
    else if (strcmp(buf, "you are not a member of this group") == 0)
    {
        cout << "你不是该群成员" << endl;
    }
    else if (strcmp(buf, "not the group owner") == 0)
    {
        cout << "你不是该群群主" << endl;
    }
    else if (strcmp(buf, "No user") == 0)
    {
        cout << "该账号未加入该群" << endl;
    }
    else if (strcmp(buf, "Yes") == 0)
    {
        cout << "取消成功" << endl;
    }
}

// 30查看群组申请列表，进行同意或拒绝
void clnt::groupapplication()
{
    json jn;
    string groupid;
    string mmm;
    groups grps;
    privateChat pChat2;

    cout << "请输入你要批阅群申请的群号，请不要超过20个字符，也不要包含空格" << endl;
    while (!(cin >> groupid) || groupid.size() > 20)
    {
        if (cin.eof())
        {
            cout << "读到文件结束，函数返回" << endl;
            return;
        }
        cout << "输入有误，请重新输入" << endl;
        cin.clear();
        cin.ignore(INT32_MAX, '\n');
    }
    cin.ignore(INT32_MAX, '\n'); //清空cin缓冲

    pChat.setNumber(u.getNumber()); //设置自己的uid
    pChat.setName(u.getName());     //设置自己的姓名
    pChat.setFriendUid(groupid);    //设置群号
    pChat.To_Json(jn, pChat);

    flag = htonl(flag); //发送要进行的操作
    ssock::SendMsg(clnt_fd, (void *)&flag, sizeof(flag));
    ssock::SendMsg(clnt_fd, jn.dump().c_str(), strlen(jn.dump().c_str()) + 1);

    char buf[BUFSIZ];

    ssock::ReadMsg(clnt_fd, buf, sizeof(buf));
    if (strcmp(buf, "No group") == 0)
    {
        cout << "该群不存在" << endl;
        return;
    }
    else if (strcmp(buf, "you are not a member of this group") == 0)
    {
        cout << "你不是该群成员" << endl;
        return;
    }
    else if (strcmp(buf, "You can't agree to someone else's friend request") == 0)
    {
        cout << "你只是普通成员，不能同意别人的群申请" << endl;
        return;
    }
    else if (strcmp(buf, "No one has applied to join the group") == 0)
    {
        cout << "没有人申请加入群" << endl;
        return;
    }

    jn = json::parse(buf);
    pChat2.From_Json(jn, pChat2);
    cout << "你是群" << pChat2.getFriendUid() << "的管理员"
         << "，你收到了" << pChat2.getNumber() << "的加群申请" << endl;
    printf("Yes同意/No拒绝\n");
    while (!(cin >> mmm) || (mmm != "Yes" && mmm != "No"))
    {
        if (cin.eof())
        {
            cout << "读到文件结束，函数返回" << endl;
            return;
        }
        cout << "输入有误，请重新输入" << endl;
        cin.clear();
        cin.ignore(INT32_MAX, '\n');
    }
    cin.ignore(INT32_MAX, '\n'); //清空cin缓冲
    ssock ::SendMsg(clnt_fd, mmm.c_str(), strlen(mmm.c_str()) + 1);
    cout << "操作成功" << endl;
}

// 31踢人
void clnt::kickpeople()
{
    json jn;
    string groupid, uid;
    groups grps;

    cout << "请输入你要踢人的群号，请不要超过20个字符，也不要包含空格" << endl;
    while (!(cin >> groupid) || groupid.size() > 20)
    {
        if (cin.eof())
        {
            cout << "读到文件结束，函数返回" << endl;
            return;
        }
        cout << "输入有误，请重新输入" << endl;
        cin.clear();
        cin.ignore(INT32_MAX, '\n');
    }
    cin.ignore(INT32_MAX, '\n'); //清空cin缓冲

    cout << "请输入你要踢的人的id，请不要超过20个字符，也不要包含空格" << endl;
    while (!(cin >> uid) || uid.size() > 20)
    {
        if (cin.eof())
        {
            cout << "读到文件结束，函数返回" << endl;
            return;
        }
        cout << "输入有误，请重新输入" << endl;
        cin.clear();
        cin.ignore(INT32_MAX, '\n');
    }
    cin.ignore(INT32_MAX, '\n'); //清空cin缓冲

    pChat.setNumber(u.getNumber()); //设置自己的uid
    pChat.setName(u.getName());     //设置自己的姓名
    pChat.setFriendUid(groupid);    //设置群号
    pChat.setMessage(uid);          //设置管理员id
    pChat.To_Json(jn, pChat);
    if (uid == u.getNumber())
    {
        cout << "不可以踢自己，只能踢其他群成员" << endl;
        return;
    }

    flag = htonl(flag); //发送要进行的操作
    ssock::SendMsg(clnt_fd, (void *)&flag, sizeof(flag));
    ssock::SendMsg(clnt_fd, jn.dump().c_str(), strlen(jn.dump().c_str()) + 1);

    char buf[BUFSIZ];

    ssock::ReadMsg(clnt_fd, buf, sizeof(buf));
    if (strcmp(buf, "No group") == 0)
    {
        cout << "该群不存在" << endl;
    }
    else if (strcmp(buf, "you are not a member of this group") == 0)
    {
        cout << "你不是该群成员" << endl;
    }
    else if (strcmp(buf, "you can't kick people") == 0)
    {
        cout << "你只是普通成员，不能踢人" << endl;
    }
    else if (strcmp(buf, "No user") == 0)
    {
        cout << "该账号未加入该群" << endl;
    }
    else if (strcmp(buf, "Failed to kick") == 0)
    {
        cout << "踢人失败，你要踢的人也是管理员或是群主" << endl;
    }
    else if (strcmp(buf, "Yes") == 0)
    {
        cout << "踢人成功" << endl;
    }
}
void clnt::show_Menu6()
{
    while (1)
    {
        flag = 0;
        cout << "请输入要执行的操作" << endl;
        cout << "33、查看群历史聊天记录" << endl;
        cout << "34、给群发消息" << endl;
        cout << "35、给群发送文件" << endl;
        cout << "36、接收群文件" << endl;
        cout << "37、返回上一层" << endl;

        while (!(cin >> flag) || flag < 33 || flag > 37)
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
        if (flag == RETURNON4) // 37返回上一层
        {
            break;
        }
        switch (flag)
        {
        case HISTORY_GROUPMESSAGE: // 33查看群历史消息记录
            history_groupmessage();
            break;
        case CHAT_SEND_GROUP: // 34给群发消息
            chat_send_group();
            break;
        case SEND_FILE_GROUP: // 35给群发文件
            send_file_group();
            break;
        case RECV_FILE_GROUP: // 36接收群文件
            recv_file_group();
            break;
        }
    }
}

void clnt::history_groupmessage() // 33查看群历史消息记录
{
    int ret;
    string number;
    json jn;
    char buf[BUFSIZ];
    privateChat pChat2;

    cout << "请输入你要查看的群id，不要超过20个字符" << endl;
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
        pChat2.From_Json(jn, pChat2);
        cout << pChat2.getTimeNow()
             << pChat2.getNumber() << "："
             << pChat2.getMessage() << endl;
    }
}
void clnt::chat_send_group() // 34给群发消息
{
    pthread_t tid;
    json jn;
    string message;
    string groupUid;

    cout << "请输入你要发送消息的群id，不要超过20个字符，也不要含有空格" << endl;
    while (!(cin >> groupUid) || groupUid.size() > 20)
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

    pChat.setFriendUid(groupUid);                                //设置群的id
    pChat.setNumber(u.getNumber());                              //设置自己的uid
    pChat.setName(u.getName());                                  //设置自己的姓名
    pChat.setServ_fd(clnt_fd);                                   //设置自己套间字，为了传给接受消息的线程，让接收消息的线程使用
    pthread_create(&tid, NULL, chat_recv_group, (void *)&pChat); //创建一个接受消息的线程
    pthread_detach(tid);                                         //设置线程分离
    cout << "输入exit退出" << endl;

    flag = htonl(flag);
    ssock::SendMsg(clnt_fd, (void *)&flag, sizeof(flag)); //向服务器发送要执行的操作

    while (1)
    {
        cin >> message;
        if (cin.eof())
        {
            cout << "读到文件结尾，函数返回" << endl;
            break;
        }

        pChat.setTimeNow(); //设置时间
        pChat.setMessage(message);

        pChat.To_Json(jn, pChat); //将类转为序列

        ssock::SendMsg(clnt_fd, jn.dump().c_str(), strlen(jn.dump().c_str()) + 1); //将序列发给服务器
        if (strcmp(message.c_str(), "exit") == 0)
            break;
    }
}

void *chat_recv_group(void *arg) //聊天中接受群消息的线程
{
    privateChat pChat2;
    json jn;
    privateChat pChat = *((privateChat *)arg);

    char buf[BUFSIZ];
    int clnt_fd = pChat.getServ_fd();
    string number = pChat.getFriendUid(); //确保消息是我要聊天的群发送过来的，如果不是我要聊天的群发送过来的，就将其添加到聊天的历史记录里面
    int ret;
    while (1)
    {
        ret = ssock::ReadMsg(clnt_fd, buf, sizeof(buf));
        if (ret == 0)
        {
            continue;
        }
        if (strcmp(buf, "No group") == 0)
        {
            continue;
        }
        else if (strcmp(buf, "you are not a member of this group") == 0)
        {
            continue;
        }
        jn = json::parse(buf);
        pChat2.From_Json(jn, pChat2);
        if (strcmp(pChat2.getMessage().c_str(), "exit") == 0)
        {
            break;
        }
        if (number == pChat2.getFriendUid())
            cout << pChat2.getTimeNow()
                 << pChat2.getNumber() << endl
                 << pChat2.getMessage() << endl;
    }

    return NULL;
}
void clnt::send_file_group() // 35给群发文件
{
    string groupUid;
    json jn;
    char buf[BUFSIZ];

    cout << "请输入你要发送文件的群uid，不要超过20个字符" << endl;
    while (!(cin >> groupUid) || groupUid.size() > 20)
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

    pChat.setFriendUid(groupUid);   //设置群的uid
    pChat.setNumber(u.getNumber()); //设置自己的uid
    pChat.setName(u.getName());     //设置自己的姓名

    pChat.setTimeNow(); //设置时间

    string file_name;
    cout << "请输入你要发送的文件路径和文件名" << endl;
    while (!(cin >> file_name))
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
    pChat.setMessage(file_name); //设置文件名

    int filefd = open(file_name.c_str(), O_RDONLY);
    if (filefd == -1)
    {
        cout << "文件名或文件路径错误" << endl;
        return;
    }

    flag = htonl(flag);
    ssock::SendMsg(clnt_fd, (void *)&flag, sizeof(flag));
    pChat.To_Json(jn, pChat);                                                  //将类转为序列
    ssock::SendMsg(clnt_fd, jn.dump().c_str(), strlen(jn.dump().c_str()) + 1); //将序列发给服务器

    ssock::ReadMsg(clnt_fd, buf, sizeof(buf));
    if (strcmp(buf, "No Group") == 0)
    {
        cout << "没有该群" << endl;
        return;
    }
    if (strcmp(buf, "you are not a member of this group") == 0)
    {
        cout << "你不是该群成员" << endl;
        return;
    }

    __off_t size;
    struct stat file_stat;
    //为了获取文件大小
    fstat(filefd, &file_stat);
    size = file_stat.st_size;
    size = htonll(size);
    ssock::SendMsg(clnt_fd, (void *)&size, sizeof(size));

    int ret;
    while ((ret = sendfile(clnt_fd, filefd, NULL, file_stat.st_size)) != 0)
    {
    }
    cout << "发送完毕" << endl;

    close(filefd);
}
void clnt::recv_file_group() // 36接收群文件
{
    privateChat pChat2;
    json jn;
    char buf[BUFSIZ];
    string message;

    pChat.setNumber(u.getNumber()); //设置自己的uid
    pChat.setName(u.getName());     //设置自己的姓名
    pChat.To_Json(jn, pChat);

    flag = htonl(flag);
    ssock::SendMsg(clnt_fd, (void *)&flag, sizeof(flag));
    ssock::SendMsg(clnt_fd, (void *)jn.dump().c_str(), strlen(jn.dump().c_str()) + 1);

    while (1)
    {
        ssock::ReadMsg(clnt_fd, (void *)buf, sizeof(buf));
        if (strcmp(buf, "finish") == 0)
        {
            break;
        }
        cout << buf << endl;
        jn = json::parse(buf);
        pChat2.From_Json(jn, pChat2);

        string filename(pChat2.getMessage()); //文件名及其文件路径
        cout << filename << endl;
        auto f = filename.rfind('/');
        filename.erase(0, f + 1); //删除文件前缀，只保留文件名
        cout << filename << endl;

        cout << "你收到了" << pChat2.getNumber() << "的文件" << filename << endl;
        cout << "Yes同意接收/No拒绝接收" << endl;
        while (!(cin >> message) || (message != "Yes" && message != "No"))
        {
            if (cin.eof())
            {
                cout << "读到文件结束，函数返回" << endl;
                return;
            }
            cout << "输入有误，请重新输入" << endl;
            cin.clear();
            cin.ignore(INT32_MAX, '\n');
        }
        cin.ignore(INT32_MAX, '\n'); //清空cin缓冲
        ssock::SendMsg(clnt_fd, (void *)message.c_str(), strlen(message.c_str()) + 1);
        if (message == "Yes")
        {
            cout << "接收成功" << endl;
            FILE *fp = fopen(filename.c_str(), "w");
            int n;
            __off_t size;
            ssock::ReadMsg(clnt_fd, (void *)&size, sizeof(size));
            size = ntohll(size);
            while (size > 0)
            {
                if (sizeof(buf) < size)
                {
                    n = ssock::Readn(clnt_fd, buf, sizeof(buf));
                }
                else
                {
                    n = ssock::Readn(clnt_fd, buf, size);
                }
                if (n < 0)
                {
                    continue;
                }
                cout << "size = " << size << endl;
                size -= n;
                fwrite(buf, n, 1, fp);
            }

            fclose(fp);
        }
        else
        {
            cout << "接收失败" << endl;
        }
    }
}

//向服务器发送100，然后读取信息
void *continue_receive(void *arg)
{
    privateChat &pChat = *((privateChat *)arg);

    char buf[BUFSIZ];
    json jn;
    privateChat pChat2;
    int clnt_fd = ssock::Socket();
    ssock::Connect(clnt_fd, atoi(argv[2]), argv[1]);

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
            memset(buf, 0, sizeof(buf));
            continue;
        }
        jn = json::parse(buf);
        pChat2.From_Json(jn, pChat2);
        if (pChat2.getMessage() == "exit")
        {
            pChat.setFlag(1);
            continue;
        }
        if (pChat2.getFlag() == ADDFRIEND)
        {
            cout << "你收到了来自" << pChat2.getNumber() << "的好友申请，请在申请列表查看" << endl;
            continue;
        }
        if (pChat2.getFlag() == SEND_FILE)
        {
            cout << "你收到了来自" << pChat2.getNumber() << "的文件，请去同意或拒绝" << endl;
            continue;
        }
        if (pChat2.getFlag() == JOINGROUP)
        {
            cout << "你是群" << pChat2.getFriendUid() << "的管理员"
                 << "，你收到了" << pChat2.getNumber() << "的加群申请" << endl;
            continue;
        }
        if (pChat2.getFlag() == SEND_FILE_GROUP)
        {
            cout << "你收到了来自群" << pChat2.getFriendUid() << "的文件，请去同意或拒绝" << endl;
            continue;
        }
        if (pChat2.getFlag() == CHAT_SEND_FRIEND)
        {
            if (pChat.getFlag() == CHAT_SEND_FRIEND)
            {
                if (pChat2.getNumber() == pChat.getFriendUid())
                {
                    continue;
                }
            }
            cout << "你收到了来自" << pChat2.getNumber() << "的一条消息，请在历史记录中查看" << endl;
        }
        if (pChat2.getFlag() == CHAT_SEND_GROUP)
        {
            if (pChat.getFlag() == CHAT_SEND_GROUP)
            {
                if (pChat2.getFriendUid() == pChat.getFriendUid())
                {
                    continue;
                }
            }
            cout << "你收到了来自群" << pChat2.getFriendUid() << "的一条消息，请在历史记录中查看" << endl;
        }
    }
}

//主机序转网络序
unsigned long long htonll(unsigned long long val)
{
    if (__BYTE_ORDER == __LITTLE_ENDIAN)
    {
        return (((unsigned long long)htonl((int)((val << 32) >> 32))) << 32) | (unsigned int)htonl((int)(val >> 32));
    }
    else if (__BYTE_ORDER == __BIG_ENDIAN)
    {
        return val;
    }
}

//网络序转主机序
unsigned long long ntohll(unsigned long long val)
{
    if (__BYTE_ORDER == __LITTLE_ENDIAN)
    {
        return (((unsigned long long)ntohl((int)((val << 32) >> 32))) << 32) | (unsigned int)ntohl((int)(val >> 32));
    }
    else if (__BYTE_ORDER == __BIG_ENDIAN)
    {
        return val;
    }
}

#endif