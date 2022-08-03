#ifndef MESSAGE_H
#define MESSAGE_H
#include <iostream>
#include <string>
#include <list>
#include <vector>
#include <map>
#include <nlohmann/json.hpp>
using namespace std;
using namespace nlohmann;

//用户登录类
class User
{
public:
    User()
    {
        name = "";
        passwd = "";
        key = "";
        uid = "";
        flag = 0;
        serv_fd = 0;
    }

    void From_Json(const json &jn, User &user)
    {
        jn["name"].get_to(user.name);
        jn.at("passwd").get_to(user.passwd);
        // key = jn["key"].get<string>();
        jn["key"].get_to(user.key);
        jn["uid"].get_to(user.uid);
        jn["flag"].get_to(user.flag);
        jn["serv_fd"].get_to(user.serv_fd);
    }
    void To_Json(json &jn, const User &user)
    {
        jn = json{
            {"name", user.name},
            {"passwd", user.passwd},
            {"key", user.key},
            {"uid", user.uid},
            {"flag", user.flag},
            {"serv_fd", user.serv_fd},
        };
    }
    void setName(const string &s)
    {
        name = s;
    }
    string getName()
    {
        return name;
    }
    void setNumber(const string &u)
    {
        uid = u;
    }
    string getNumber()
    {
        return uid;
    }
    void setPasswd(const string &p)
    {
        passwd = p;
    }
    string getPasswd()
    {
        return passwd;
    }
    void setKey(const string &k)
    {
        key = k;
    }
    string getKey()
    {
        return key;
    }
    void setFlag(const int &f)
    {
        flag = f;
    }
    int getFlag()
    {
        return flag;
    }
    void setServ_fd(const int &c)
    {
        serv_fd = c;
    }
    int getServ_fd()
    {
        return serv_fd;
    }

    void print()
    {
        cout << "账号：" << uid << endl
             << "昵称：" << name << endl;
    }

private:
    int serv_fd;   //服务器和客户端相连接的套间字套间字
    string uid;    // uid唯一表示一个账号
    string name;   //名字
    string passwd; //密码
    string key;    //密匙
    int flag;      //操作，1登陆，2注册，3找回密码，4退出
};

//聊天类
class privateChat
{
public:
    //好友保存在和发送者同名的map中，其中key为目标uid，value为该好友状态，正常或屏蔽
    privateChat()
    {
        serv_fd = 0;
        uid = "";
        name = "";
        flag = 0;
        friendUid = "";
        message = "";
        timeNow = "";
    }

    void setServ_fd(const int &s)
    {
        serv_fd = s;
    }
    int getServ_fd()
    {
        return serv_fd;
    }

    void setNumber(const string &u)
    {
        uid = u;
    }
    string getNumber()
    {
        return uid;
    }
    void setName(const string &s)
    {
        name = s;
    }
    string getName()
    {
        return name;
    }
    void setFlag(const int &f)
    {
        flag = f;
    }
    int getFlag()
    {
        return flag;
    }
    void setFriendUid(const string &f)
    {
        friendUid = f;
    }
    string getFriendUid()
    {
        return friendUid;
    }
    void setMessage(const string &m)
    {
        message = m;
    }
    string getMessage()
    {
        return message;
    }
    void setTimeNow()
    {
        time_t timeN;
        time(&timeN);
        timeNow = ctime(&timeN);
        while (1)
        {
            auto a = timeNow.find(' ');
            if (a == string::npos)
            {
                break;
            }
            timeNow[a] = '-';
        }
    }
    string getTimeNow()
    {
        return timeNow;
    }

    void
    From_Json(const json &jn, privateChat &p)
    {
        jn["uid"].get_to(p.uid);
        jn["name"].get_to(p.name);
        jn["flag"].get_to(p.flag);
        jn["friendUid"].get_to(p.friendUid);
        jn["message"].get_to(p.message);
        jn["timeNow"].get_to(p.timeNow);
        jn["serv_fd"].get_to(p.serv_fd);
    }
    void To_Json(json &jn, const privateChat &p)
    {
        jn = json{
            {"uid", p.uid},
            {"name", p.name},
            {"flag", p.flag},
            {"friendUid", p.friendUid},
            {"message", p.message},
            {"timeNow", p.timeNow},
            {"serv_fd", p.serv_fd}};
    }

private:
    int serv_fd;      //服务器和客户端相连接的套间字套间字
    string uid;       //账号
    string name;      //姓名
    int flag;         //操作
    string friendUid; //对象的uid
    string message;   //聊天信息
    string timeNow;   //当前时间
};

//朋友信息类
class friends //用来保存好友信息，好友uid，加好友权限
{
public:
    void From_Json(const json &jn, friends &f)
    {
        jn["friendUid"].get_to(f.friendUid);
        jn["flag"].get_to(f.flag);
    }
    void To_Json(json &jn, const friends &f)
    {
        jn = json{
            {"friendUid", f.friendUid},
            {"flag", f.flag}};
    }

    void setfriendUid(const string &f)
    {
        friendUid = f;
    }
    string getfriendUid()
    {
        return friendUid;
    }
    void setflag(const int &f)
    {
        flag = f;
    }
    int getflag()
    {
        return flag;
    }

private:
    string friendUid; // 1、朋友的Uid  2、在线用户的uid
    int flag;         // 1、好友的状态0屏蔽，1正常   2、在线用户的套间字
};

struct file //保存打开的文件描述符和要发送的客户端文件描述符
{
    int filefd;
    int clnt_sock;
    string filename; //文件名
};

class onlineUser //用来保存好友信息，好友uid，加好友权限
{
public:
    void From_Json(const json &jn, onlineUser &u)
    {
        jn["uid"].get_to(u.uid);
        jn["sock"].get_to(u.sock);
        jn["flag"].get_to(u.flag);
    }
    void To_Json(json &jn, const onlineUser &u)
    {
        jn = json{
            {"uid", u.uid},
            {"sock", u.sock},
            {"flag", u.flag}};
    }

    void setUid(const string &f)
    {
        uid = f;
    }
    string getUid()
    {
        return uid;
    }
    void setsock(const int &s)
    {
        sock = s;
    }
    int getsock()
    {
        return sock;
    }
    void setflag(const int &f)
    {
        flag = f;
    }
    int getflag()
    {
        return flag;
    }

private:
    string uid; // 在线用户的Uid
    int sock;   // 套间字
    int flag;   //有没有在聊天
};

//群聊用户信息类
class groups
{
public:
    void From_Json(const json &jn, groups &g)
    {
        jn["group_number"].get_to(g.group_number);
        jn["user_number"].get_to(g.user_number);
        jn["flag"].get_to(g.flag);
    }
    void To_Json(json &jn, const groups &g)
    {
        jn = json{
            {"group_number", g.group_number},
            {"user_number", g.user_number},
            {"flag", g.flag}};
    }
    void setgroup_number(const string &g)
    {
        group_number = g;
    }
    string getgroup_number()
    {
        return group_number;
    }
    void setuser_number(const string &u)
    {
        user_number = u;
    }
    string getuser_number()
    {
        return user_number;
    }
    void setflag(const int &f)
    {
        flag = f;
    }
    int getflag()
    {
        return flag;
    }

private:
    string group_number; //群id
    string user_number;  //用户id
    int flag;            //用户权限0普通用户，1管理员，2群主
};
//群通过哈希实现，一个哈希集合，哈希名为群id，里面保存用户uid和用户权限。加入群时需要给每个管理员发送消息。

// string friends;          //好友哈希索引，通过uid_1查询数据库，哈希中key保存好友uid，value保存好友的状态
// vector<string> friendd;  //存放好友uid的容器
// string friendUid;        //私聊对象，好友uid或者群聊uid
// string message;          //聊天信息，包括各种操作（例如，创建群、解散群或加入群时需要发送的群id等）
// string shield;           //屏蔽好友列表索引，通过uid_2查询数据库
// string record;           //聊天历史记录列表索引，通过uid_3查询数据库，该数据库保存的是和谁聊天，通过uid_3+保存的对方uid_3查找数据库，list+list
// string groups;           //群索引，保存用户加入的群，uid_4，用哈希实现，群id+权限
// map<string, int> groupp; //存放加入的群组的id+权限，0是普通用户，1是管理员，2是群主。

//每个用户有自己加入的群的信息，用哈希保存，通过用户uid_x找到数据库，数据库时一个哈希结构，保存群uid+用户的权限
//群用哈希实现，通过群号作为索引，用哈希保存，一个为用户的uid，一个为用户的权限。
#endif
