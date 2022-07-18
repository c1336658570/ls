/*
    编译g++ -o level levelDB.cc -pthread -Iinclude -lleveldb

    插入、获取和删除一条记录
*/

#include <iostream>
#include <pthread.h>
#include <string>
#include <assert.h>
#include "leveldb/db.h"

using namespace std;

int main(void)
{

    leveldb::DB *db;
    leveldb::Options options;
    options.create_if_missing = true;

    /*
    如果你想在数据库已存在的时候触发一个异常，将下面这行配置加到 leveldb::DB::Open 调用之前：
    options.error_if_exists = true;
    */

    //打开数据库
    leveldb::Status status = leveldb::DB::Open(options, "./leveldbtest", &db);
    assert(status.ok());

    //检查返回值是否Ok，然后打印相关错误信息
    /*
    if (!status.ok())
    {
        cerr << status.ToString() << endl;
    }
    */

    string key = "name";
    string value = "chenqi";

    //写数据库
    status = db->Put(leveldb::WriteOptions(), key, value);
    assert(status.ok());

    //读数据库
    status = db->Get(leveldb::ReadOptions(), key, &value);
    assert(status.ok());

    cout << value << endl;

    //删除数据库
    status = db->Delete(leveldb::WriteOptions(), key);
    assert(status.ok());

    status = db->Get(leveldb::ReadOptions(), key, &value);
    if (!status.ok())
    {
        cerr << key << "    " << status.ToString() << endl;
    }
    else
    {
        cout << key << "===" << value << endl;
    }

    delete db; //关闭数据库

    return 0;
}