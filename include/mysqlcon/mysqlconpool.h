#ifndef MYSQLCONPOOL
#define MYSQLCONPOOL
#include <queue>
#include <mutex>
#include <condition_variable>
#include <fstream>
#include <thread>
#include "mysqlcon.h"
#include "json/json.h"
using std::queue;
using std::mutex;
using std::condition_variable;
using std::ifstream;
using Json::Reader;
using Json::Value;
using std::thread;
using std::shared_ptr;
class MysqlconPool{
private:
    MysqlconPool();
    string m_ip;
    string m_user;
    string m_password;
    string m_database;
    unsigned int m_port;
    int m_maxConnection;
    int m_minConnection;
    queue<Mysqlcon*> qu;
    int m_timeout;      // 最大超时时间（超时时间内未获得有效连接，则返回nullptr）
    int m_maxIdleTime;  // 最大空闲时长
    mutex m_mutexQu;
    condition_variable m_cond;
    bool shutdown;
    bool init();
    void producerConnection();
    void recycleConnection();
    bool addConnection();
    int blockThreadCount;
public:
    static MysqlconPool* getConnectionPool();
    MysqlconPool(const MysqlconPool& obj) = delete;
    MysqlconPool& operator=(const MysqlconPool& obj) = delete;
    shared_ptr<Mysqlcon> getConnection();
    ~MysqlconPool();
};
#endif