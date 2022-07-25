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
#include <fcntl.h>
#include "message.hpp"
#include "redis1.hpp"
#include "threadpool.hpp"
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
    void delFriend();          // 11删除好友
    void findFriend();         // 12查询好友
    void onlineStatus();       // 13显示好友在线信息

    privateChat &getpChat();     //得到pChat对象
    pthread_mutex_t &getMutex(); //获得锁
    void talkwithfriends();      // 17和朋友聊天

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
    if (g.getpChat().getFlag() == 9) // 9退出登录
    {
        g.signout();
    }
    else if ((g.getpChat().getFlag() == 10)) // 10添加好友
    {
        g.addFriend();
    }
    else if ((g.getpChat().getFlag() == 11))
    {
        g.delFriend(); // 11删除好友
    }
    else if ((g.getpChat().getFlag() == 12))
    {
        g.findFriend(); // 12查询好友
    }
    else if ((g.getpChat().getFlag() == 13))
    {
        g.onlineStatus(); // 13显示好友在线状态
    }
    else if ((g.getpChat().getFlag() == 14))
    {
    }
    else if ((g.getpChat().getFlag() == 15))
    {
    }
    else if ((g.getpChat().getFlag() == 16))
    {
    }
    else if (g.getpChat().getFlag() == 17) // 17和好友聊天
    {
        g.talkwithfriends();
    }
    else if ((g.getpChat().getFlag() == 18))
    {
    }
}

void gay::setEfd(const int &e)
{
    efd = e;
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

void gay::addFriend()
{
    struct epoll_event ep;
    int clnt_sock = pChat.getServ_fd();
    char buf[BUFSIZ];
    json jn;

    ssock::ReadMsg(clnt_sock, buf, BUFSIZ);
    jn = json::parse(buf);

    pChat.From_Json(jn, pChat); //将会修改pChat中本来的文件描述符
    pChat.setServ_fd(clnt_sock);
    friends fri; //定义一个friends类，将获取到的朋友信息进行解析

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
            ssock::SendMsg(clnt_sock, "Added successfully", strlen("Added successfully") + 1);
            freeReplyObject(r);
            fri.setflag(1);                         //设置朋友的权限
            fri.setfriendUid(pChat.getFriendUid()); //设置朋友的uid
            fri.To_Json(jn, fri);                   //将fri转换为json

            //将转换后的字符串写入数据库
            r = Redis::hsetValue(c, pChat.getNumber() + "friend", fri.getfriendUid(), jn.dump()); // 1表示正常，0表示屏蔽
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

// 11删除好友
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

        //删除好友
        r = Redis::hashdel(c, pChat.getNumber() + "friend", pChat.getFriendUid());
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

// 12查询好友
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

void gay::onlineStatus() // 13显示好友在线信息
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

privateChat &gay::getpChat()
{
    return pChat;
}

pthread_mutex_t &gay::getMutex() //获得锁
{
    return mutex;
}

// 17和好友聊天
void gay::talkwithfriends()
{
    struct epoll_event ep;
    int clnt_sock = pChat.getServ_fd();
    char buf[BUFSIZ];
    json jn, jn2;

    friends fri;
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

        if (strcmp(pChat.getMessage().c_str(), "exit") == 0) //如果发送exit，就退出私聊，然后给自己发exit让自己的接受消息的线程退出
        {
            ssock::SendMsg(clnt_sock, jn.dump().c_str(), strlen(jn.dump().c_str()) + 1); //将消息转发给自己
            break;
        }

        c = Redis::RedisConnect("127.0.0.1", 6379);
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
        ssock::SendMsg(fri.getflag(), jn.dump().c_str(), strlen(jn.dump().c_str()) + 1); //将消息转发给好友

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

#endif