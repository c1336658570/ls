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
             << "操作" << flag << endl;
    }

private:
    int clnt_fd;   //客户端套间字
    string uid;    // uid唯一表示一个账号
    string name;   //名字
    string passwd; //密码
    string key;    //密匙
    int flag;      //操作，1登陆，2注册，3找回密码，4退出
};

//私聊类
class privateChat
{
    string uid;       //账号
    string name;      //姓名
    int flag;         //操作
    string friendUid; //私聊对象的uid
    string message;   //聊天信息
};

//群聊类
class groupChat
{
    int flag; //操作
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
