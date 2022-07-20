#ifndef JJSON_H
#define JJSON_H
#include <iostream>
#include <string>
#include <list>
#include <vector>
#include <map>
#include <nlohmann/json.hpp>
using namespace std;
using namespace nlohmann;

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
        friends = "";
        friendUid = "";
        shield = "";
        record = "";
        clnt_fd = 0;
    }

    void From_Json(const json &jn, User &user)
    {
        jn["name"].get_to(user.name);
        jn.at("passwd").get_to(user.passwd);
        // key = jn["key"].get<string>();
        jn["key"].get_to(user.key);
        jn["uid"].get_to(user.uid);
        jn["flag"].get_to(user.flag);
        jn["friends"].get_to(user.friends);
        jn["friendUid"].get_to(user.friendUid);
        jn["shield"].get_to(user.shield);
        jn["record"].get_to(user.record);
        jn["clnt_fd"].get_to(user.clnt_fd);
    }
    void To_Json(json &jn, const User &user)
    {
        jn = json{
            {"name", user.name},
            {"passwd", user.passwd},
            {"key", user.key},
            {"uid", user.uid},
            {"flag", user.flag},
            {"friends", user.friends},
            {"friendUid", user.friendUid},
            {"shield", user.shield},
            {"record", user.record},
            {"clnt_fd", user.clnt_fd},
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
    void setFriends(const string &f)
    {
        friends = f;
    }
    string getFriends()
    {
        return friends;
    }
    void setfriendUid(const string &fu)
    {
        friendUid = fu;
    }
    string getfriendUid()
    {
        return friendUid;
    }
    void setShield(const string &s)
    {
        shield = s;
    }
    string getShield()
    {
        return shield;
    }
    void setRecord(const string &s)
    {
        record = s;
    }
    string getRecord()
    {
        return record;
    }
    void setClnt_fd(const int &c)
    {
        clnt_fd = c;
    }
    int getClnt_fd()
    {
        return clnt_fd;
    }
    void print()
    {
        cout << "账号：" << uid << endl
             << "昵称：" << name << endl
             << "密码" << passwd << endl
             << "密匙" << key << endl
             << "操作" << flag << endl
             << "好友列表索引" << friends << endl
             << "要发送消息的好友的uid" << friendUid << endl
             << "屏蔽好友列表索引" << shield << endl
             << "聊天记录列表索引" << record << endl
             << "套间字" << clnt_fd << endl;
    }

private:
    int clnt_fd;             //客户端套间字
    string uid;              // uid唯一表示一个账号
    string name;             //名字
    string passwd;           //密码
    string key;              //密匙
    int flag;                //操作，1登陆，2注册，3找回密码，4退出
    string friends;          //好友列表索引，通过uid_1查询数据库
    vector<string> friendd;  //存放好友uid的容器
    string friendUid;        //私聊对象，好友uid或者群聊uid
    string message;          //聊天信息，包括各种操作（例如，创建群、解散群或加入群时需要发送的群id等）
    string shield;           //屏蔽好友列表索引，通过uid_2查询数据库
    string record;           //聊天历史记录列表索引，通过uid_3查询数据库，该数据库保存的是和谁聊天，通过uid_3+保存的对方uid_3查找数据库，list+list
    string groups;           //群索引，保存用户加入的群，uid_4，用哈希实现，群id+权限
    map<string, int> groupp; //存放加入的群组的id+权限，0是普通用户，1是管理员，2是群主。
};

struct client //创建结构体
{
    int flag;          //功能标志位 -1:退出群聊 0:通知所有在线用户 1:私聊 2:群聊 3.发送文件 4.修改密码 5.管理员权限操作
    int root;          //权限标志位 -1:首次进入聊天室 0:普通用户 1:管理员 2;VIP用户
    int forbit;        //管理员权限 1:禁言 2:解除禁言 3:踢人
    char name[50];     //账号名
    char password[20]; //密码
    char msg[500];     //聊天信息
    char to_name[50];  //私聊对象
    struct client *next;
};

//群通过map实现，一个哈希集合，里面保存用户uid和用户权限。加入群时需要给每个管理员发送消息。

#endif
