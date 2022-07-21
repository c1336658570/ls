#include "jjson.hpp"

int main(void)
{
    json jn = json{
        {"name", "张三"},
        {"passwd", "123"},
        {"key", "456"},
        {"uid", "555"},
        {"flag", 1},
        {"friends", "asdasd"},
        {"friendUid", "asdasd"},
        {"shield", "asdasd"},
        {"record", "asdasd"}};
    User u;
    u.From_Json(jn, u);
    u.print();
    json jn2;
    u.To_Json(jn2, u);
    User u2;
    u.From_Json(jn2, u2);
    cout << endl;
    u2.print();
    string jj = jn2.dump();
    cout << jj;
    json jn3 = json::parse(jj.c_str());
    User u3;
    u3.From_Json(jn3, u3);
    cout << endl;
    u3.print();
}