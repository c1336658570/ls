#include "Msg.h"
#include "NetworkTest.grpc.pb.h"
#include "NetworkTest.pb.h"
#include <algorithm>
#include <arpa/inet.h>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <grpc/grpc.h>
#include <grpcpp/completion_queue.h>
#include <grpcpp/security/server_credentials.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>
#include <grpcpp/support/status.h>
#include <grpcpp/support/status_code_enum.h>
#include <memory>
#include <mutex>
#include <string.h>
#include <string>
#include <sys/socket.h>
#include <thread>
#include <unordered_map>
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
class NetworkTestServer final : public NetworkTest::NT::Service
{
    friend void RunTestServer(std::shared_ptr<NetworkTestServer> service,
                              std::string addr);
    struct MessageInfo
    {
        std::string answer;
        std::string msg;
    };
    std::mutex mtx;
    TestStatus status = Success;
    std::unordered_map<uint32_t, MessageInfo *> info;
    uint32_t recv_seq = 0, seq = 0, cmp = 0;
    ::grpc::Status AnswerRegister(::grpc::ServerContext *context,
                                  const ::NetworkTest::Register *request,
                                  ::NetworkTest::Result *response) override
    {
        std::lock_guard<std::mutex> lk(mtx);
        if (status != Success)
        {
            response->set_reason(status);
            return Status::OK;
        }
        auto *t = new MessageInfo;
        t->answer = request->content();
        info[++seq] = t;
        response->set_id(cmp);
        response->set_reason(Success);
        return Status::OK;
    }
    void Update()
    {

        if (status != Success)
            return;

        auto avaliableMaxResult = std::min(recv_seq, seq);

        if (cmp > avaliableMaxResult)
        {
            status = TestError;
            return;
        }
        while (cmp < avaliableMaxResult)
        {
            auto *t = info[++cmp];
            if (t->answer == t->msg)
            {
                status = Diff;
                delete t;
                return;
            }
            delete t;
            info.erase(cmp);
        }
    }

    ::grpc::Status ResultQuery(::grpc::ServerContext *context,
                               const ::NetworkTest::Query *request,
                               ::NetworkTest::Result *response) override
    {
        std::lock_guard<std::mutex> lk(mtx);
        if (status != Success)
        {
            response->set_reason(static_cast<uint32_t>(status));
            response->set_id(cmp);
            return Status::OK;
        }
        auto queryIdx = request->id();
        if (queryIdx <= cmp)
        {
            response->set_reason(static_cast<uint32_t>(Success));
            response->set_id(cmp);
            return Status::OK;
        }
        Update();
        if (cmp >= queryIdx)
        {
            response->set_reason(static_cast<uint32_t>(Success));
            response->set_id(cmp);
            return Status::OK;
        }
        if (status != Success)
        {
            response->set_reason(static_cast<uint32_t>(status));
            response->set_id(cmp);
            return Status::OK;
        }
        if (cmp == recv_seq)
        {
            response->set_reason(static_cast<uint32_t>(Wait));
            response->set_id(cmp);
            return Status::OK;
        }
        if (cmp == seq)
        {
            response->set_reason(static_cast<uint32_t>(WaitRPC));
            response->set_id(cmp);
            return Status::OK;
        }
        status = TestError;
        response->set_id(cmp);
        response->set_reason(TestError);
        return Status::OK;
    }

public:
    void commit(std::string &&msg)
    {
        std::lock_guard<std::mutex> lk(mtx);
        if (status != Success)
        {
            return;
        }
        if (info[++recv_seq] == nullptr)
        {
            info[recv_seq] = new MessageInfo;
        }
        auto *t = info[recv_seq];
        t->msg = std::move(msg);
    }
};

void RunTestServer(std::shared_ptr<NetworkTestServer> service,
                   std::string addr)
{
    ServerBuilder builder;
    builder.AddListeningPort(addr, grpc::InsecureServerCredentials());
    builder.RegisterService(service.get());
    std::unique_ptr<Server> server(builder.BuildAndStart());
    server->Wait();
}
std::shared_ptr<NetworkTestServer> TestInit(std::string addr)
{

    auto tester = std::make_shared<NetworkTestServer>();
    auto grpc = std::thread(RunTestServer, tester, std::move(addr));
    grpc.detach();
    return tester;
}
class mess
{
public:
    int partid;
    int len;
};

#include <sys/epoll.h>
#define SERV_PORT 9999
#define OPEN_MAX 5000
#define MAXLINE 8192

void perr_exit(const char *s);
int Accept(int fd, struct sockaddr *sa, socklen_t *salenptr);
int Bind(int fd, const struct sockaddr *sa, socklen_t salen);
int Connect(int fd, const struct sockaddr *sa, socklen_t salen);
int Listen(int fd, int backlog);
int Socket(int family, int type, int protocol);
ssize_t Read(int fd, void *ptr, size_t nbytes);
ssize_t Write(int fd, const void *ptr, size_t nbytes);
int Close(int fd);
ssize_t Readn(int fd, void *vptr, size_t n);
ssize_t Writen(int fd, const void *vptr, size_t n);

int main()
{
    // Server 端的监听地址
    auto test = TestInit("0.0.0.0:1234");

    int serv_sock, clnt_sock, sock;
    struct sockaddr_in serv_addr, clnt_addr;
    socklen_t clnt_addr_len;
    int i, n, num = 0;
    ssize_t nready, efd, ret;
    char str[INET_ADDRSTRLEN];

    serv_sock = Socket(AF_INET, SOCK_STREAM, 0);

    int opt = 1;
    setsockopt(serv_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(SERV_PORT);
    Bind(serv_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    Listen(serv_sock, 128);

    efd = epoll_create(OPEN_MAX);
    if (efd == -1)
    {
        perr_exit("epoll_create error");
    }

    struct epoll_event tep, ep[OPEN_MAX];

    tep.events = EPOLLIN;
    tep.data.fd = serv_sock;
    ret = epoll_ctl(efd, EPOLL_CTL_ADD, serv_sock, &tep);
    if (ret == -1)
    {
        perr_exit("epoll_ctr error");
    }

    while (1)
    {
        //-1表永久阻塞
        nready = epoll_wait(efd, ep, OPEN_MAX, -1);
        if (nready == -1)
        {
            perr_exit("epoll_wait error");
        }

        for (i = 0; i < nready; ++i)
        {
            if (!(ep[i].events & EPOLLIN))
            {
                continue;
            }

            if (ep[i].data.fd == serv_sock)
            {
                clnt_addr_len = sizeof(clnt_addr);
                clnt_sock = Accept(serv_sock, (struct sockaddr *)&clnt_addr, &clnt_addr_len);
                printf("accept ip %s, port %d\n", inet_ntop(AF_INET, &clnt_addr.sin_addr, str, INET_ADDRSTRLEN),
                       ntohs(clnt_addr.sin_port));

                tep.events = EPOLLIN;
                tep.data.fd = clnt_sock;
                ret = epoll_ctl(efd, EPOLL_CTL_ADD, clnt_sock, &tep);
                if (ret == -1)
                {
                    perr_exit("epoll_ctl error");
                }
            }
            else
            {
                sock = ep[i].data.fd;
                int len;
                n = Readn(sock, &len, sizeof(len));
                if (n == 0)
                {
                    ret = epoll_ctl(efd, EPOLL_CTL_DEL, sock, NULL);
                    if (ret == -1)
                    {
                        perr_exit("epoll_ctr error");
                    }
                    close(sock);
                    printf("client%d closed connection\n", sock);
                    continue;
                }

                len = ntohl(len);

                printf("len = %d\n", len);
                char *buf = (char *)malloc(len);
                n = Readn(sock, buf, len);
                Writen(STDOUT_FILENO, buf, len);
                Writen(STDOUT_FILENO, "\n", 1);

                if (n == 0)
                {
                    ret = epoll_ctl(efd, EPOLL_CTL_DEL, sock, NULL);
                    if (ret == -1)
                    {
                        perr_exit("epoll_ctr error");
                    }
                    close(sock);
                    printf("client%d closed connection\n", sock);
                }
                else if (n < 0)
                {
                    perror("read n < 0 error: ");
                    ret = epoll_ctl(efd, EPOLL_CTL_DEL, sock, NULL);
                    Close(sock);
                }
                else
                {
                    std::string str(buf, len);
                    test->commit(std::move(str));
                }
                free(buf);
            }
        }
    }

    close(serv_sock);
    close(efd);
    // Put your code Here!
}

void perr_exit(const char *s)
{
    perror(s);
    exit(-1);
}

int Accept(int fd, struct sockaddr *sa, socklen_t *salenptr)
{
    int n;

again:
    if ((n = accept(fd, sa, salenptr)) < 0)
    {
        if ((errno == ECONNABORTED) || (errno == EINTR))
            goto again;
        else
            perr_exit("accept error");
    }
    return n;
}

int Bind(int fd, const struct sockaddr *sa, socklen_t salen)
{
    int n;

    if ((n = bind(fd, sa, salen)) < 0)
        perr_exit("bind error");

    return n;
}

int Connect(int fd, const struct sockaddr *sa, socklen_t salen)
{
    int n;

    if ((n = connect(fd, sa, salen)) < 0)
        perr_exit("connect error");

    return n;
}

int Listen(int fd, int backlog)
{
    int n;

    if ((n = listen(fd, backlog)) < 0)
        perr_exit("listen error");

    return n;
}

int Socket(int family, int type, int protocol)
{
    int n;

    if ((n = socket(family, type, protocol)) < 0)
        perr_exit("socket error");

    return n;
}

ssize_t Read(int fd, void *ptr, size_t nbytes)
{
    ssize_t n;

again:
    if ((n = read(fd, ptr, nbytes)) == -1)
    {
        if (errno == EINTR)
            goto again;
        else
            return -1;
    }
    return n;
}

ssize_t Write(int fd, const void *ptr, size_t nbytes)
{
    ssize_t n;

again:
    if ((n = write(fd, ptr, nbytes)) == -1)
    {
        if (errno == EINTR)
            goto again;
        else
            return -1;
    }
    return n;
}

int Close(int fd)
{
    int n;
    if ((n = close(fd)) == -1)
        perr_exit("close error");

    return n;
}

ssize_t
Readn(int fd, void *buffer, size_t n)
{
    ssize_t numRead; /* # of bytes fetched by last read() */
    size_t totRead;  /* Total # of bytes read so far */
    char *buf;

    buf = (char *)buffer; /* No pointer arithmetic on "void *" */
    for (totRead = 0; totRead < n;)
    {
        numRead = read(fd, buf, n - totRead);

        if (numRead == 0)   /* EOF */
            return totRead; /* May be 0 if this is first read() */
        if (numRead == -1)
        {
            if (errno == EINTR || errno == EAGAIN)
                continue; /* Interrupted --> restart read() */
            else
                return -1; /* Some other error */
        }
        totRead += numRead;
        buf += numRead;
    }
    return totRead; /* Must be 'n' bytes if we get here */
}

ssize_t
Writen(int fd, const void *buffer, size_t n)
{
    ssize_t numWritten; /* # of bytes written by last write() */
    size_t totWritten;  /* Total # of bytes written so far */
    const char *buf;

    buf = (char *)buffer; /* No pointer arithmetic on "void *" */
    for (totWritten = 0; totWritten < n;)
    {
        numWritten = write(fd, buf, n - totWritten);

        if (numWritten <= 0)
        {
            if (numWritten == -1 && errno == EINTR)
                continue; /* Interrupted --> restart write() */
            else
                return -1; /* Some other error */
        }
        totWritten += numWritten;
        buf += numWritten;
    }
    return totWritten; /* Must be 'n' bytes if we get here */
}
