#include <iostream>
#include <pthread.h>
#include <string>
#include <assert.h>
#include "leveldb/db.h"
#include "leveldb/write_batch.h"
using namespace std;

int main(void)
{
    leveldb::DB *db;
    leveldb::Options options;
    options.create_if_missing = true;
    // 数据库存在时触发一个异常
    options.error_if_exists = true;
    leveldb::Status status = leveldb::DB::Open(options, "./leveldbtest2", &db);
    assert(status.ok()); //检查是否出错

    //打开出错，打印错误信息
    if (!status.ok())
    {
        cerr << status.ToString() << endl;
    }

    //将key1对应的value移动到Key2下
    string value;
    string key1("aaa"), key2("bbb");
    leveldb::Status s = db->Get(leveldb::ReadOptions(), key1, &value); //获取key1对应的value
    if (s.ok())
    {
        s = db->Put(leveldb::WriteOptions(), key2, value); //设置Key2对应的value
    }
    if (s.ok())
    {
        s = db->Delete(leveldb::WriteOptions(), key1); //删除key1对应的value
    }

    //设置key2对应的value和删除key对应的value之间程序可能挂掉，所以需要原子操作
    //上述操作的原子操作，使用WriteBatch，writeBatch不仅可以原子操作，而且可以把多个操作添加到同一个batch一次执行
    s = db->Get(leveldb::ReadOptions(), key1, &value);
    if (s.ok())
    {
        leveldb::WriteBatch batch;
        batch.Delete(key1);
        batch.Put(key2, value);
        s = db->Write(leveldb::WriteOptions(), &batch);
    }

    //默认下，leveldb写操作是异步的，我们可以设置同步写，通常异步写比同步写快1000倍
    //可以为某个特定的写操作打开同步标识：write_options.sync = true，以等到数据真正被记录到持久化存储后再返回
    leveldb::WriteOptions write_options;
    write_options.sync = true;
    string key3 = "ccc";
    db->Put(write_options, key3, value);

    /*
    一个数据库同时只能被一个进程打开。leveldb 会从操作系统获取一把锁来防止多进程同时打开同一个数据库。
    在单个进程中，同一个 leveldb::DB 对象可以被多个并发线程安全地使用，也就是说，
    不同的线程可以在不需要任何外部同步原语的情况下，写入、获取迭代器或者调用Getleveldb实现会确保所需的同步）。
    但是其它对象，比如 Iterator 或者 WriteBatch 需要外部自己提供同步保证，
    如果两个线程共享此类对象，需要使用自己的锁进行互斥访问。具体见对应的头文件。
    */

    //打印数据库中全部的(key, value)对
    leveldb::Iterator *it = db->NewIterator(leveldb::ReadOptions());
    for (it->SeekToFirst(); it->Valid(); it->Next())
    {
        cout << it->key().ToString() << ": " << it->value().ToString() << endl;
    }
    assert(it->status().ok());
    delete it;

    //打印[start, limit]范围内的数据
    for (it->Seek(start); it->Valid() && it->key().ToString() < limit; it->Next())
    {
    }

    //反向遍历
    for (it->SeekToLast(); it->Valid(); it->Prev())
    {
    }

    /*
    it->key()和it->value()调用返回的值是leveldb::Slice类型。
    Slice 是一个简单的数据结构，包含一个长度和一个指向外部字节数组的指针，
    返回一个Slice比返回一个 std::string 更高效，因为不需要隐式地拷贝大量的keys和values。
    另外，leveldb方法不返回\0截止符结尾的C风格字符串，因为leveldb的keys和values允许包含\0字节。
    */
    // C++ 风格的string和C风格的空字符结尾的字符串转换为一个Slice
    leveldb::Slice s1 = "hello";
    std::string str("World");
    leveldb::Slice s2 = str;
    //一个Slice转换回C++风格的字符串
    std::string str = s1.ToString();
    assert(str == std::string("hello"));

    delete db; //关闭数据库
}