#ifndef MYSQLCON
#define MYSQLCON
#include <mysql/mysql.h>
#include <iostream>
#include <chrono>
using namespace std::chrono;
using std::string;
class Mysqlcon
{
private:
    /* data */
    MYSQL* m_mysql;
    MYSQL_RES* m_result;
    MYSQL_ROW m_row;
    steady_clock::time_point m_aliveTime;
    void freeResult();
public:
    /* 初始化数据库连接 */
    Mysqlcon();
    ~Mysqlcon();
    /* 连接数据库 */
    bool connect(string ip, string user, string password, string database, unsigned int port=3306);
    /* 更新数据库 insert/delete/update */
    bool update(string sql);
    /* 查询数据库 */
    bool query(string sql);
    /* 弹出查询记录 */
    bool getRecord();
    /* 获取记录指定字段的值 */
    string getValue(int index);
    /* */
    bool transaction();
    /* 事务提交 */
    bool commit();
    /* 事务回滚 */
    bool rollback();
    /* 刷性空闲时间点 */
    void refreshAliveTime();
    /* 计算空闲时长 */
    long long getIdleTime();
};
#endif