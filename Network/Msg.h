#ifndef Msg_H
#define Msg_H
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <deque>
#include <mutex>
#include <string>
#include <unistd.h>
class MsgBuf {
    std::deque<std::string> queue;
    std::mutex mtx;

public:
    MsgBuf() = default;
    void push(std::string &&s) {
        std::lock_guard<std::mutex> lk(mtx);
        queue.emplace_back(std::move(s));
    }
    std::string pop() {
        std::lock_guard<std::mutex> lk(mtx);
        // mtx.lock();
        while (queue.empty()) {
            mtx.unlock();
            sleep(1);
            mtx.lock();
        }
        auto t = std::move(queue.front());
        queue.pop_front();
        return t;
    }
};
enum TestStatus : uint32_t {
    Success,
    Wait,
    WaitRPC,
    ErrorLevel,
    TestError,
    Diff,
    Unknow
};
constexpr int PORT = 4001;
#define ServerAddr "192.168.30.170"
#endif
