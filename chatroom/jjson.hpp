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
    }

    void From_Json(const json &jn, User &user)
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
    string getName()
    {
        return name;
    }
    void setNumber(string &u)
    {
        uid = u;
    }
    string getNumber()
    {
        return uid;
    }
    void setPasswd(string &p)
    {
        passwd = p;
    }
    string getPasswd()
    {
        return passwd;
    }
    void setKey(string &k)
    {
        key = k;
    }
    string getKey()
    {
        return key;
    }
    int setFlag(int &f)
    {
        flag = f;
    }
    int getFlag()
    {
        return flag;
    }
    void print()
    {
        cout << "账号：" << uid << endl
             << "昵称：" << name << endl;
    }

private:
    string uid;    // uid唯一表示一个账号
    string name;   //名字
    string passwd; //密码
    string key;    //密匙
    int flag;      //操作，1登陆，2注册，3找回密码，4退出
};

#endif
