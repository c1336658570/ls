#ifndef JJSON_H
#define JJSON_H
#include <iostream>
#include <string>
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
        key = jn["key"].get<string>();
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
            {"clnt_fd", user.clnt_fd}};
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
    int clnt_fd;      //客户端套间字
    string uid;       // uid唯一表示一个账号
    string name;      //名字
    string passwd;    //密码
    string key;       //密匙
    int flag;         //操作，1登陆，2注册，3找回密码，4退出
    string friends;   //好友列表索引，通过uid_1查询数据库
    string friendUid; //要发送消息的好友的uid
    string shield;    //屏蔽好友列表索引，通过uid_2查询数据库
    string record;    //聊天记录列表索引，通过uid_3查询数据库，该数据库保存的是与该好友聊天记录相关的数据表
};

#endif
