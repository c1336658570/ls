#ifndef SERV_H
#define SERV_H

#include <iostream>
#include <cstring>
#include <string>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "ssock.h"
#include "leveldb/db.h"

using namespace std;
using namespace leveldb;

class serv1
{
public:
    //从数据库读数据并与登陆输入的数据比较
    static void login()
    {
        int serv_fd = ssock::Socket();
        ssock::Bind(serv_fd, 9999, "127.0.0.1");
        ssock::Listen(serv_fd, 128);
        ssock::Accept(serv_fd);

        DB *db;
        Options options;
        options.create_if_missing = true;

        //打开
        Status status = leveldb::DB::Open(options, "/tmp/testdb", &db);
        if (!status.ok())
        {
            cerr << status.ToString() << endl;
        }
        Iterator *it = db->NewIterator(ReadOptions());
        for (it->SeekToFirst(); it->Valid(); it->Next())
        {
        }
    }
};

#endif