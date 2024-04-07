#include <iostream>
#include <unistd.h>
#include <chrono>
#include <thread>
#include "mysqlcon/mysqlcon.h"
#include "mysqlcon/mysqlconpool.h"
using namespace std;
void insert1(int count){
    for(int i = 0; i < count; i ++ ){
        Mysqlcon con;
        if(!con.connect("127.0.0.1", "root", "123456", "reservation_system")){
            cout << "数据库连接失败！" << endl;
            return ;
        }
        // cout << "0000" << endl;
        string sql = "insert into administrator values('xujiahao', '136447')";
        con.update(sql);
    }
}
void insert2(int count, MysqlconPool *connectionpool){
    for(int i = 0; i < count; i ++ ){
        shared_ptr<Mysqlcon> con = connectionpool->getConnection();
        string sql = "insert into administrator values('xujiahao', '136447')";
        con->update(sql);
    }
}
void clear_table(){
    Mysqlcon con;
    con.connect("127.0.0.1", "root", "123456", "reservation_system");
    string sql = "select count(*) from administrator";
    con.query(sql);
    con.getRecord();
    cout << "delete " << con.getValue(0) << "records" << endl;
    sql = "delete from administrator";
    con.update(sql);
}
void signalThread(int count){
    {
        // 无数据库连接池(单线程)耗时:33479ms
        steady_clock::time_point begin = steady_clock::now();
        insert1(count);
        steady_clock::time_point end = steady_clock::now();
        cout << "无数据库连接池(单线程)耗时:" << (end - begin).count() / 1000000 << "ms" << endl;
    }

    clear_table();

    {
        // 有数据库连接池(单线程)耗时:12689ms
        MysqlconPool *connectionpool = MysqlconPool::getConnectionPool();
        steady_clock::time_point begin = steady_clock::now();
        insert2(count, connectionpool);
        steady_clock::time_point end = steady_clock::now();
        cout << "有数据库连接池(单线程)耗时:" << (end - begin).count() / 1000000 << "ms" << endl;;
    }
    clear_table();

}
void multiThread(){
    cout << "+++++++++++++++++++++++++++++" << endl;
    {
        steady_clock::time_point begin = steady_clock::now();
        thread tp1(insert1, 1000);
        thread tp2(insert1, 1000);
        thread tp3(insert1, 1000);
        thread tp4(insert1, 1000);
        thread tp5(insert1, 1000);
        tp1.join();
        tp2.join();
        tp3.join();
        tp4.join();
        tp5.join();
        steady_clock::time_point end = steady_clock::now();
        cout << "无数据库连接池(多线程)耗时:" << (end - begin).count() / 1000000 << "ms" << endl;
    }
    clear_table();
    

    cout << "-----------------------------" << endl;
    {
        MysqlconPool *connectionpool = MysqlconPool::getConnectionPool();
        steady_clock::time_point begin = steady_clock::now();
        thread tp1(insert2, 1000, connectionpool);
        thread tp2(insert2, 1000, connectionpool);
        thread tp3(insert2, 1000, connectionpool);
        thread tp4(insert2, 1000, connectionpool);
        thread tp5(insert2, 1000, connectionpool);
        tp1.join();
        tp2.join();
        tp3.join();
        tp4.join();
        tp5.join();
        steady_clock::time_point end = steady_clock::now();
        cout << "有数据库连接池(多线程)耗时:" << (end - begin).count() / 1000000 << "ms" << endl;
    }
    clear_table();
    cout << "+++++++++++++++++++++++++++++" << endl;

}
int main(){
    signalThread(5000);
    // multiThread();
    return 0;
}