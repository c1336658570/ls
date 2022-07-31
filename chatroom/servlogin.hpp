/*
g++ -o account account.cc account.hpp ssock.cpp -lhiredis
*/
#ifndef SERV_H
#define SERV_H
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
#include <signal.h>
#include "message.hpp"
#include "redis1.hpp"
#include "threadpool.hpp"
#include "macro.h"
using namespace std;

#define OPEN_MAX 5000

void startlogin(void *arg);
void startpchat(void *arg);

class account
{
public:
    account();
    ~account();
    bool login();              //从数据库读数据并与登陆输入的数据比较
    void reg();                //注册
    void retrieve();           //找回密码
    void setEfd(const int &e); //设置监听红黑树根节点
    User &getUser();           //获得u这个对象
    pthread_mutex_t &getMutex();

    // void setfriendfd(const string &s, const int &f); //设置用户名和fd
    // map<string, int> &getfriendfd();                 //获取用户名和fd

private:
    pthread_mutex_t mutex; //定义锁
    int efd;               //监听红黑书根节点，用于将文件描述符挂上树
    User u;                //用户信息
    //在数据库中实现了
    // map<string, int> friendfd; //保存用户名和fd
};

account::account()
{
    pthread_mutex_init(&mutex, NULL);
}
account::~account()
{
    pthread_mutex_destroy(&mutex);
}

pthread_mutex_t &account::getMutex()
{
    return mutex;
}

void startlogin(void *arg)
{
    account s = *((account *)arg);
    pthread_mutex_unlock(&(((account *)arg)->getMutex()));
    if ((s.getUser()).getFlag() == LOGIN)
    {
        // bool ret =
        s.login();
        /*
        在数据库中实现在线用户的添加
        if (ret == true)
        {
            //将登录成功的fd加入到map中，必须用s的User对象中的Number和serv_fd，
            //因为在主线程中没有读取number只是设置了efd和serv_fd，number为空，serv_fd也可能在后续的代码逻辑中被修改
            (*((account *)arg)).setfriendfd(s.getUser().getNumber(), s.getUser().getServ_fd());
            cout << "serv_fd = " << (*((account *)arg)).getfriendfd().at(s.getUser().getNumber()) << endl;
        }
        */
    }
    else if ((s.getUser()).getFlag() == REGISTER)
    {
        s.reg();
    }
    else if ((s.getUser()).getFlag() == RETRIEVE)
    {
        s.retrieve();
    }
}

void setsp()
{
    sigset_t set;

    sigemptyset(&set);
    sigaddset(&set, SIGPIPE);
    sigaddset(&set, SIGBUS);
    sigprocmask(SIG_BLOCK, &set, NULL);
}

//从数据库读数据并与登陆输入的数据比较
bool account::login()
{
    struct epoll_event ep;
    int clnt_sock = u.getServ_fd();
    json jn;
    char buf[BUFSIZ];

    cout << u.getServ_fd();
    ssock::ReadMsg(clnt_sock, buf, sizeof(buf));
    cout << buf << endl;

    jn = json::parse(buf);
    u.From_Json(jn, u);      //将会修改U中本来的内容，导致u中的文件描述符改变
    u.setServ_fd(clnt_sock); //重新设置u中的serv_fd

    //打开数据库
    redisContext *c = Redis::RedisConnect("127.0.0.1", 6379);
    //在在线用户中查找看有没有当前用户，如果有登录失败，否则继续判断
    redisReply *r = Redis::hsetexist(c, "Onlineuser", u.getNumber());
    if (r == NULL)
    {
        printf("Execut getValue failure\n");
        redisFree(c);
        close(clnt_sock); //登录失败，关闭套间字
        return false;
    }
    if (r->integer == 1)
    {
        ssock::SendMsg(clnt_sock, "has online", strlen("has online") + 1);
        freeReplyObject(r);
        redisFree(c);
        close(clnt_sock); //登录失败，关闭套间字
        return false;
    }
    freeReplyObject(r);

    //查找与Number同名的哈希表，在map中查找密码，如果秘密相同返回该用户所有信息，
    //密码不同返回No
    r = Redis::hgethash(c, "account", u.getNumber());

    if (r == NULL)
    {
        printf("Execut getValue failure\n");
        redisFree(c);
        close(clnt_sock); //登录失败，关闭套间字
        return false;
    }
    if (r->type != REDIS_REPLY_STRING)
    {
        printf("Execut getValue failure\n");
        freeReplyObject(r);
        redisFree(c);
        ssock::SendMsg(clnt_sock, "No", 2);
        close(clnt_sock); //登录失败，关闭套间字
        return false;
    }
    json jn2 = json::parse(r->str);
    User u2;
    u2.From_Json(jn2, u2);
    if (u2.getPasswd() == u.getPasswd())
    {
        ssock::SendMsg(clnt_sock, r->str, r->len + 1);
        freeReplyObject(r);

        //将在线用户添加到数据库中，key为在线用户，value为在线用户的套间字和uid
        onlineUser onlineU;
        json jn3;
        onlineU.setUid(u.getNumber());   //在线用户id
        onlineU.setsock(u.getServ_fd()); //在线用户的套间字
        onlineU.setflag(0);              //在线用户状态
        onlineU.To_Json(jn3, onlineU);
        r = Redis::hsetValue(c, "Onlineuser", u.getNumber(), jn3.dump());
        freeReplyObject(r);
        redisFree(c);

        //登录成功后将文件描述符挂上监听红黑树
        ep.events = EPOLLIN | EPOLLET;
        ep.data.fd = clnt_sock;
        int ret = epoll_ctl(efd, EPOLL_CTL_ADD, clnt_sock, &ep);
        if (ret == -1)
        {
            ssock::perr_exit("epoll_ctr error");
        }

        return true;
    }
    else
    {
        ssock::SendMsg(clnt_sock, "No", 2);
        freeReplyObject(r);
        redisFree(c);
        close(u.getServ_fd()); //登录失败关闭套间字
        return false;
    }
}

//注册
void account::reg()
{
    int clnt_sock = u.getServ_fd();
    json jn;
    char buf[BUFSIZ];
    ssock::ReadMsg(clnt_sock, buf, sizeof(buf));
    jn = json::parse(buf);
    u.From_Json(jn, u); //将会修改u中本来的内容，导致u中的内容改变
    u.setServ_fd(clnt_sock);

    redisContext *c = Redis::RedisConnect("127.0.0.1", 6379);

    redisReply *r = Redis::hsetexist(c, "account", u.getNumber());
    if (r == NULL)
    {
        printf("Execut getValue failure\n");
        redisFree(c);
        close(clnt_sock);
        return;
    }
    if (r->integer == 0)
    {
        printf("该账号数据库中没有\n");
        freeReplyObject(r);
        r = Redis::hsetValue(c, "account", u.getNumber(), jn.dump());
        freeReplyObject(r);
        ssock::SendMsg(clnt_sock, "Yes", 3);
    }
    else
    {
        freeReplyObject(r);
        ssock::SendMsg(clnt_sock, "No", 2);
    }
    redisFree(c);
    close(clnt_sock);
}

//找回密码
void account::retrieve()
{
    int clnt_sock = u.getServ_fd();
    json jn;
    char buf[BUFSIZ];
    ssock::ReadMsg(clnt_sock, buf, sizeof(buf));
    jn = json::parse(buf);
    u.From_Json(jn, u);
    u.setServ_fd(clnt_sock); //将会修改u中本来的内容，导致U中的文件描述符被改变

    //打开数据库
    redisContext *c = Redis::RedisConnect("127.0.0.1", 6379);
    //查找与Number同名的哈希表，在map中查找密码，如果秘密相同返回该用户所有信息，
    //密码不同返回No

    redisReply *r = Redis::hgethash(c, "account", u.getNumber());
    if (r == NULL)
    {
        printf("Execut getValue failure\n");
        redisFree(c);
        close(clnt_sock);
        return;
    }
    if (r->type != REDIS_REPLY_STRING)
    {
        printf("Execut getValue failure\n");
        freeReplyObject(r);
        redisFree(c);
        ssock::SendMsg(clnt_sock, "No", 2);
        close(clnt_sock);
        return;
    }
    json jn2 = json::parse(r->str);
    User u2;
    u2.From_Json(jn2, u2);

    if ((u2.getKey() == u.getKey()) && (u2.getNumber() == u.getNumber()))
    {
        ssock::SendMsg(clnt_sock, r->str, r->len + 1);
    }
    else
    {
        ssock::SendMsg(clnt_sock, "No", 2);
    }
    close(u.getServ_fd());

    freeReplyObject(r);
    redisFree(c);
    close(clnt_sock);
}

void account::setEfd(const int &e)
{
    efd = e;
}

//获得u这个对象
User &account::getUser()
{
    return u;
}

/*
void account::setfriendfd(const string &s, const int &f) //设置用户名和fd
{
    friendfd.insert(pair<string, int>(s, f));
}
map<string, int> &account::getfriendfd() //获取用户名和fd
{
    return friendfd;
}

*/

#endif