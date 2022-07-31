#ifndef SERVFRIEND_H
#define SERVFRIEND_H
#include <iostream>
#include <sstream>
#include <cstring>
#include <string>
#include <set>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "ssock.hpp"
#include "redis1.hpp"
#include <hiredis/hiredis.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/sendfile.h>
#include <fcntl.h>
#include <errno.h>
#include "message.hpp"
#include "redis1.hpp"
#include "threadpool.hpp"
#include "macro.h"
using namespace std;

unsigned long long htonll(unsigned long long val); //主机序转网络序
unsigned long long ntohll(unsigned long long val); //网络序转主机序

void qqqqquit(int clnt_sock) //将其从在线用户中删除
{
    int ret = close(clnt_sock);
    printf("closeret = %d，%d关闭\n", ret, clnt_sock);
    json jn;
    onlineUser onlineU;
    redisContext *c = Redis::RedisConnect("127.0.0.1", 6379);
    redisReply *r = Redis::hgethashall(c, "Onlineuser"); //获取所有在线用户
    if (r == NULL)
    {
        printf("Execut getValue failure\n");
        redisFree(c);
        return;
    }
    for (int i = 0; i < r->elements; ++i) //遍历在线用户
    {
        if (i % 2 == 1) //偶数是key，奇数是value
        {
            jn = json::parse(r->element[i]->str); //将获得的value序列化
            onlineU.From_Json(jn, onlineU);
            if (onlineU.getsock() == clnt_sock) //将value中的套间字和clnt_sock比较，相同就删除该用户
            {
                freeReplyObject(r);
                r = Redis::hashdel(c, "Onlineuser", onlineU.getUid());
                if (r == NULL)
                {
                    printf("Execut getValue failure\n");

                    redisFree(c);
                    return;
                }
                if (r->integer == 0)
                {
                    cout << "下线失败" << endl;
                }
                break;
            }
        }
    }

    freeReplyObject(r);
    redisFree(c);
}

class gay
{
public:
    gay();
    ~gay();
    void setEfd(const int &e); //设置监听树根
    void signout();            // 9退出登录
    void addFriend();          // 10 添加好友
    void inquqireAdd();        // 11查询好友添加信息
    void delFriend();          // 12删除好友
    void findFriend();         // 13查询好友
    void onlineStatus();       // 14显示好友在线信息
    void blockFriend();        // 15屏蔽号有消息

    privateChat &getpChat();     //得到pChat对象
    pthread_mutex_t &getMutex(); //获得锁
    int getefd();                //获得根节点
    void history_message();      // 17查看历史聊天记录
    void talkwithfriends();      // 18和朋友聊天
    void send_file();            // 19从客户端读文件
    void recv_file();            // 20向客户端发文件
    void creategroup();          // 22创建群
    void dissolvegroup();        // 23 解散群
    void joingroup();            // 24加入群
    void quitgroup();            // 25退出群
    void hasjoingroup();         // 26查看已加入的群组
    void groupmembers();         // 27查看群组成员
    void pullmanagepeople();     // 28设置管理员
    void kickmanagepeople();     // 29取消管理员
    void groupapplication();     // 30查看群组申请列表，进行同意或拒绝
    void kickpeople();           // 31踢人
    void history_groupmessage(); // 33查看群历史消息记录
    void chat_send_group();      // 34给群发消息
    void send_file_group();      // 35给群发文件
    void recv_file_group();      // 36接收群文件
    void send_part_file();       // 38接收未接收完的文件

private:
    pthread_mutex_t mutex;
    int efd;
    privateChat pChat;
};

gay::gay()
{
    pthread_mutex_init(&mutex, NULL);
}

gay::~gay()
{
    pthread_mutex_destroy(&mutex);
}

void startpchat(void *arg)
{
    gay g = *((gay *)arg);
    pthread_mutex_unlock(&(((gay *)arg)->getMutex()));

    cout << g.getpChat().getFlag() << endl;
    switch (g.getpChat().getFlag())
    {
    case SIGNOUT:
        g.signout(); // 9退出登录
        break;
    case ADDFRIEND:
        g.addFriend(); // 10添加好友
        break;
    case INQUIREADD: // 11查看好友添加信息
        g.inquqireAdd();
        break;
    case DELFRIEND: // 12删除好友
        g.delFriend();
        break;
    case FINDFRIEND: // 13查询好友
        g.findFriend();
        break;
    case ONLINESTATUS: // 14显示好友在线状态
        g.onlineStatus();
        break;
    case BLOCKFRIEND: // 15屏蔽好友消息
        g.blockFriend();
        break;
    case HISTORY_MESSAGE: // 17查看历史聊天记录
        g.history_message();
        break;
    case CHAT_SEND_FRIEND: // 18和好友聊天
        g.talkwithfriends();
        break;
    case SEND_FILE: // 19接收客户端的文件
        g.send_file();
        break;
    case RECV_FILE: // 20向客户端发文件
        g.recv_file();
        break;
    case CREATEGROUP: // 22创建群
        g.creategroup();
        break;
    case DISSOLVEGROUP: // 23 解散群
        g.dissolvegroup();
        break;
    case JOINGROUP: // 24加入群
        g.joingroup();
        break;
    case QUITGROUP: // 25退出群
        g.quitgroup();
        break;
    case HASJOINGROUP: // 26查看已加入的群组
        g.hasjoingroup();
        break;
    case GROUPMEMBERS: // 27查看群组成员
        g.groupmembers();
        break;
    case PULLMANAGEPEOPLE: // 28设置管理员
        g.pullmanagepeople();
        break;
    case KICKMANAGEPEOPLE: // 29取消管理员
        g.kickmanagepeople();
        break;
    case GROUPAPPLICATION: // 30查看群组申请列表，进行同意或拒绝
        g.groupapplication();
        break;
    case KICKPEOPLE:
        g.kickpeople(); // 31踢人
        break;
    case HISTORY_GROUPMESSAGE: // 33查看群历史消息记录
        g.history_groupmessage();
        break;
    case CHAT_SEND_GROUP: // 34给群发消息
        g.chat_send_group();
        break;
    case SEND_FILE_GROUP: // 35给群发文件
        g.send_file_group();
        break;
    case RECV_FILE_GROUP: // 36接收群文件
        g.recv_file_group();
        break;
    case SENDPARTFILE: // 38接收未发送完的文件
        g.send_part_file();
        break;
    }
}

void gay::setEfd(const int &e)
{
    efd = e;
}
int gay::getefd()
{
    return efd;
}

void gay::signout() // 9退出登录
{

    int clnt_sock = pChat.getServ_fd();
    char buf[BUFSIZ];
    json jn;

    int ret = ssock::ReadMsg(clnt_sock, buf, BUFSIZ);
    cout << "ret = " << ret << buf << endl;
    jn = json::parse(buf);

    pChat.From_Json(jn, pChat);  //将会修改pChat中本来的文件描述符
    pChat.setServ_fd(clnt_sock); //将文件描述符改回去

    ret = close(pChat.getServ_fd());
    printf("close ret = %d\n", ret);

    redisContext *c = Redis::RedisConnect("127.0.0.1", 6379);
    redisReply *r = Redis::hashdel(c, "Onlineuser", pChat.getNumber());
    if (r == NULL)
    {
        printf("Execut getValue failure\n");
        redisFree(c);
        return;
    }
    if (r->integer == 0)
    {
        cout << "下线失败" << endl;
    }

    //退出操作，执行完不需要将文件描述符挂上监听红黑树，将文件描述符关闭

    freeReplyObject(r);
    redisFree(c);
}

// 10添加好友
void gay::addFriend()
{
    struct epoll_event ep;
    int clnt_sock = pChat.getServ_fd();
    char buf[BUFSIZ];
    json jn, jn2;

    ssock::ReadMsg(clnt_sock, buf, BUFSIZ);
    jn = json::parse(buf);

    pChat.From_Json(jn, pChat);  //将会修改pChat中本来的文件描述符
    pChat.setServ_fd(clnt_sock); //将文件描述符改回去
    friends fri, fri2;           //定义一个friends类，将获取到的朋友信息进行解析，也将自己信息解析，添加到朋友的好友列表里面

    redisContext *c = Redis::RedisConnect("127.0.0.1", 6379);
    redisReply *r = Redis::hsetexist(c, pChat.getNumber() + "friend", pChat.getFriendUid()); //判断好友是否已经在好友列表
    do
    {
        if (r == NULL)
        {
            printf("Execut getValue failure\n");
            break;
        }
        if (r->integer == 1)
        {
            printf("已经是好友\n");
            freeReplyObject(r);
            ssock::SendMsg(clnt_sock, "Has user", strlen("Has user") + 1); //添加失败，已经是好友
            break;
        }
        freeReplyObject(r);
        r = Redis::hsetexist(c, "account", pChat.getFriendUid()); //判断账号列表是否有这个人
        if (r == NULL)
        {
            printf("Execut getValue failure\n");
            break;
        }
        if (r->integer == 0)
        {
            printf("该账号数据库中没有\n");
            freeReplyObject(r);
            ssock::SendMsg(clnt_sock, "No user", strlen("No user") + 1); //添加失败，数据库没有该用户
        }
        else
        {
            ssock::SendMsg(clnt_sock, "wait", strlen("wait") + 1);
            freeReplyObject(r);
            r = Redis::listlrem(c, pChat.getFriendUid() + "addfriend", "1", jn.dump()); //移除数据库中和要添加的数据一样的数据一条
            if (r == NULL)
            {
                printf("Execut getValue failure\n");
                break;
            }
            freeReplyObject(r);
            r = Redis::listrpush(c, pChat.getFriendUid() + "addfriend", jn.dump()); //将添加好友的信息放到数据库等待对方同意
            freeReplyObject(r);
            //将消息放到接收好友请求的用户的数据库中，提醒该用户有人添加它
            r = Redis::listlrem(c, pChat.getFriendUid() + "message", "1", jn.dump());
            if (r == NULL)
            {
                printf("Execut getValue failure\n");
                break;
            }
            freeReplyObject(r);
            r = Redis::listrpush(c, pChat.getFriendUid() + "message", jn.dump());
            freeReplyObject(r);
        }

    } while (0);
    redisFree(c);

    //执行完添加后将文件描述符挂上监听红黑树
    ep.events = EPOLLIN | EPOLLET;
    ep.data.fd = clnt_sock;
    int ret = epoll_ctl(efd, EPOLL_CTL_ADD, clnt_sock, &ep);
    if (ret == -1)
    {
        ssock::perr_exit("epoll_ctr error");
    }
}

// 11查询好友添加信息
void gay::inquqireAdd()
{
    struct epoll_event ep;
    int clnt_sock = pChat.getServ_fd();
    char buf[5];
    friends fri, fri2; //定义一个friends类，将获取到的朋友信息进行解析，也将自己信息解析，添加到朋友的好友列表里面
    json jn, jn2, jn3;

    char number[21];
    string addf = "addfriend";

    int ret = ssock::ReadMsg(clnt_sock, number, sizeof(number));
    if (ret == 0)
    {
        qqqqquit(clnt_sock);
        cout << "clnt_sock"
             << "关闭" << endl;
        return;
    }

    redisContext *c = Redis::RedisConnect("127.0.0.1", 6379);
    redisReply *r = Redis::listlen(c, number + addf);
    if (r == NULL)
    {
        printf("Execut getValue failure\n");
        redisFree(c);
    }
    int listlen = r->integer;
    freeReplyObject(r);

    for (int i = 0; i < listlen; ++i)
    {
        r = Redis::listlpop(c, number + addf);
        cout << r->str << endl;
        jn3 = json::parse(r->str);
        pChat.From_Json(jn3, pChat); //将会修改pChat中本来的文件描述符
        pChat.setServ_fd(clnt_sock); //将文件描述符改回去
        ret = ssock::SendMsg(clnt_sock, r->str, strlen(r->str) + 1);
        if (ret == -1)
        {
            qqqqquit(clnt_sock);
            freeReplyObject(r);
            redisFree(c);
            return;
        }
        freeReplyObject(r);
        ret = ssock::ReadMsg(clnt_sock, buf, sizeof(buf));
        if (ret == 0)
        {
            qqqqquit(clnt_sock);
            redisFree(c);
            return;
        }
        if (strcmp(buf, "Yes") == 0)
        {
            cout << "成功添加" << endl;
            fri.setflag(1);                         //设置朋友的权限
            fri.setfriendUid(pChat.getFriendUid()); //设置朋友的uid
            fri.To_Json(jn, fri);                   //将fri转换为json
            fri2.setflag(1);
            fri2.setfriendUid(pChat.getNumber());
            fri2.To_Json(jn2, fri2);

            //将转换后的字符串写入数据库
            //将要添加的好友写入自己的好友列表
            r = Redis::hsetValue(c, pChat.getNumber() + "friend", fri.getfriendUid(), jn.dump()); // 1表示正常，0表示屏蔽
            freeReplyObject(r);
            //将自己写入要添加的好友的好友列表
            r = Redis::hsetValue(c, pChat.getFriendUid() + "friend", fri2.getfriendUid(), jn2.dump()); // 1表示正常，0表示屏蔽
            freeReplyObject(r);
        }
        else
        {
            cout << "添加失败" << endl;
        }
    }
    ret = ssock::SendMsg(clnt_sock, "finish", strlen("finish") + 1);
    if (ret == -1)
    {
        qqqqquit(clnt_sock);
        redisFree(c);
        return;
    }

    redisFree(c);

    //执行完添加后将文件描述符挂上监听红黑树
    ep.events = EPOLLIN | EPOLLET;
    ep.data.fd = clnt_sock;
    ret = epoll_ctl(efd, EPOLL_CTL_ADD, clnt_sock, &ep);
    if (ret == -1)
    {
        ssock::perr_exit("epoll_ctr error");
    }
}

// 12删除好友
void gay::delFriend()
{
    struct epoll_event ep;
    int clnt_sock = pChat.getServ_fd();
    char buf[BUFSIZ];
    json jn;

    ssock::ReadMsg(clnt_sock, buf, BUFSIZ);
    jn = json::parse(buf);

    pChat.From_Json(jn, pChat); //将会修改pChat中本来的文件描述符
    pChat.setServ_fd(clnt_sock);

    redisContext *c = Redis::RedisConnect("127.0.0.1", 6379);
    redisReply *r = Redis::hsetexist(c, pChat.getNumber() + "friend", pChat.getFriendUid()); //判断好友是否已经在好友列表
    do
    {
        if (r == NULL)
        {
            printf("Execut getValue failure\n");
            break;
        }
        if (r->integer == 0)
        {
            printf("没有该好友\n");
            freeReplyObject(r);
            ssock::SendMsg(clnt_sock, "No user", strlen("No user") + 1); //删除失败，没有这个好友
            break;
        }
        freeReplyObject(r);
        ssock::SendMsg(clnt_sock, "del successfully", strlen("del successfully") + 1);

        //删除好友，将好友从自己的列表删除
        r = Redis::hashdel(c, pChat.getNumber() + "friend", pChat.getFriendUid());
        freeReplyObject(r);
        //删除好友，将自己从好友的列表删除
        r = Redis::hashdel(c, pChat.getFriendUid() + "friend", pChat.getNumber());
        freeReplyObject(r);

    } while (0);
    redisFree(c);

    //执行完添加后将文件描述符挂上监听红黑树
    ep.events = EPOLLIN | EPOLLET;
    ep.data.fd = clnt_sock;
    int ret = epoll_ctl(efd, EPOLL_CTL_ADD, clnt_sock, &ep);
    if (ret == -1)
    {
        ssock::perr_exit("epoll_ctr error");
    }
}

// 13查询好友
void gay::findFriend()
{
    struct epoll_event ep;
    int clnt_sock = pChat.getServ_fd();
    json jn;
    privateChat pChat;
    char buf[BUFSIZ];

    ssock::ReadMsg(clnt_sock, buf, sizeof(buf));
    jn = json::parse(buf);
    pChat.From_Json(jn, pChat); //将会修改pChat中本来的文件描述符
    pChat.setServ_fd(clnt_sock);

    redisContext *c = Redis::RedisConnect("127.0.0.1", 6379);
    redisReply *r = Redis::hgethashall(c, pChat.getNumber() + "friend"); //获取所有好友
    if (r == NULL)
    {
        printf("Execut getValue failure\n");
        redisFree(c);
        return;
    }
    for (int i = 0; i < r->elements; ++i) //遍历好友列表
    {
        if (i % 2 == 1) //偶数是key，奇数是value
        {
            ssock::SendMsg(clnt_sock, r->element[i]->str, strlen(r->element[i]->str) + 1); //将得到的字符串发送给客户端
        }
    }
    ssock::SendMsg(clnt_sock, "finish", strlen("finish") + 1);

    freeReplyObject(r);
    redisFree(c);

    //执行完添加后将文件描述符挂上监听红黑树
    ep.events = EPOLLIN | EPOLLET;
    ep.data.fd = clnt_sock;
    int ret = epoll_ctl(efd, EPOLL_CTL_ADD, clnt_sock, &ep);
    if (ret == -1)
    {
        ssock::perr_exit("epoll_ctr error");
    }
}

void gay::onlineStatus() // 14显示好友在线信息
{
    struct epoll_event ep;
    int clnt_sock = pChat.getServ_fd();
    json jn;
    privateChat pChat;
    char buf[BUFSIZ];

    ssock::ReadMsg(clnt_sock, buf, sizeof(buf));
    jn = json::parse(buf);
    pChat.From_Json(jn, pChat); //将会修改pChat中本来的文件描述符
    pChat.setServ_fd(clnt_sock);

    redisContext *c = Redis::RedisConnect("127.0.0.1", 6379);
    redisReply *r = Redis::hgethashall(c, pChat.getNumber() + "friend"); //获取所有好友
    redisReply *r2;
    if (r == NULL)
    {
        printf("Execut getValue failure\n");
        redisFree(c);
        return;
    }
    int flag;
    for (int i = 0; i < r->elements; ++i) //遍历好友列表
    {
        if (i % 2 == 0) //偶数是key，奇数是value
        {
            r2 = Redis::hsetexist(c, "Onlineuser", r->element[i]->str); //判断好友是否在在线列表
            if (r2 == NULL)
            {
                printf("Execut getValue failure\n");
                redisFree(c);
                return;
            }
            flag = r2->integer;
            flag = htonl(flag);
            if (r2->integer == 0) //好友不在线
            {
                ssock::SendMsg(clnt_sock, (void *)&flag, sizeof(flag));                                //发送0表示不在线
                ssock::SendMsg(clnt_sock, r->element[i + 1]->str, strlen(r->element[i + 1]->str) + 1); //将得到的value发送给对端
            }
            else //好友在线
            {
                freeReplyObject(r2);
                r2 = Redis::hgethash(c, "Onlineuser", r->element[i]->str);
                ssock::SendMsg(clnt_sock, (void *)&flag, sizeof(flag)); //发送其他数字表示在线
                ssock::SendMsg(clnt_sock, r2->str, strlen(r2->str) + 1);
            }
            freeReplyObject(r2);
        }
    }
    ssock::SendMsg(clnt_sock, (void *)&flag, sizeof(flag));
    ssock::SendMsg(clnt_sock, "finish", strlen("finish") + 1);

    freeReplyObject(r);
    redisFree(c);

    //执行完添加后将文件描述符挂上监听红黑树
    ep.events = EPOLLIN | EPOLLET;
    ep.data.fd = clnt_sock;
    cout << efd << endl;
    int ret = epoll_ctl(efd, EPOLL_CTL_ADD, clnt_sock, &ep);
    if (ret == -1)
    {
        ssock::perr_exit("epoll_ctr error");
    }
}

// 15屏蔽好友消息
void gay::blockFriend()
{
    struct epoll_event ep;
    int clnt_sock = pChat.getServ_fd();
    json jn, jn2;
    privateChat pChat;
    char buf[BUFSIZ];
    friends fri;

    ssock::ReadMsg(clnt_sock, buf, sizeof(buf));
    jn = json::parse(buf);
    pChat.From_Json(jn, pChat);
    pChat.setServ_fd(clnt_sock);

    redisContext *c = Redis::RedisConnect("127.0.0.1", 6379);
    redisReply *r = Redis::hsetexist(c, pChat.getNumber() + "friend", pChat.getFriendUid()); //判断好友是否已经在好友列表
    do
    {
        if (r == NULL)
        {
            printf("Execut getValue failure\n");
            break;
        }
        if (r->integer == 0)
        {
            printf("没有该好友\n");
            freeReplyObject(r);
            ssock::SendMsg(clnt_sock, "No user", strlen("No user") + 1); //删除失败，没有这个好友
            break;
        }
        freeReplyObject(r);
        if (pChat.getMessage() == "1")
        {
            ssock::SendMsg(clnt_sock, "Block successfully", strlen("Block successfully") + 1);

            //屏蔽好友
            fri.setflag(0);
            fri.setfriendUid(pChat.getFriendUid());
            fri.To_Json(jn2, fri);
            r = Redis::hsetValue(c, pChat.getNumber() + "friend", pChat.getFriendUid(), jn2.dump());
            freeReplyObject(r);
        }
        else
        {
            ssock::SendMsg(clnt_sock, "No block successfully", strlen("No block successfully") + 1);

            //解除屏蔽好友
            fri.setflag(1);
            fri.setfriendUid(pChat.getFriendUid());
            fri.To_Json(jn2, fri);
            r = Redis::hsetValue(c, pChat.getNumber() + "friend", pChat.getFriendUid(), jn2.dump());
            freeReplyObject(r);
        }

    } while (0);
    redisFree(c);

    //执行完添加后将文件描述符挂上监听红黑树
    ep.events = EPOLLIN | EPOLLET;
    ep.data.fd = clnt_sock;
    int ret = epoll_ctl(efd, EPOLL_CTL_ADD, clnt_sock, &ep);
    if (ret == -1)
    {
        ssock::perr_exit("epoll_ctr error");
    }
}

privateChat &gay::getpChat()
{
    return pChat;
}

pthread_mutex_t &gay::getMutex() //获得锁
{
    return mutex;
}

// 17查看历史聊天记录
void gay::history_message()
{
    struct epoll_event ep;
    int clnt_sock = pChat.getServ_fd();
    char buf[BUFSIZ];
    json jn;
    int ret;

    ssock::ReadMsg(clnt_sock, buf, sizeof(buf));
    jn = json::parse(buf);
    pChat.From_Json(jn, pChat);  //会修改pChat中本来的套间字的值
    pChat.setServ_fd(clnt_sock); //将套间字的值修改回去

    redisContext *c = Redis::RedisConnect("127.0.0.1", 6379);
    redisReply *r = Redis::listlrange(c, pChat.getNumber() + "history" + pChat.getFriendUid(), "0", "-1");
    if (r == NULL)
    {
        printf("Execut getValue failure\n");
        redisFree(c);
    }
    for (int i = 0; i < r->elements; ++i) //遍历消息记录
    {
        cout << r->element[i]->str << endl;
        cout << strlen(r->element[i]->str) + 1;
        ret = ssock::SendMsg(clnt_sock, r->element[i]->str, strlen(r->element[i]->str) + 1);
        if (ret == -1)
        {
            qqqqquit(clnt_sock);
            redisFree(c);
            return;
        }
    }
    ssock::SendMsg(clnt_sock, "finish", strlen("finish") + 1);
    if (ret == -1)
    {
        qqqqquit(clnt_sock);
        redisFree(c);
        return;
    }
    redisFree(c);

    //执行完添加后将文件描述符挂上监听红黑树
    ep.events = EPOLLIN | EPOLLET;
    ep.data.fd = clnt_sock;
    ret = epoll_ctl(efd, EPOLL_CTL_ADD, clnt_sock, &ep);
    if (ret == -1)
    {
        ssock::perr_exit("epoll_ctr error");
    }
}

// 18和好友聊天
void gay::talkwithfriends()
{
    int flag = 0; //定义一个标记，在第一次进入循环时改变自己的状态
    struct epoll_event ep;
    int clnt_sock = pChat.getServ_fd();
    char buf[BUFSIZ];
    json jn, jn2, jn3, jn4;
    onlineUser onlineU, onlineU2;
    friends fri;

    redisContext *c;
    redisReply *r;
    c = Redis::RedisConnect("127.0.0.1", 6379);

    while (1)
    {
        int ret = ssock::ReadMsg(clnt_sock, buf, sizeof(buf));
        cout << "ret = " << ret << endl;
        if (ret == 0)
        {
            qqqqquit(clnt_sock); //下线
            redisFree(c);
            return;
        }

        cout << buf << endl;
        jn = json::parse(buf);
        pChat.From_Json(jn, pChat);  //将序列转为类，会修改原有的套间字
        pChat.setServ_fd(clnt_sock); //将套间字修改回去
        if (flag == 0)
        {
            r = Redis::hgethash(c, "Onlineuser", pChat.getNumber());
            jn4 = json::parse(r->str);
            onlineU2.From_Json(jn4, onlineU2); //将自己的状态改为聊天
            onlineU2.setflag(18);
            onlineU2.To_Json(jn4, onlineU2); //将jn修改
            freeReplyObject(r);
            r = Redis::hsetValue(c, "Onlineuser", onlineU2.getUid(), jn4.dump()); //将自己修改后的状态写入数据库
            freeReplyObject(r);
        }

        if (strcmp(pChat.getMessage().c_str(), "exit") == 0) //如果发送exit，就退出私聊，然后给自己发exit让自己的接受消息的线程退出
        {
            ret = ssock::SendMsg(clnt_sock, jn.dump().c_str(), strlen(jn.dump().c_str()) + 1); //将消息转发给自己
            if (ret == -1)
            {
                qqqqquit(clnt_sock);
                redisFree(c);
                return;
            }
            //将消息写入自己的列表里。意味着自己关闭，continue_receive线程需要开始工作了
            r = Redis::listrpush(c, pChat.getNumber() + "message", jn.dump().c_str());
            freeReplyObject(r);
            break;
        }

        //判断发送者发送的人是否是它的好友
        r = Redis::hsetexist(c, pChat.getNumber() + "friend", pChat.getFriendUid());
        if (r == NULL)
        {
            printf("Execut getValue failure\n");
            redisFree(c);
            continue;
        }
        if (r->integer == 0) //没有该好友
        {
            cout << "没有该好友" << endl;
            ret = ssock::SendMsg(clnt_sock, "No friend", strlen("No friend") + 1); //将消息转发给自己
            if (ret == -1)
            {
                qqqqquit(clnt_sock);
                redisFree(c);
                return;
            }
            freeReplyObject(r);
            continue;
        }
        freeReplyObject(r);
        //判断它是否被它的好友屏蔽了它
        r = Redis::hgethash(c, pChat.getFriendUid() + "friend", pChat.getNumber());
        if (r == NULL)
        {
            printf("Execut getValue failure\n");
            redisFree(c);
            continue;
        }
        jn3 = json::parse(r->str);
        fri.From_Json(jn3, fri);
        if (fri.getflag() == 0)
        {
            cout << "屏蔽了" << endl;
            continue;
        }

        freeReplyObject(r);

        r = Redis::hsetexist(c, "Onlineuser", pChat.getFriendUid()); //在数据库中查找好友是否在线
        if (r == NULL)
        {
            printf("Execut getValue failure\n");
            redisFree(c);
            continue;
        }
        if (r->integer == 0) //不在线，继续循环
        {
            cout << "好友不在线" << endl;
            freeReplyObject(r);
            //将消息写入一个列表里，然后在客户端从该列表中读取数据
            r = Redis::listrpush(c, pChat.getFriendUid() + "message", jn.dump().c_str());
            freeReplyObject(r);

            //将聊天记录添加到历史记录里
            r = Redis::listrpush(c, pChat.getNumber() + "history" + pChat.getFriendUid(), jn.dump().c_str());
            freeReplyObject(r);
            r = Redis::listrpush(c, pChat.getFriendUid() + "history" + pChat.getNumber(), jn.dump().c_str());
            freeReplyObject(r);
            continue;
        }
        freeReplyObject(r);

        r = Redis::hgethash(c, "Onlineuser", pChat.getFriendUid());
        if (r == NULL)
        {
            printf("Execut getValue failure\n");
            redisFree(c);
            continue;
        }
        if (r->type != REDIS_REPLY_STRING)
        {
            printf("Execut getValue failure\n");
            freeReplyObject(r);
            redisFree(c);
            continue;
        }
        jn2 = json::parse(r->str);
        onlineU.From_Json(jn2, onlineU);
        cout << jn.dump() << endl;
        if (onlineU.getflag() == 18)
        {
            ret = ssock::SendMsg(onlineU.getsock(), jn.dump().c_str(), strlen(jn.dump().c_str()) + 1); //将消息转发给好友
            if (ret == -1)
            {
                qqqqquit(clnt_sock);
                redisFree(c);
                return;
            }
        }

        freeReplyObject(r);

        //将消息写入一个列表里，然后在客户端从该列表中读取数据
        r = Redis::listrpush(c, pChat.getFriendUid() + "message", jn.dump().c_str());
        freeReplyObject(r);

        //将聊天记录添加到历史记录里
        r = Redis::listrpush(c, pChat.getNumber() + "history" + pChat.getFriendUid(), jn.dump().c_str());
        freeReplyObject(r);
        r = Redis::listrpush(c, pChat.getFriendUid() + "history" + pChat.getNumber(), jn.dump().c_str());
        freeReplyObject(r);
    }

    r = Redis::hgethash(c, "Onlineuser", pChat.getNumber());
    cout << "r->str = " << r->str;
    jn = json::parse(r->str);
    onlineU2.From_Json(jn, onlineU2); //将自己的状态改为不聊天
    onlineU2.setflag(0);
    onlineU2.To_Json(jn, onlineU2); //将jn修改
    freeReplyObject(r);
    r = Redis::hsetValue(c, "Onlineuser", onlineU2.getUid(), jn.dump()); //将自己修改后的状态写入数据库
    freeReplyObject(r);
    redisFree(c);

    //执行完添加后将文件描述符挂上监听红黑树
    ep.events = EPOLLIN | EPOLLET;
    ep.data.fd = clnt_sock;
    int ret = epoll_ctl(efd, EPOLL_CTL_ADD, clnt_sock, &ep);
    if (ret == -1)
    {
        ssock::perr_exit("epoll_ctr error");
    }
}

string longtostring(long int t)
{
    std::string result;
    stringstream ss;
    ss << t;
    ss >> result;
    return result;
}

// 19从客户端读文件
void gay::send_file()
{
    struct epoll_event ep;
    int clnt_sock = pChat.getServ_fd();
    char buf[BUFSIZ];
    json jn, jn2, jn3;

    friends fri;
    redisContext *c;
    redisReply *r;

    int ret = ssock::ReadMsg(clnt_sock, buf, sizeof(buf));
    cout << "ret = " << ret << endl;
    if (ret == 0)
    {
        qqqqquit(clnt_sock); //下线
        return;
    }

    cout << buf << endl;
    jn = json::parse(buf);
    pChat.From_Json(jn, pChat);  //将序列转为类，会修改原有的套间字
    pChat.setServ_fd(clnt_sock); //将套间字修改回去

    do
    {
        c = Redis::RedisConnect("127.0.0.1", 6379);

        //判断发送者发送的人是否是它的好友
        r = Redis::hsetexist(c, pChat.getNumber() + "friend", pChat.getFriendUid());
        if (r == NULL)
        {
            printf("Execut getValue failure\n");
            redisFree(c);
            break;
        }
        if (r->integer == 0) //没有该好友
        {
            cout << "没有该好友" << endl;
            ssock::SendMsg(clnt_sock, "No friend", strlen("No friend") + 1); //将消息转发给自己
            freeReplyObject(r);
            redisFree(c);
            break;
        }
        else
        {
            ssock::SendMsg(clnt_sock, "Has friend", strlen("Has friend") + 1); //将消息转发给自己
        }
        freeReplyObject(r);
        //判断它是否被它的好友屏蔽了它
        r = Redis::hgethash(c, pChat.getFriendUid() + "friend", pChat.getNumber());
        if (r == NULL)
        {
            printf("Execut getValue failure\n");
            redisFree(c);
            break;
        }
        jn3 = json::parse(r->str);
        fri.From_Json(jn3, fri);
        if (fri.getflag() == 0)
        {
            cout << "屏蔽了" << endl;
            ssock::SendMsg(clnt_sock, "Has block", strlen("Has block") + 1); //将消息转发给自己
            freeReplyObject(r);
            redisFree(c);
            break;
        }
        else
        {
            ssock::SendMsg(clnt_sock, "No block", strlen("No block") + 1);
        }
        freeReplyObject(r);

        string filename(pChat.getMessage()); //文件名
        auto f = filename.rfind('/');
        filename.erase(0, f + 1);
        filename.insert(0, "../Temporaryfiles/");
        DIR *d = opendir("../Temporaryfiles");
        if (d == NULL)
        {
            mkdir("../Temporaryfiles", 0777);
        }
        FILE *fp = fopen(filename.c_str(), "w");

        int n;
        //将文件存入本地
        __off_t size;
        ret = ssock::ReadMsg(clnt_sock, &size, sizeof(size));
        if (ret == 0)
        {
            qqqqquit(clnt_sock);
            redisFree(c);
            return;
        }

        __off_t len = 0; //用来保存已经接收的文件大小
        size = ntohll(size);
        cout << "size = " << size << endl;
        while (size > 0)
        {
            if (sizeof(buf) < size)
            {
                n = ssock::Readn(clnt_sock, buf, sizeof(buf));
            }
            else
            {
                n = ssock::Readn(clnt_sock, buf, size);
            }
            if (n < 0)
            {
                continue;
            }
            if (n == 0)
            {
                cout << "len = " << len << endl; //已经接收的文件长度
                r = Redis::listrpush(c, pChat.getNumber() + "partfile1", jn.dump());
                freeReplyObject(r);
                qqqqquit(clnt_sock);
                redisFree(c);
                fclose(fp);
                return;
            }
            len += n;
            size -= n;
            fwrite(buf, n, 1, fp);
            cout << "size = " << size << endl;
        }

        //将消息写入一个列表里，然后在客户端从该列表中读取数据，提醒客户端有数据来了
        r = Redis::listrpush(c, pChat.getFriendUid() + "message", jn.dump().c_str());
        freeReplyObject(r);
        //将消息写到一个列表里（包括其中的文件名），让客户端可以知道自己要哪个文件
        r = Redis::listrpush(c, pChat.getFriendUid() + "file", jn.dump().c_str());
        freeReplyObject(r);

        fclose(fp);
        redisFree(c);
    } while (0);

    //执行完添加后将文件描述符挂上监听红黑树
    ep.events = EPOLLIN | EPOLLET;
    ep.data.fd = clnt_sock;
    ret = epoll_ctl(efd, EPOLL_CTL_ADD, clnt_sock, &ep);
    if (ret == -1)
    {
        ssock::perr_exit("epoll_ctr error");
    }
}

// 20向客户端发送文件
void gay::recv_file()
{
    privateChat pChat2;
    struct epoll_event ep;
    int clnt_sock = pChat.getServ_fd();
    char buf[BUFSIZ];
    json jn, jn2;
    int ret;

    ret = ssock::ReadMsg(clnt_sock, buf, sizeof(buf));
    if (ret == 0)
    {
        qqqqquit(clnt_sock);
        cout << "clnt_sock"
             << "关闭" << endl;
        return;
    }
    jn = json::parse(buf);
    pChat.From_Json(jn, pChat);  //将序列转为类，会修改原有的套间字
    pChat.setServ_fd(clnt_sock); //将套间字修改回去

    redisContext *c = Redis::RedisConnect("127.0.0.1", 6379);
    redisReply *r = Redis::listlen(c, pChat.getNumber() + "file");
    if (r == NULL)
    {
        printf("Execut getValue failure\n");
        redisFree(c);
    }
    int listlen = r->integer;
    freeReplyObject(r);

    for (int i = 0; i < listlen; ++i)
    {
        r = Redis::listlpop(c, pChat.getNumber() + "file");
        cout << r->str << endl;
        jn2 = json::parse(r->str);
        pChat2.From_Json(jn2, pChat2); //将会修改pChat中本来的文件描述符
        ret = ssock::SendMsg(clnt_sock, r->str, strlen(r->str) + 1);
        if (ret == -1)
        {
            qqqqquit(clnt_sock);
            freeReplyObject(r);
            redisFree(c);
            return;
        }
        freeReplyObject(r);
        ret = ssock::ReadMsg(clnt_sock, buf, sizeof(buf));
        if (ret == 0)
        {
            qqqqquit(clnt_sock);
            redisFree(c);
            return;
        }
        if (strcmp(buf, "Yes") == 0)
        {
            cout << "同意接收" << endl;
            string filename(pChat2.getMessage()); //文件名
            auto f = filename.rfind('/');
            filename.erase(0, f + 1);
            filename.insert(0, "../Temporaryfiles/");
            int filefd = open(filename.c_str(), O_RDONLY);

            __off_t size;
            struct stat file_stat;
            //为了获取文件大小
            fstat(filefd, &file_stat);
            size = file_stat.st_size;
            size = htonll(size);

            ssock::SendMsg(clnt_sock, (void *)&size, sizeof(size));
            if (ret == -1)
            {
                qqqqquit(clnt_sock);
                redisFree(c);
                return;
            }

            while ((ret = sendfile(clnt_sock, filefd, NULL, file_stat.st_size)) != 0)
            {
                cout << "ret = " << ret << endl;
                if (ret == -1 && errno == 104)
                {
                    cout << "errno" << errno << endl;
                    qqqqquit(clnt_sock);
                    redisFree(c);
                    close(filefd);
                    return;
                }
            }
            cout << "ret = " << ret << endl;

            close(filefd);
        }
        else
        {
            cout << "拒绝接收" << endl;
        }
    }
    ret = ssock::SendMsg(clnt_sock, "finish", strlen("finish") + 1);
    if (ret == -1)
    {
        qqqqquit(clnt_sock);
        redisFree(c);
        return;
    }

    redisFree(c);

    //执行完添加后将文件描述符挂上监听红黑树
    ep.events = EPOLLIN | EPOLLET;
    ep.data.fd = clnt_sock;
    ret = epoll_ctl(efd, EPOLL_CTL_ADD, clnt_sock, &ep);
    if (ret == -1)
    {
        ssock::perr_exit("epoll_ctr error");
    }
}

// 22创建群
void gay::creategroup()
{
    privateChat pChat2;
    struct epoll_event ep;
    int clnt_sock = pChat.getServ_fd(); //获取服务器套间字
    char buf[BUFSIZ];
    int ret;
    json jn, jn2;
    groups grps;

    ssock::ReadMsg(clnt_sock, buf, sizeof(buf));
    jn = json::parse(buf);
    pChat2.From_Json(jn, pChat2);

    grps.setgroup_number(pChat2.getFriendUid()); //设置群号
    grps.setuser_number(pChat2.getNumber());     //设置群成员id
    grps.setflag(2);                             //设置权限2，群主
    grps.To_Json(jn2, grps);

    redisContext *c = Redis::RedisConnect("127.0.0.1", 6379);
    redisReply *r;
    do
    {
        r = Redis::exists(c, pChat2.getFriendUid() + "group"); //判断一个key是否已经存在，即判断群是否已经存在
        if (r == NULL)
        {
            printf("Execut getValue failure\n");
            break;
        }
        if (r->integer == 0)
        {
            printf("该群号数据库中没有\n");
            freeReplyObject(r);
            r = Redis::hsetValue(c, pChat2.getFriendUid() + "group", pChat2.getNumber(), jn2.dump()); //添加群信息
            freeReplyObject(r);
            r = Redis::hsetValue(c, pChat2.getNumber() + "groups", pChat2.getFriendUid(), jn2.dump()); //将群添加到自己已经加入的群哈希中
            freeReplyObject(r);
            ssock::SendMsg(clnt_sock, "Yes", 4);
        }
        else
        {
            printf("该群已经存在\n");
            freeReplyObject(r);
            ssock::SendMsg(clnt_sock, "No", 3);
        }
    } while (0);
    redisFree(c);

    //执行完添加后将文件描述符挂上监听红黑树
    ep.events = EPOLLIN | EPOLLET;
    ep.data.fd = clnt_sock;
    ret = epoll_ctl(efd, EPOLL_CTL_ADD, clnt_sock, &ep);
    if (ret == -1)
    {
        ssock::perr_exit("epoll_ctr error");
    }
}

// 23解散群
void gay::dissolvegroup()
{
    privateChat pChat2;
    struct epoll_event ep;
    int clnt_sock = pChat.getServ_fd(); //获取服务器套间字
    char buf[BUFSIZ];
    int ret;
    json jn, jn2, jn3;
    groups grps;

    ssock::ReadMsg(clnt_sock, buf, sizeof(buf));
    jn = json::parse(buf);
    pChat2.From_Json(jn, pChat2);

    redisContext *c = Redis::RedisConnect("127.0.0.1", 6379);
    redisReply *r, *r2;
    do
    {
        r = Redis::exists(c, pChat2.getFriendUid() + "group"); //判断一个key是否已经存在，即判断群是否已经存在
        if (r == NULL)
        {
            printf("Execut getValue failure\n");
            break;
        }
        if (r->integer == 0)
        {
            printf("该群号数据库中没有\n");
            freeReplyObject(r);
            ssock::SendMsg(clnt_sock, "No group", strlen("No group") + 1);
            break;
        }
        else
        {
            freeReplyObject(r);

            r = Redis::hgethash(c, pChat2.getFriendUid() + "group", pChat2.getNumber());
            if (r == NULL)
            {
                printf("Execut getValue failure\n");
                break;
            }
            if (r->str == NULL)
            {
                printf("你不是该群成员\n");
                freeReplyObject(r);
                //你不是该群成员
                ssock::SendMsg(clnt_sock, "you are not a member of this group", strlen("you are not a member of this group") + 1);
                break;
            }
            else
            {
                jn3 = json::parse(r->str);
                grps.From_Json(jn3, grps);
                freeReplyObject(r);
                if (grps.getflag() != 2)
                {
                    cout << "你不是该群群主" << endl;
                    ssock::SendMsg(clnt_sock, "not the group owner", strlen("not the group owner") + 1);
                    break;
                }
                else
                {
                    cout << "解散成功！" << endl;
                    ssock::SendMsg(clnt_sock, "Yes", strlen("Yes") + 1);
                    r = Redis::hgethashall(c, pChat2.getFriendUid() + "group");
                    string groups = "groups";
                    for (int i = 0; i < r->elements; ++i) //遍历所有用户，将该群从所有用户保存自己加入的群的数据库中移除
                    {
                        if (i % 2 == 0) //偶数是key，奇数是value
                        {
                            //将该群信息从用户保存群信息的数据库移除
                            r2 = Redis::hashdel(c, r->element[i]->str + groups, pChat2.getFriendUid());
                            freeReplyObject(r2);
                            //将该用户从群中删除
                            r2 = Redis::hashdel(c, pChat2.getFriendUid() + "group", r->element[i]->str);
                            freeReplyObject(r2);
                        }
                    }
                }
            }
        }
    } while (0);
    redisFree(c);

    //执行完添加后将文件描述符挂上监听红黑树
    ep.events = EPOLLIN | EPOLLET;
    ep.data.fd = clnt_sock;
    ret = epoll_ctl(efd, EPOLL_CTL_ADD, clnt_sock, &ep);
    if (ret == -1)
    {
        ssock::perr_exit("epoll_ctr error");
    }
}

// 24加入群
void gay::joingroup()
{
    privateChat pChat2;
    struct epoll_event ep;
    int clnt_sock = pChat.getServ_fd(); //获取服务器套间字
    char buf[BUFSIZ];
    int ret;
    json jn, jn2;
    groups grps;
    string message = "message";

    ssock::ReadMsg(clnt_sock, buf, sizeof(buf));
    jn = json::parse(buf);
    pChat2.From_Json(jn, pChat2);

    redisContext *c = Redis::RedisConnect("127.0.0.1", 6379);
    redisReply *r, *r2;
    do
    {
        r = Redis::exists(c, pChat2.getFriendUid() + "group"); //判断一个key是否已经存在，即判断群是否已经存在
        if (r == NULL)
        {
            printf("Execut getValue failure\n");
            break;
        }
        if (r->integer == 0)
        {
            printf("该群号数据库中没有\n");
            freeReplyObject(r);
            ssock::SendMsg(clnt_sock, "No group", strlen("No group") + 1);
            break;
        }
        else
        {
            freeReplyObject(r);

            r = Redis::hgethash(c, pChat2.getFriendUid() + "group", pChat2.getNumber());
            if (r == NULL)
            {
                printf("Execut getValue failure\n");
                break;
            }
            if (r->str == NULL)
            {
                printf("你还不是该群成员，等待管理员审核\n");
                freeReplyObject(r);
                //你不是该群成员，遍历成员列表，将加群提示分发给群中的每个用户
                r = Redis::hgethashall(c, pChat2.getFriendUid() + "group");
                if (r == NULL)
                {
                    printf("Execut getValue failure\n");
                    break;
                }
                for (int i = 0; i < r->elements; ++i) //遍历群成员
                {
                    if (i % 2 == 0) //偶数是key，奇数是value
                    {
                        jn2 = json::parse(r->element[i + 1]->str);
                        grps.From_Json(jn2, grps);
                        if (grps.getflag() == 1 || grps.getflag() == 2)
                        {
                            r2 = Redis::listlrem(c, r->element[i]->str + message, "1", jn.dump()); //移除数据库中和要添加的数据一样的数据一条
                            if (r2 == NULL)
                            {
                                printf("Execut getValue failure\n");
                                break;
                            }
                            freeReplyObject(r2);
                            r2 = Redis::listrpush(c, r->element[i]->str + message, jn.dump()); //在提醒列表中加入有成员申请加群信息
                            freeReplyObject(r2);
                        }
                    }
                }
                freeReplyObject(r);
                r = Redis::listlrem(c, pChat2.getFriendUid() + message, "1", jn.dump()); //移除数据库中和要添加的数据一样的数据一条
                if (r == NULL)
                {
                    printf("Execut getValue failure\n");
                    break;
                }
                freeReplyObject(r);
                r = Redis::listrpush(c, pChat2.getFriendUid() + message, jn.dump());
                ssock::SendMsg(clnt_sock, "Yes", strlen("Yes") + 1);
                break;
            }
            else
            {
                freeReplyObject(r);
                cout << "你已经是该群成员" << endl;
                ssock::SendMsg(clnt_sock, "you are already a member of this group", strlen("you are already a member of this group") + 1);
            }
        }
    } while (0);
    redisFree(c);

    //执行完添加后将文件描述符挂上监听红黑树
    ep.events = EPOLLIN | EPOLLET;
    ep.data.fd = clnt_sock;
    ret = epoll_ctl(efd, EPOLL_CTL_ADD, clnt_sock, &ep);
    if (ret == -1)
    {
        ssock::perr_exit("epoll_ctr error");
    }
}

// 25退出群
void gay::quitgroup()
{
    privateChat pChat2;
    struct epoll_event ep;
    int clnt_sock = pChat.getServ_fd(); //获取服务器套间字
    char buf[BUFSIZ];
    int ret;
    json jn, jn2;
    groups grps;

    ssock::ReadMsg(clnt_sock, buf, sizeof(buf));
    jn = json::parse(buf);
    pChat2.From_Json(jn, pChat2);

    redisContext *c = Redis::RedisConnect("127.0.0.1", 6379);
    redisReply *r;
    do
    {
        r = Redis::exists(c, pChat2.getFriendUid() + "group"); //判断一个key是否已经存在，即判断群是否已经存在
        if (r == NULL)
        {
            printf("Execut getValue failure\n");
            break;
        }
        if (r->integer == 0)
        {
            printf("该群号数据库中没有\n");
            freeReplyObject(r);
            ssock::SendMsg(clnt_sock, "No group", strlen("No group") + 1);
            break;
        }
        else
        {
            freeReplyObject(r);

            r = Redis::hgethash(c, pChat2.getFriendUid() + "group", pChat2.getNumber()); //获取自己在该群的信息
            if (r == NULL)
            {
                printf("Execut getValue failure\n");
                break;
            }
            if (r->str == NULL) //该群不存在自己
            {
                printf("你还不是该群成员，无法退出该群\n");
                freeReplyObject(r);
                //你不是该群成员
                ssock::SendMsg(clnt_sock, "you are not a member of this group", strlen("you are not a member of this group") + 1);
                break;
            }
            else //存在
            {
                jn2 = json::parse(r->str);
                grps.From_Json(jn2, grps);
                freeReplyObject(r);
                if (grps.getflag() == 2) //是群主，无法退群
                {
                    ssock::SendMsg(clnt_sock, "you are the leader", strlen("you are the leader") + 1);
                    break;
                }
                else
                {
                    cout << "你是该群成员，成功退出该群" << endl;
                    r = Redis::hashdel(c, pChat2.getFriendUid() + "group", pChat2.getNumber()); //删除自己在群中的信息
                    freeReplyObject(r);
                    r = Redis::hashdel(c, pChat2.getNumber() + "groups", pChat2.getFriendUid()); //将群从自己已经加入的群哈希中删除
                    freeReplyObject(r);
                    ssock::SendMsg(clnt_sock, "Yes", strlen("Yes") + 1);
                }
            }
        }
    } while (0);
    redisFree(c);

    //执行完添加后将文件描述符挂上监听红黑树
    ep.events = EPOLLIN | EPOLLET;
    ep.data.fd = clnt_sock;
    ret = epoll_ctl(efd, EPOLL_CTL_ADD, clnt_sock, &ep);
    if (ret == -1)
    {
        ssock::perr_exit("epoll_ctr error");
    }
}

// 26查看已加入的群组
void gay::hasjoingroup()
{
    privateChat pChat2;
    struct epoll_event ep;
    int clnt_sock = pChat.getServ_fd(); //获取服务器套间字
    char buf[BUFSIZ];
    int ret;
    json jn;
    groups grps;

    ssock::ReadMsg(clnt_sock, buf, sizeof(buf));
    jn = json::parse(buf);
    pChat2.From_Json(jn, pChat2);

    redisContext *c = Redis::RedisConnect("127.0.0.1", 6379);
    redisReply *r;
    do
    {
        r = Redis::hgethashall(c, pChat2.getNumber() + "groups"); //获取自己在该群的信息
        if (r == NULL)
        {
            printf("Execut getValue failure\n");
            break;
        }
        for (int i = 0; i < r->elements; ++i) //遍历群列表
        {
            if (i % 2 == 1) //偶数是key，奇数是value
            {
                jn = json::parse(r->element[i]->str);
                ret = ssock::SendMsg(clnt_sock, jn.dump().c_str(), strlen(jn.dump().c_str()) + 1);
                if (ret == -1)
                {
                    qqqqquit(clnt_sock);
                    redisFree(c);
                    return;
                }
            }
        }
        ret = ssock::SendMsg(clnt_sock, "finish", strlen("finish") + 1);
        if (ret == -1)
        {
            qqqqquit(clnt_sock);
            redisFree(c);
            return;
        }

    } while (0);
    freeReplyObject(r);
    redisFree(c);

    //执行完添加后将文件描述符挂上监听红黑树
    ep.events = EPOLLIN | EPOLLET;
    ep.data.fd = clnt_sock;
    ret = epoll_ctl(efd, EPOLL_CTL_ADD, clnt_sock, &ep);
    if (ret == -1)
    {
        ssock::perr_exit("epoll_ctr error");
    }
}

// 27查看群组成员
void gay::groupmembers()
{
    privateChat pChat2;
    struct epoll_event ep;
    int clnt_sock = pChat.getServ_fd(); //获取服务器套间字
    char buf[BUFSIZ];
    int ret;
    json jn;
    groups grps;

    ssock::ReadMsg(clnt_sock, buf, sizeof(buf));
    jn = json::parse(buf);
    pChat2.From_Json(jn, pChat2);

    redisContext *c = Redis::RedisConnect("127.0.0.1", 6379);
    redisReply *r;
    do
    {
        r = Redis::exists(c, pChat2.getFriendUid() + "group"); //判断一个key是否已经存在，即判断群是否已经存在
        if (r == NULL)
        {
            printf("Execut getValue failure\n");
            break;
        }
        if (r->integer == 0)
        {
            printf("该群不存在\n");
            freeReplyObject(r);
            ssock::SendMsg(clnt_sock, "No group", strlen("No group") + 1);
            break;
        }
        else
        {
            freeReplyObject(r);

            r = Redis::hgethash(c, pChat2.getFriendUid() + "group", pChat2.getNumber()); //获取自己在该群的信息
            if (r == NULL)
            {
                printf("Execut getValue failure\n");
                break;
            }
            if (r->str == NULL) //该群不存在自己
            {
                printf("你不是该群成员，无法查看该群成员信息\n");
                freeReplyObject(r);
                //你不是该群成员
                ssock::SendMsg(clnt_sock, "you are not a member of this group", strlen("you are not a member of this group") + 1);
                break;
            }
            else //存在
            {
                freeReplyObject(r);
                ssock::SendMsg(clnt_sock, "Yes", strlen("Yes") + 1);
                r = Redis::hgethashall(c, pChat2.getFriendUid() + "group");
                for (int i = 0; i < r->elements; ++i) //遍历群列表
                {
                    if (i % 2 == 1) //偶数是key，奇数是value
                    {
                        jn = json::parse(r->element[i]->str);
                        ssock::SendMsg(clnt_sock, jn.dump().c_str(), strlen(jn.dump().c_str()) + 1);
                    }
                }
                ssock::SendMsg(clnt_sock, "finish", strlen("finish") + 1);
            }
        }
    } while (0);
    redisFree(c);

    //执行完添加后将文件描述符挂上监听红黑树
    ep.events = EPOLLIN | EPOLLET;
    ep.data.fd = clnt_sock;
    ret = epoll_ctl(efd, EPOLL_CTL_ADD, clnt_sock, &ep);
    if (ret == -1)
    {
        ssock::perr_exit("epoll_ctr error");
    }
}

// 28设置管理员
void gay::pullmanagepeople()
{
    privateChat pChat2;
    struct epoll_event ep;
    int clnt_sock = pChat.getServ_fd(); //获取服务器套间字
    char buf[BUFSIZ];
    int ret;
    json jn, jn2;
    groups grps;

    ssock::ReadMsg(clnt_sock, buf, sizeof(buf));
    jn = json::parse(buf);
    pChat2.From_Json(jn, pChat2);

    redisContext *c = Redis::RedisConnect("127.0.0.1", 6379);
    redisReply *r;
    do
    {
        r = Redis::exists(c, pChat2.getFriendUid() + "group"); //判断一个key是否已经存在，即判断群是否已经存在
        if (r == NULL)
        {
            printf("Execut getValue failure\n");
            break;
        }
        if (r->integer == 0)
        {
            printf("该群号数据库中没有\n");
            freeReplyObject(r);
            ssock::SendMsg(clnt_sock, "No group", strlen("No group") + 1);
            break;
        }
        else
        {
            freeReplyObject(r);

            r = Redis::hgethash(c, pChat2.getFriendUid() + "group", pChat2.getNumber());
            if (r == NULL)
            {
                printf("Execut getValue failure\n");
                break;
            }
            if (r->str == NULL) //判断该用户是否是群成员
            {
                printf("你不是该群成员\n");
                freeReplyObject(r);
                //你不是该群成员
                ssock::SendMsg(clnt_sock, "you are not a member of this group", strlen("you are not a member of this group") + 1);
                break;
            }
            else
            {
                jn2 = json::parse(r->str);
                grps.From_Json(jn2, grps);
                freeReplyObject(r);
                if (grps.getflag() != 2) //判断该用户是否是群主
                {
                    cout << "你不是该群群主" << endl;
                    ssock::SendMsg(clnt_sock, "not the group owner", strlen("not the group owner") + 1);
                    break;
                }
                else
                {
                    r = Redis::hgethash(c, pChat2.getFriendUid() + "group", pChat2.getMessage());
                    if (r == NULL)
                    {
                        printf("Execut getValue failure\n");
                        break;
                    }
                    if (r->str == NULL)
                    {
                        printf("要设置的管理员不是该群成员\n");
                        freeReplyObject(r);
                        //你不是该群成员
                        ssock::SendMsg(clnt_sock, "No user", strlen("No user") + 1);
                        break;
                    }
                    jn2 = json::parse(r->str);
                    grps.From_Json(jn2, grps);
                    grps.setflag(1);
                    grps.To_Json(jn2, grps);
                    freeReplyObject(r);
                    //将该群保存群成员的哈希中的该用户的权限改为1
                    r = Redis::hsetValue(c, pChat2.getFriendUid() + "group", pChat2.getMessage(), jn2.dump());
                    freeReplyObject(r);
                    //在用户保存自己加入的群的哈希中修改自己在该群的权限为管理员
                    r = Redis::hsetValue(c, pChat2.getMessage() + "groups", pChat2.getFriendUid(), jn2.dump());
                    freeReplyObject(r);
                    cout << "设置成功" << endl;
                    ssock::SendMsg(clnt_sock, "Yes", strlen("Yes") + 1);
                }
            }
        }
    } while (0);
    redisFree(c);

    //执行完添加后将文件描述符挂上监听红黑树
    ep.events = EPOLLIN | EPOLLET;
    ep.data.fd = clnt_sock;
    ret = epoll_ctl(efd, EPOLL_CTL_ADD, clnt_sock, &ep);
    if (ret == -1)
    {
        ssock::perr_exit("epoll_ctr error");
    }
}

// 29取消管理员
void gay::kickmanagepeople()
{
    privateChat pChat2;
    struct epoll_event ep;
    int clnt_sock = pChat.getServ_fd(); //获取服务器套间字
    char buf[BUFSIZ];
    int ret;
    json jn, jn2;
    groups grps;

    ssock::ReadMsg(clnt_sock, buf, sizeof(buf));
    jn = json::parse(buf);
    pChat2.From_Json(jn, pChat2);

    redisContext *c = Redis::RedisConnect("127.0.0.1", 6379);
    redisReply *r;
    do
    {
        r = Redis::exists(c, pChat2.getFriendUid() + "group"); //判断一个key是否已经存在，即判断群是否已经存在
        if (r == NULL)
        {
            printf("Execut getValue failure\n");
            break;
        }
        if (r->integer == 0)
        {
            printf("该群号数据库中没有\n");
            freeReplyObject(r);
            ssock::SendMsg(clnt_sock, "No group", strlen("No group") + 1);
            break;
        }
        else
        {
            freeReplyObject(r);

            r = Redis::hgethash(c, pChat2.getFriendUid() + "group", pChat2.getNumber());
            if (r == NULL)
            {
                printf("Execut getValue failure\n");
                break;
            }
            if (r->str == NULL) //判断该用户是否是群成员
            {
                printf("你不是该群成员\n");
                freeReplyObject(r);
                //你不是该群成员
                ssock::SendMsg(clnt_sock, "you are not a member of this group", strlen("you are not a member of this group") + 1);
                break;
            }
            else
            {
                jn2 = json::parse(r->str);
                grps.From_Json(jn2, grps);
                freeReplyObject(r);
                if (grps.getflag() != 2) //判断该用户是否是群主
                {
                    cout << "你不是该群群主" << endl;
                    ssock::SendMsg(clnt_sock, "not the group owner", strlen("not the group owner") + 1);
                    break;
                }
                else
                {
                    r = Redis::hgethash(c, pChat2.getFriendUid() + "group", pChat2.getMessage());
                    if (r == NULL)
                    {
                        printf("Execut getValue failure\n");
                        break;
                    }
                    if (r->str == NULL)
                    {
                        printf("要取消的管理员不是该群成员\n");
                        freeReplyObject(r);
                        //你不是该群成员
                        ssock::SendMsg(clnt_sock, "No user", strlen("No user") + 1);
                        break;
                    }
                    jn2 = json::parse(r->str);
                    grps.From_Json(jn2, grps);
                    grps.setflag(0);
                    grps.To_Json(jn2, grps);
                    freeReplyObject(r);
                    //将该群保存群成员的哈希中的该用户的权限改为1
                    r = Redis::hsetValue(c, pChat2.getFriendUid() + "group", pChat2.getMessage(), jn2.dump());
                    freeReplyObject(r);
                    //在用户保存自己加入的群的哈希中修改自己在该群的权限为管理员
                    r = Redis::hsetValue(c, pChat2.getMessage() + "groups", pChat2.getFriendUid(), jn2.dump());
                    freeReplyObject(r);
                    cout << "设置成功" << endl;
                    ssock::SendMsg(clnt_sock, "Yes", strlen("Yes") + 1);
                }
            }
        }
    } while (0);
    redisFree(c);

    //执行完添加后将文件描述符挂上监听红黑树
    ep.events = EPOLLIN | EPOLLET;
    ep.data.fd = clnt_sock;
    ret = epoll_ctl(efd, EPOLL_CTL_ADD, clnt_sock, &ep);
    if (ret == -1)
    {
        ssock::perr_exit("epoll_ctr error");
    }
}
// 30查看群组申请列表，进行同意或拒绝
void gay::groupapplication()
{
    privateChat pChat2, pChat3;
    struct epoll_event ep;
    int clnt_sock = pChat.getServ_fd(); //获取服务器套间字
    char buf[BUFSIZ];
    int ret;
    json jn, jn2;
    groups grps, grps2;

    ssock::ReadMsg(clnt_sock, buf, sizeof(buf));
    jn = json::parse(buf);
    pChat2.From_Json(jn, pChat2);

    redisContext *c = Redis::RedisConnect("127.0.0.1", 6379);
    redisReply *r;
    do
    {
        r = Redis::exists(c, pChat2.getFriendUid() + "group"); //判断一个key是否已经存在，即判断群是否已经存在
        if (r == NULL)
        {
            printf("Execut getValue failure\n");
            break;
        }
        if (r->integer == 0)
        {
            printf("该群号数据库中没有\n");
            freeReplyObject(r);
            ssock::SendMsg(clnt_sock, "No group", strlen("No group") + 1);
            break;
        }
        else
        {
            freeReplyObject(r);

            r = Redis::hgethash(c, pChat2.getFriendUid() + "group", pChat2.getNumber());
            if (r == NULL)
            {
                printf("Execut getValue failure\n");
                break;
            }
            if (r->str == NULL) //判断该用户是否是群成员
            {
                printf("你不是该群成员\n");
                freeReplyObject(r);
                //你不是该群成员
                ssock::SendMsg(clnt_sock, "you are not a member of this group", strlen("you are not a member of this group") + 1);
                break;
            }
            else
            {
                jn2 = json::parse(r->str);
                grps.From_Json(jn2, grps);
                freeReplyObject(r);
                if (grps.getflag() != 2 && grps.getflag() != 1) //判断用户是否是管理员或群主
                {
                    cout << "你只是普通成员，不能同意别人的好友申请" << endl;
                    ssock::SendMsg(clnt_sock, "You can't agree to someone else's friend request", strlen("You can't agree to someone else's friend request") + 1);
                    break;
                }
                else
                {
                    r = Redis::listlpop(c, pChat2.getFriendUid() + "message");
                    if (r == NULL)
                    {
                        printf("Execut getValue failure\n");
                        break;
                    }
                    if (r->str == NULL) //没人申请加群
                    {
                        printf("没有人申请加入群\n");
                        freeReplyObject(r);
                        ssock::SendMsg(clnt_sock, "No one has applied to join the group", strlen("No one has applied to join the group") + 1);
                        break;
                    }
                    else
                    {
                        jn = json::parse(r->str);
                        freeReplyObject(r);
                        ret = ssock::SendMsg(clnt_sock, jn.dump().c_str(), strlen(jn.dump().c_str()));
                        if (ret == -1)
                        {
                            qqqqquit(clnt_sock);
                            redisFree(c);
                            return;
                        }
                        ret = ssock::ReadMsg(clnt_sock, buf, sizeof(buf));
                        if (ret == 0)
                        {
                            qqqqquit(clnt_sock);
                            redisFree(c);
                            return;
                        }
                        if (strcmp(buf, "Yes") == 0)
                        {
                            pChat3.From_Json(jn, pChat3);
                            grps2.setflag(0);
                            grps2.setgroup_number(pChat3.getFriendUid());
                            grps2.setuser_number(pChat3.getNumber());
                            grps2.To_Json(jn2, grps2);
                            r = Redis::hsetValue(c, pChat3.getFriendUid() + "group", pChat3.getNumber(), jn2.dump()); //添加群信息
                            freeReplyObject(r);
                            r = Redis::hsetValue(c, pChat3.getNumber() + "groups", pChat3.getFriendUid(), jn2.dump()); //将群添加到自己已经加入的群哈希中
                            freeReplyObject(r);
                        }
                        else if (strcmp(buf, "No") == 0)
                        {
                        }
                    }
                }
            }
        }
    } while (0);
    redisFree(c);

    //执行完添加后将文件描述符挂上监听红黑树
    ep.events = EPOLLIN | EPOLLET;
    ep.data.fd = clnt_sock;
    ret = epoll_ctl(efd, EPOLL_CTL_ADD, clnt_sock, &ep);
    if (ret == -1)
    {
        ssock::perr_exit("epoll_ctr error");
    }
}

// 31踢人
void gay::kickpeople()
{
    privateChat pChat2;
    struct epoll_event ep;
    int clnt_sock = pChat.getServ_fd(); //获取服务器套间字
    char buf[BUFSIZ];
    int ret;
    json jn, jn2;
    groups grps, grps2;

    ssock::ReadMsg(clnt_sock, buf, sizeof(buf));
    jn = json::parse(buf);
    pChat2.From_Json(jn, pChat2);

    redisContext *c = Redis::RedisConnect("127.0.0.1", 6379);
    redisReply *r;
    do
    {
        r = Redis::exists(c, pChat2.getFriendUid() + "group"); //判断一个key是否已经存在，即判断群是否已经存在
        if (r == NULL)
        {
            printf("Execut getValue failure\n");
            break;
        }
        if (r->integer == 0)
        {
            printf("该群号数据库中没有\n");
            freeReplyObject(r);
            ssock::SendMsg(clnt_sock, "No group", strlen("No group") + 1);
            break;
        }
        else
        {
            freeReplyObject(r);

            r = Redis::hgethash(c, pChat2.getFriendUid() + "group", pChat2.getNumber());
            if (r == NULL)
            {
                printf("Execut getValue failure\n");
                break;
            }
            if (r->str == NULL) //判断该用户是否是群成员
            {
                printf("你不是该群成员\n");
                freeReplyObject(r);
                //你不是该群成员
                ssock::SendMsg(clnt_sock, "you are not a member of this group", strlen("you are not a member of this group") + 1);
                break;
            }
            else
            {
                jn2 = json::parse(r->str);
                grps.From_Json(jn2, grps);
                freeReplyObject(r);
                if (grps.getflag() != 2 && grps.getflag() != 1) //判断用户是否是管理员或群主
                {
                    cout << "你只是普通成员，不能踢人" << endl;
                    ssock::SendMsg(clnt_sock, "you can't kick people", strlen("you can't kick people") + 1);
                    break;
                }
                else
                {
                    r = Redis::hgethash(c, pChat2.getFriendUid() + "group", pChat2.getMessage());
                    if (r == NULL)
                    {
                        printf("Execut getValue failure\n");
                        break;
                    }
                    if (r->str == NULL)
                    {
                        printf("要踢的人不是该群成员\n");
                        freeReplyObject(r);
                        //你不是该群成员
                        ssock::SendMsg(clnt_sock, "No user", strlen("No user") + 1);
                        break;
                    }
                    jn2 = json::parse(r->str);
                    grps2.From_Json(jn2, grps2);
                    freeReplyObject(r);
                    if (grps.getflag() == 1 && grps2.getflag() >= 1)
                    {
                        cout << "踢人失败，你要踢的人也是管理员或是群主" << endl;
                        ssock::SendMsg(clnt_sock, "Failed to kick", strlen("Failed to kick") + 1);
                    }
                    else
                    {
                        //将该用户从保存该群成员的哈希中删除
                        r = Redis::hashdel(c, pChat2.getFriendUid() + "group", pChat2.getMessage());
                        freeReplyObject(r);
                        //将该群从要踢的人的群哈希里删除
                        r = Redis::hashdel(c, pChat2.getMessage() + "groups", pChat2.getFriendUid());
                        freeReplyObject(r);
                        cout << "踢人成功" << endl;
                        ssock::SendMsg(clnt_sock, "Yes", strlen("Yes") + 1);
                    }
                }
            }
        }
    } while (0);
    redisFree(c);

    //执行完添加后将文件描述符挂上监听红黑树
    ep.events = EPOLLIN | EPOLLET;
    ep.data.fd = clnt_sock;
    ret = epoll_ctl(efd, EPOLL_CTL_ADD, clnt_sock, &ep);
    if (ret == -1)
    {
        ssock::perr_exit("epoll_ctr error");
    }
}

void gay::history_groupmessage() // 33查看群历史消息记录
{
    struct epoll_event ep;
    int clnt_sock = pChat.getServ_fd();
    char buf[BUFSIZ];
    json jn;
    int ret;

    ssock::ReadMsg(clnt_sock, buf, sizeof(buf));
    jn = json::parse(buf);
    pChat.From_Json(jn, pChat);  //会修改pChat中本来的套间字的值
    pChat.setServ_fd(clnt_sock); //将套间字的值修改回去

    redisContext *c = Redis::RedisConnect("127.0.0.1", 6379);
    redisReply *r = Redis::listlrange(c, pChat.getNumber() + "grouphistory" + pChat.getFriendUid(), "0", "-1");
    if (r == NULL)
    {
        printf("Execut getValue failure\n");
        redisFree(c);
        return;
    }
    for (int i = 0; i < r->elements; ++i) //遍历消息记录
    {
        cout << r->element[i]->str << endl;
        cout << strlen(r->element[i]->str) + 1;
        ret = ssock::SendMsg(clnt_sock, r->element[i]->str, strlen(r->element[i]->str) + 1);
        if (ret == -1)
        {
            qqqqquit(clnt_sock);
            redisFree(c);
            return;
        }
    }
    ret = ssock::SendMsg(clnt_sock, "finish", strlen("finish") + 1);
    if (ret == -1)
    {
        qqqqquit(clnt_sock);
        redisFree(c);
        return;
    }
    redisFree(c);

    //执行完添加后将文件描述符挂上监听红黑树
    ep.events = EPOLLIN | EPOLLET;
    ep.data.fd = clnt_sock;
    ret = epoll_ctl(efd, EPOLL_CTL_ADD, clnt_sock, &ep);
    if (ret == -1)
    {
        ssock::perr_exit("epoll_ctr error");
    }
}
void gay::chat_send_group() // 34给群发消息
{
    int flag = 0; //定义一个标记，在第一次进入循环时改变自己的状态
    struct epoll_event ep;
    int clnt_sock = pChat.getServ_fd();
    char buf[BUFSIZ];
    json jn, jn2, jn3;
    onlineUser onlineU, onlineU2;

    redisContext *c;
    redisReply *r, *r2;
    c = Redis::RedisConnect("127.0.0.1", 6379);
    string message = "message";
    string grouphistory = "grouphistory";

    while (1)
    {
        int ret = ssock::ReadMsg(clnt_sock, buf, sizeof(buf));
        if (ret == 0)
        {
            qqqqquit(clnt_sock); //下线
            redisFree(c);
            return;
        }

        cout << buf << endl;
        jn = json::parse(buf);
        pChat.From_Json(jn, pChat);  //将序列转为类，会修改原有的套间字
        pChat.setServ_fd(clnt_sock); //将套间字修改回去
        if (flag == 0)
        {
            r = Redis::hgethash(c, "Onlineuser", pChat.getNumber());
            jn2 = json::parse(r->str);
            onlineU.From_Json(jn2, onlineU); //将自己的状态改为聊天
            onlineU.setflag(CHAT_SEND_GROUP);
            onlineU.To_Json(jn2, onlineU); //将jn修改
            freeReplyObject(r);
            r = Redis::hsetValue(c, "Onlineuser", onlineU.getUid(), jn2.dump()); //将自己修改后的状态写入数据库
            freeReplyObject(r);
        }

        if (strcmp(pChat.getMessage().c_str(), "exit") == 0) //如果发送exit，就退出私聊，然后给自己发exit让自己的接受消息的线程退出
        {
            ret = ssock::SendMsg(clnt_sock, jn.dump().c_str(), strlen(jn.dump().c_str()) + 1); //将消息转发给自己
            if (ret == -1)
            {
                qqqqquit(clnt_sock);
                redisFree(c);
                return;
            }
            //将消息写入自己的列表里。意味着自己关闭，continue_receive线程需要开始工作了
            r = Redis::listrpush(c, pChat.getNumber() + "message", jn.dump().c_str());
            freeReplyObject(r);
            break;
        }

        r = Redis::exists(c, pChat.getFriendUid() + "group"); //判断一个key是否已经存在，即判断群是否已经存在
        if (r == NULL)
        {
            printf("Execut getValue failure\n");
            break;
        }
        if (r->integer == 0)
        {
            printf("该群号数据库中没有\n");
            freeReplyObject(r);
            ret = ssock::SendMsg(clnt_sock, "No group", strlen("No group") + 1);
            if (ret == -1)
            {
                qqqqquit(clnt_sock);
                redisFree(c);
                return;
            }
            continue;
        }
        else
        {
            freeReplyObject(r);

            r = Redis::hgethash(c, pChat.getFriendUid() + "group", pChat.getNumber());
            if (r == NULL)
            {
                printf("Execut getValue failure\n");
                break;
            }
            if (r->str == NULL) //判断该用户是否是群成员
            {
                printf("你不是该群成员\n");
                freeReplyObject(r);
                //你不是该群成员
                ret = ssock::SendMsg(clnt_sock, "you are not a member of this group", strlen("you are not a member of this group") + 1);
                if (ret == -1)
                {
                    qqqqquit(clnt_sock);
                    redisFree(c);
                    return;
                }
                continue;
            }
            else
            {
                freeReplyObject(r);
                r = Redis::hgethashall(c, pChat.getFriendUid() + "group");
                if (r == NULL)
                {
                    printf("Execut getValue failure\n");
                    break;
                }
                for (int i = 0; i < r->elements; ++i) //遍历群成员
                {
                    if (i % 2 == 0) //偶数是key，奇数是value
                    {
                        if (strcmp(r->element[i]->str, pChat.getNumber().c_str()) == 0)
                        {
                            //将聊天记录添加到自己的历史记录里
                            r2 = Redis::listrpush(c, r->element[i]->str + grouphistory + pChat.getFriendUid(), jn.dump().c_str());
                            freeReplyObject(r2);
                            continue;
                        }
                        r2 = Redis::hgethash(c, "Onlineuser", r->element[i]->str); //获取群用户的在线信息
                        if (r2 == NULL)
                        {
                            printf("Execut getValue failure\n");
                            break;
                        }
                        if (r2->str == NULL) //该用户不在线，将聊天记录添加到历史记录中
                        {
                            printf("该用户不在线\n");
                            freeReplyObject(r2);
                            r2 = Redis::listrpush(c, r->element[i]->str + message, jn.dump().c_str());
                            freeReplyObject(r2);

                            //将聊天记录添加到历史记录里
                            r2 = Redis::listrpush(c, r->element[i]->str + grouphistory + pChat.getFriendUid(), jn.dump().c_str());
                            freeReplyObject(r2);
                        }
                        else
                        {
                            jn3 = json::parse(r2->str);
                            onlineU2.From_Json(jn3, onlineU2);
                            freeReplyObject(r2);
                            if (onlineU2.getflag() == CHAT_SEND_GROUP) //对方也正在群聊
                            {
                                ret = ssock::SendMsg(onlineU2.getsock(), jn.dump().c_str(), strlen(jn.dump().c_str()) + 1); //将消息转发给好友
                                if (ret == -1)
                                {
                                    qqqqquit(clnt_sock);
                                    redisFree(c);
                                    return;
                                }
                            }
                            //将消息写入一个列表里，然后在客户端从该列表中读取数据
                            r2 = Redis::listrpush(c, r->element[i]->str + message, jn.dump().c_str());
                            freeReplyObject(r2);
                            //将聊天记录添加到历史记录里
                            r2 = Redis::listrpush(c, r->element[i]->str + grouphistory + pChat.getFriendUid(), jn.dump().c_str());
                            freeReplyObject(r2);
                        }
                    }
                }
                freeReplyObject(r);
            }
        }
    }

    r = Redis::hgethash(c, "Onlineuser", pChat.getNumber());
    cout << "r->str = " << r->str;
    jn2 = json::parse(r->str);
    onlineU.From_Json(jn2, onlineU); //将自己的状态改为不聊天
    onlineU.setflag(0);
    onlineU.To_Json(jn2, onlineU); //将jn修改
    freeReplyObject(r);
    r = Redis::hsetValue(c, "Onlineuser", onlineU.getUid(), jn2.dump()); //将自己修改后的状态写入数据库
    freeReplyObject(r);
    redisFree(c);

    //执行完添加后将文件描述符挂上监听红黑树
    ep.events = EPOLLIN | EPOLLET;
    ep.data.fd = clnt_sock;
    int ret = epoll_ctl(efd, EPOLL_CTL_ADD, clnt_sock, &ep);
    if (ret == -1)
    {
        ssock::perr_exit("epoll_ctr error");
    }
}
void gay::send_file_group() // 35从客户端读文件
{
    struct epoll_event ep;
    int clnt_sock = pChat.getServ_fd();
    char buf[BUFSIZ];
    json jn, jn2, jn3;

    friends fri, fri2;
    redisContext *c;
    redisReply *r, *r2;
    string message = "message";
    string file1 = "file1";

    int ret = ssock::ReadMsg(clnt_sock, buf, sizeof(buf));
    cout << "ret = " << ret << endl;
    if (ret == 0)
    {
        qqqqquit(clnt_sock); //下线
        return;
    }

    cout << buf << endl;
    jn = json::parse(buf);
    pChat.From_Json(jn, pChat);  //将序列转为类，会修改原有的套间字
    pChat.setServ_fd(clnt_sock); //将套间字修改回去

    do
    {
        c = Redis::RedisConnect("127.0.0.1", 6379);

        r = Redis::exists(c, pChat.getFriendUid() + "group"); //判断一个key是否已经存在，即判断群是否已经存在
        if (r == NULL)
        {
            printf("Execut getValue failure\n");
            break;
        }
        if (r->integer == 0)
        {
            printf("该群号数据库中没有\n");
            freeReplyObject(r);
            ret = ssock::SendMsg(clnt_sock, "No group", strlen("No group") + 1);
            if (ret == -1)
            {
                qqqqquit(clnt_sock);
                redisFree(c);
                return;
            }
            break;
        }
        else
        {
            freeReplyObject(r);

            r = Redis::hgethash(c, pChat.getFriendUid() + "group", pChat.getNumber());
            if (r == NULL)
            {
                printf("Execut getValue failure\n");
                break;
            }
            if (r->str == NULL) //判断该用户是否是群成员
            {
                printf("你不是该群成员\n");
                freeReplyObject(r);
                //你不是该群成员
                ssock::SendMsg(clnt_sock, "you are not a member of this group", strlen("you are not a member of this group") + 1);
                break;
            }
            else
            {
                ssock::SendMsg(clnt_sock, "Yes", strlen("Yes") + 1);

                string filename(pChat.getMessage()); //文件名
                auto f = filename.rfind('/');
                filename.erase(0, f + 1);
                filename.insert(0, "../Temporaryfiles/");
                DIR *d = opendir("../Temporaryfiles");
                if (d == NULL)
                {
                    mkdir("../Temporaryfiles", 0777);
                }
                FILE *fp = fopen(filename.c_str(), "w");

                int n;
                //将文件存入本地
                __off_t size;
                ret = ssock::ReadMsg(clnt_sock, &size, sizeof(size));
                if (ret == 0)
                {
                    qqqqquit(clnt_sock);
                    redisFree(c);
                    return;
                }
                size = ntohll(size);
                cout << "size = " << size << endl;
                while (size > 0)
                {
                    if (sizeof(buf) < size)
                    {
                        n = ssock::Readn(clnt_sock, buf, sizeof(buf));
                    }
                    else
                    {
                        n = ssock::Readn(clnt_sock, buf, size);
                    }
                    if (n < 0)
                    {
                        continue;
                    }
                    if (n == 0)
                    {
                        qqqqquit(clnt_sock);
                        redisFree(c);
                        return;
                    }
                    size -= n;
                    fwrite(buf, n, 1, fp);
                    cout << "size = " << size << endl;
                }

                fclose(fp);

                r = Redis::hgethashall(c, pChat.getFriendUid() + "group");
                if (r == NULL)
                {
                    printf("Execut getValue failure\n");
                    break;
                }
                for (int i = 0; i < r->elements; ++i) //遍历群成员
                {
                    if (i % 2 == 0) //偶数是key，奇数是value
                    {
                        if (strcmp(r->element[i]->str, pChat.getNumber().c_str()) == 0)
                        {
                            continue;
                        }

                        //将消息写入一个列表里，然后在客户端从该列表中读取数据，提醒客户端有数据来了
                        r2 = Redis::listrpush(c, r->element[i]->str + message, jn.dump().c_str());
                        freeReplyObject(r2);

                        //将消息写到一个列表里（包括其中的文件名），让客户端可以知道自己要哪个文件
                        r2 = Redis::listrpush(c, r->element[i]->str + file1, jn.dump().c_str());
                        freeReplyObject(r2);
                    }
                }
            }
        }
    } while (0);
    redisFree(c);

    //执行完添加后将文件描述符挂上监听红黑树
    ep.events = EPOLLIN | EPOLLET;
    ep.data.fd = clnt_sock;
    ret = epoll_ctl(efd, EPOLL_CTL_ADD, clnt_sock, &ep);
    if (ret == -1)
    {
        ssock::perr_exit("epoll_ctr error");
    }
}
void gay::recv_file_group() // 36给群成员发文件
{
    privateChat pChat2;
    struct epoll_event ep;
    int clnt_sock = pChat.getServ_fd();
    char buf[BUFSIZ];
    json jn, jn2;
    int ret;

    ret = ssock::ReadMsg(clnt_sock, buf, sizeof(buf));
    if (ret == 0)
    {
        qqqqquit(clnt_sock);
        cout << "clnt_sock"
             << "关闭" << endl;
        return;
    }
    jn = json::parse(buf);
    pChat.From_Json(jn, pChat);  //将序列转为类，会修改原有的套间字
    pChat.setServ_fd(clnt_sock); //将套间字修改回去

    redisContext *c = Redis::RedisConnect("127.0.0.1", 6379);
    redisReply *r = Redis::listlen(c, pChat.getNumber() + "file1");
    if (r == NULL)
    {
        printf("Execut getValue failure\n");
        redisFree(c);
        return;
    }
    int listlen = r->integer;
    freeReplyObject(r);

    for (int i = 0; i < listlen; ++i)
    {
        r = Redis::listlpop(c, pChat.getNumber() + "file1");
        cout << r->str << endl;
        jn2 = json::parse(r->str);
        pChat2.From_Json(jn2, pChat2); //将会修改pChat中本来的文件描述符
        ret = ssock::SendMsg(clnt_sock, r->str, strlen(r->str) + 1);
        if (ret == -1)
        {
            qqqqquit(clnt_sock);
            freeReplyObject(r);
            redisFree(c);
            return;
        }
        freeReplyObject(r);
        ret = ssock::ReadMsg(clnt_sock, buf, sizeof(buf));
        if (ret == 0)
        {
            qqqqquit(clnt_sock);
            redisFree(c);
            return;
        }
        if (strcmp(buf, "Yes") == 0)
        {
            cout << "同意接收" << endl;
            string filename(pChat2.getMessage()); //文件名
            auto f = filename.rfind('/');
            filename.erase(0, f + 1);
            filename.insert(0, "../Temporaryfiles/");
            int filefd = open(filename.c_str(), O_RDONLY);

            __off_t size;
            struct stat file_stat;
            //为了获取文件大小
            fstat(filefd, &file_stat);
            size = file_stat.st_size;
            size = htonll(size);

            ret = ssock::SendMsg(clnt_sock, (void *)&size, sizeof(size));
            if (ret == -1)
            {
                qqqqquit(clnt_sock);
                redisFree(c);
                return;
            }
            cout << "file_stat.st_size = " << file_stat.st_size << endl;
            while ((ret = sendfile(clnt_sock, filefd, NULL, file_stat.st_size)) != 0)
            {
                cout << "ret = " << ret << endl;
                if (ret == -1 && errno == EPIPE)
                {
                    qqqqquit(clnt_sock);
                    redisFree(c);
                    return;
                }
            }
            cout << "ret = " << ret << endl;

            close(filefd);
        }
        else
        {
            cout << "拒绝接收" << endl;
        }
    }
    ret = ssock::SendMsg(clnt_sock, "finish", strlen("finish") + 1);
    if (ret == -1)
    {
        qqqqquit(clnt_sock);
        redisFree(c);
        return;
    }

    redisFree(c);

    //执行完添加后将文件描述符挂上监听红黑树
    ep.events = EPOLLIN | EPOLLET;
    ep.data.fd = clnt_sock;
    ret = epoll_ctl(efd, EPOLL_CTL_ADD, clnt_sock, &ep);
    if (ret == -1)
    {
        ssock::perr_exit("epoll_ctr error");
    }
}

// 38接收未接收完的文件
void gay::send_part_file()
{
    struct epoll_event ep;
    int clnt_sock = pChat.getServ_fd();
    char buf[BUFSIZ];
    json jn, jn2, jn3;

    friends fri;
    redisContext *c;
    redisReply *r;

    int ret = ssock::ReadMsg(clnt_sock, buf, sizeof(buf));
    cout << "ret = " << ret << endl;
    if (ret == 0)
    {
        qqqqquit(clnt_sock); //下线
        return;
    }

    cout << buf << endl;
    jn = json::parse(buf);
    pChat.From_Json(jn, pChat);  //将序列转为类，会修改原有的套间字
    pChat.setServ_fd(clnt_sock); //将套间字修改回去

    do
    {
        c = Redis::RedisConnect("127.0.0.1", 6379);

        //判断是否有未发送完的文件
        r = Redis::listlpop(c, pChat.getNumber() + "partfile1");
        if (r == NULL)
        {
            printf("Execut getValue failure\n");
            redisFree(c);
            break;
        }
        if (r->str == NULL)
        {
            ssock::SendMsg(clnt_sock, "No file", strlen("No file") + 1);
            freeReplyObject(r);
            break;
        }
        else
        {
            ssock::SendMsg(clnt_sock, "Yes", strlen("Yes") + 1);
        }
        jn2 = json::parse(r->str);
        freeReplyObject(r);
        pChat.From_Json(jn2, pChat);
        pChat.setServ_fd(clnt_sock); //将套间字修改回去

        //判断发送者发送的人是否是它的好友
        r = Redis::hsetexist(c, pChat.getNumber() + "friend", pChat.getFriendUid());
        if (r == NULL)
        {
            printf("Execut getValue failure\n");
            redisFree(c);
            break;
        }
        if (r->integer == 0) //没有该好友
        {
            cout << "没有该好友" << endl;
            ssock::SendMsg(clnt_sock, "No friend", strlen("No friend") + 1); //将消息转发给自己
            freeReplyObject(r);
            redisFree(c);
            break;
        }
        else
        {
            ssock::SendMsg(clnt_sock, "Has friend", strlen("Has friend") + 1); //将消息转发给自己
        }
        freeReplyObject(r);
        //判断它是否被它的好友屏蔽了它
        r = Redis::hgethash(c, pChat.getFriendUid() + "friend", pChat.getNumber());
        if (r == NULL)
        {
            printf("Execut getValue failure\n");
            redisFree(c);
            break;
        }
        jn3 = json::parse(r->str);
        fri.From_Json(jn3, fri);
        if (fri.getflag() == 0)
        {
            cout << "屏蔽了" << endl;
            ssock::SendMsg(clnt_sock, "Has block", strlen("Has block") + 1); //将消息转发给自己
            freeReplyObject(r);
            redisFree(c);
            break;
        }
        else
        {
            ssock::SendMsg(clnt_sock, "No block", strlen("No block") + 1);
        }
        freeReplyObject(r);

        ssock::SendMsg(clnt_sock, jn2.dump().c_str(), strlen(jn2.dump().c_str()) + 1); //给对端发送消息，让对端确认是否继续发送未发完的文件
        ret = ssock::ReadMsg(clnt_sock, buf, sizeof(buf));
        if (ret == 0)
        {
            qqqqquit(clnt_sock);
            redisFree(c);
            return;
        }
        if (strcmp(buf, "No") == 0) //对端发No代表不继续发送文件，结束该函数
        {
            break;
        }

        string filename(pChat.getMessage()); //文件名
        auto f = filename.rfind('/');
        filename.erase(0, f + 1);
        filename.insert(0, "../Temporaryfiles/");
        DIR *d = opendir("../Temporaryfiles");
        if (d == NULL)
        {
            mkdir("../Temporaryfiles", 0777);
        }

        struct stat file_stat;
        __off_t size, len;
        int filefd = open(filename.c_str(), O_RDONLY);

        fstat(filefd, &file_stat);
        len = file_stat.st_size;
        len = htonll(len);
        close(filefd);
        ssock::SendMsg(clnt_sock, (void *)&len, sizeof(len)); //向对端发送已经接收的文件长度

        FILE *fp = fopen(filename.c_str(), "a");

        int n;
        //将文件存入本地
        ret = ssock::ReadMsg(clnt_sock, &size, sizeof(size));
        if (ret == 0)
        {
            qqqqquit(clnt_sock);
            redisFree(c);
            return;
        }

        len = 0; //用来保存已经接收的文件大小
        size = ntohll(size);
        cout << "size = " << size << endl;
        while (size > 0)
        {
            if (sizeof(buf) < size)
            {
                n = ssock::Readn(clnt_sock, buf, sizeof(buf));
            }
            else
            {
                n = ssock::Readn(clnt_sock, buf, size);
            }
            if (n < 0)
            {
                continue;
            }
            if (n == 0)
            {
                cout << "len = " << len << endl; //已经接收的文件长度
                r = Redis::listrpush(c, pChat.getNumber() + "partfile1", jn2.dump());
                freeReplyObject(r);
                qqqqquit(clnt_sock);
                redisFree(c);
                return;
            }
            len += n;
            size -= n;
            fwrite(buf, n, 1, fp);
            cout << "size = " << size << endl;
        }

        //将消息写入一个列表里，然后在客户端从该列表中读取数据，提醒客户端有数据来了
        r = Redis::listrpush(c, pChat.getFriendUid() + "message", jn2.dump().c_str());
        freeReplyObject(r);
        //将消息写到一个列表里（包括其中的文件名），让客户端可以知道自己要哪个文件
        r = Redis::listrpush(c, pChat.getFriendUid() + "file", jn2.dump().c_str());
        freeReplyObject(r);

        fclose(fp);
        redisFree(c);
    } while (0);

    //执行完添加后将文件描述符挂上监听红黑树
    ep.events = EPOLLIN | EPOLLET;
    ep.data.fd = clnt_sock;
    ret = epoll_ctl(efd, EPOLL_CTL_ADD, clnt_sock, &ep);
    if (ret == -1)
    {
        ssock::perr_exit("epoll_ctr error");
    }
}

// 100一直发消息的线程
void *continue_send(void *arg)
{
    gay g = *((gay *)arg);

    pthread_mutex_unlock(&(((gay *)arg)->getMutex()));

    int clnt_sock = g.getpChat().getServ_fd();
    struct epoll_event ep;
    char number[21];
    string message = "message";

    int ret = ssock::ReadMsg(clnt_sock, number, sizeof(number));
    if (ret == 0)
    {
        qqqqquit(clnt_sock);
        cout << "clnt_sock"
             << "关闭" << endl;
        return NULL;
    }

    redisContext *c = Redis::RedisConnect("127.0.0.1", 6379);
    redisReply *r = Redis::listlen(c, number + message);
    if (r == NULL)
    {
        printf("Execut getValue failure\n");
        redisFree(c);
    }
    int listlen = r->integer;
    freeReplyObject(r);

    for (int i = 0; i < listlen; ++i)
    {
        r = Redis::listlpop(c, number + message);
        ret = ssock::SendMsg(clnt_sock, r->str, strlen(r->str) + 1);
        if (ret == -1)
        {
            qqqqquit(clnt_sock);
            freeReplyObject(r);
            redisFree(c);
            return NULL;
        }
        freeReplyObject(r);
    }
    ret = ssock::SendMsg(clnt_sock, "finish", strlen("finish") + 1);
    if (ret == -1)
    {
        qqqqquit(clnt_sock);
        redisFree(c);
        return NULL;
    }

    redisFree(c);

    //执行完添加后将文件描述符挂上监听红黑树
    ep.events = EPOLLIN | EPOLLET;
    ep.data.fd = clnt_sock;
    ret = epoll_ctl(g.getefd(), EPOLL_CTL_ADD, clnt_sock, &ep);
    if (ret == -1)
    {
        ssock::perr_exit("epoll_ctr error");
    }

    return NULL;
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