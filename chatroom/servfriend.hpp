#ifndef SERVFRIEND_H
#define SERVFRIEND_H
#include <iostream>
#include <cstring>
#include <string>
#include <set>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "ssock.hpp"
#include "redis1.hpp"
#include <hiredis/hiredis.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/sendfile.h>
#include <fcntl.h>
#include "message.hpp"
#include "redis1.hpp"
#include "threadpool.hpp"
#include "macro.h"
using namespace std;

void qqqqquit(int clnt_sock) //将其从在线用户中删除
{
    json jn;
    friends fri;
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
            fri.From_Json(jn, fri);
            if (fri.getflag() == clnt_sock) //将value中的套间字和clnt_sock比较，相同就删除该用户
            {
                freeReplyObject(r);
                r = Redis::hashdel(c, "Onlineuser", fri.getfriendUid());
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

    //将通过ctrl+c退出的程序从在线用户中删除，不需要close文件描述符，主函数已经做了
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
    if (g.getpChat().getFlag() == SIGNOUT) // 9退出登录
    {
        g.signout();
    }
    else if ((g.getpChat().getFlag() == ADDFRIEND)) // 10添加好友
    {
        g.addFriend();
    }
    else if (g.getpChat().getFlag() == INQUIREADD) // 11查看好友添加信息
    {
        g.inquqireAdd();
    }
    else if ((g.getpChat().getFlag() == DELFRIEND)) // 12删除好友
    {
        g.delFriend();
    }
    else if ((g.getpChat().getFlag() == FINDFRIEND)) // 13查询好友
    {
        g.findFriend();
    }
    else if ((g.getpChat().getFlag() == ONLINESTATUS)) // 14显示好友在线状态
    {
        g.onlineStatus();
    }
    else if ((g.getpChat().getFlag() == BLOCKFRIEND)) // 15屏蔽好友消息
    {
        g.blockFriend();
    }
    else if ((g.getpChat().getFlag() == HISTORY_MESSAGE)) // 17查看历史聊天记录
    {
        g.history_message();
    }
    else if (g.getpChat().getFlag() == CHAT_SEND_FRIEND) // 18和好友聊天
    {
        g.talkwithfriends();
    }
    else if ((g.getpChat().getFlag() == SEND_FILE)) // 19接收客户端的文件
    {
        g.send_file();
    }
    else if ((g.getpChat().getFlag() == RECV_FILE)) // 20向客户端发文件
    {
        g.recv_file();
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

    ssock::ReadMsg(clnt_sock, buf, BUFSIZ);
    jn = json::parse(buf);

    pChat.From_Json(jn, pChat);  //将会修改pChat中本来的文件描述符
    pChat.setServ_fd(clnt_sock); //将文件描述符改回去

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
    close(pChat.getServ_fd());

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
            r = Redis::listrpush(c, pChat.getFriendUid() + "addfriend", jn.dump()); //将添加好友的信息放到数据库等待对方同意
            freeReplyObject(r);
            //将消息放到接收好友请求的用户的数据库中，提醒该用户有人添加它
            r = Redis::listrpush(c, pChat.getFriendUid() + "message", jn.dump());
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
            return;
        }
        freeReplyObject(r);
        ret = ssock::ReadMsg(clnt_sock, buf, sizeof(buf));
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
            if (r2->integer == 0) //好友不在线
            {
                ssock::SendMsg(clnt_sock, r->element[i + 1]->str, strlen(r->element[i + 1]->str) + 1); //将得到的value发送给对端
            }
            else //好友在线
            {
                freeReplyObject(r2);
                r2 = Redis::hgethash(c, "Onlineuser", r->element[i]->str);
                ssock::SendMsg(clnt_sock, r2->str, strlen(r2->str) + 1);
            }
            freeReplyObject(r2);
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
        ssock::SendMsg(clnt_sock, r->element[i]->str, strlen(r->element[i]->str) + 1);
    }
    ssock::SendMsg(clnt_sock, "finish", strlen("finish") + 1);

    //执行完添加后将文件描述符挂上监听红黑树
    ep.events = EPOLLIN | EPOLLET;
    ep.data.fd = clnt_sock;
    int ret = epoll_ctl(efd, EPOLL_CTL_ADD, clnt_sock, &ep);
    if (ret == -1)
    {
        ssock::perr_exit("epoll_ctr error");
    }
}

// 18和好友聊天
void gay::talkwithfriends()
{
    struct epoll_event ep;
    int clnt_sock = pChat.getServ_fd();
    char buf[BUFSIZ];
    json jn, jn2, jn3;

    friends fri, fri2;
    redisContext *c;
    redisReply *r;

    while (1)
    {
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

        c = Redis::RedisConnect("127.0.0.1", 6379);
        if (strcmp(pChat.getMessage().c_str(), "exit") == 0) //如果发送exit，就退出私聊，然后给自己发exit让自己的接受消息的线程退出
        {
            ssock::SendMsg(clnt_sock, jn.dump().c_str(), strlen(jn.dump().c_str()) + 1); //将消息转发给自己
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
            ssock::SendMsg(clnt_sock, "No friend", strlen("No friend") + 1); //将消息转发给自己
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
        fri2.From_Json(jn3, fri2);
        if (fri2.getflag() == 0)
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
        fri.From_Json(jn2, fri);
        cout << jn.dump() << endl;
        ret = ssock::SendMsg(fri.getflag(), jn.dump().c_str(), strlen(jn.dump().c_str()) + 1); //将消息转发给好友

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

    //执行完添加后将文件描述符挂上监听红黑树
    ep.events = EPOLLIN | EPOLLET;
    ep.data.fd = clnt_sock;
    int ret = epoll_ctl(efd, EPOLL_CTL_ADD, clnt_sock, &ep);
    if (ret == -1)
    {
        ssock::perr_exit("epoll_ctr error");
    }
}

// 19从客户端读文件
void gay::send_file()
{
    struct epoll_event ep;
    int clnt_sock = pChat.getServ_fd();
    char buf[BUFSIZ];
    json jn, jn2, jn3;

    friends fri, fri2;
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
        fri2.From_Json(jn3, fri2);
        if (fri2.getflag() == 0)
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
        FILE *fp = fopen(filename.c_str(), "w");

        //将消息写入一个列表里，然后在客户端从该列表中读取数据，提醒客户端有数据来了
        r = Redis::listrpush(c, pChat.getFriendUid() + "message", jn.dump().c_str());
        freeReplyObject(r);
        //将消息写到一个列表里（包括其中的文件名），让客户端可以知道自己要哪个文件
        r = Redis::listrpush(c, pChat.getFriendUid() + "file", jn.dump().c_str());
        freeReplyObject(r);

        int n;
        //将文件存入本地
        __off_t size;
        ssock::ReadMsg(clnt_sock, &size, sizeof(size));
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
            size -= n;
            fwrite(buf, n, 1, fp);
        }

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
            return;
        }
        freeReplyObject(r);
        ret = ssock::ReadMsg(clnt_sock, buf, sizeof(buf));
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
            size = htonl(size);
            ssock::SendMsg(clnt_sock, (void *)&size, sizeof(file_stat.st_size));
            sendfile(clnt_sock, filefd, NULL, file_stat.st_size);

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
            return NULL;
        }
        freeReplyObject(r);
    }
    ret = ssock::SendMsg(clnt_sock, "finish", strlen("finish") + 1);
    if (ret == -1)
    {
        qqqqquit(clnt_sock);
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

#endif