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
    void To_User(const json &jn, User &user)
    {
        jn["name"].get_to(user.name);
        jn.at("passwd").get_to(user.passwd);
        key = jn["key"].get<string>();
        jn["uid"].get_to(user.uid);
        jn["flag"].get_to(user.flag);
    }
    void To_Json(json &jn, const User &user)
    {
        jn = json{
            {"name", user.name},
            {"passwd", user.passwd},
            {"key", user.key},
            {"uid", user.uid},
            {"flag", user.flag}};
    }
    void setName(string &s)
    {
        name = s;
    }
    void setPasswd(string &p)
    {
        passwd = p;
    }
    void setKey(string &k)
    {
        key = k;
    }
    void setFlag(int &f)
    {
        flag = f;
    }

private:
    string name = "";   //账号
    string passwd = ""; //密码
    string key = "";    //密匙
    int uid = 0;        // uid唯一表示一个账号
    int flag = 0;       //操作，1登陆，2注册，3找回密码，4退出
};

#endif
