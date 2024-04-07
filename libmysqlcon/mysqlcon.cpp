#include "mysqlcon/mysqlcon.h"

Mysqlcon::Mysqlcon(){
    this->m_mysql = mysql_init(nullptr);
    this->m_result = nullptr;
    this->m_row = nullptr;
    mysql_set_character_set(this->m_mysql, "utf8");
}



Mysqlcon::~Mysqlcon(){
    if(this->m_mysql) mysql_close(this->m_mysql);
    this->freeResult();
}

bool Mysqlcon::connect(string ip, string user, string password, string database, unsigned int port){
    this->m_mysql = mysql_real_connect(this->m_mysql, ip.c_str(), user.c_str(), password.c_str(), database.c_str(), port, nullptr, 0);
    if(!this->m_mysql)  return false;
    return true;
}

bool Mysqlcon::update(string sql){
    return !mysql_query(this->m_mysql, sql.c_str());
}


bool Mysqlcon::query(string sql){
    if(mysql_query(this->m_mysql, sql.c_str())) return false;
    if(this->m_result)  this->freeResult();
    this->m_result = mysql_store_result(this->m_mysql);
    return true;
}

bool Mysqlcon::getRecord(){
    if(!this->m_result) return false;
    this->m_row = mysql_fetch_row(this->m_result);
    return this->m_row != nullptr;
}

string Mysqlcon::getValue(int index){
    int length = mysql_num_fields(this->m_result);
    if(index >= length || index < 0) return "";
    char *tmp = this->m_row[index];
    unsigned long field_length = mysql_fetch_lengths(this->m_result)[index];
    return string(tmp, field_length);
}

bool Mysqlcon::transaction(){
    return mysql_autocommit(this->m_mysql, false);
}


bool Mysqlcon::commit(){
    return mysql_commit(this->m_mysql);
}

bool Mysqlcon::rollback(){
    return mysql_rollback(this->m_mysql);
}

void Mysqlcon::freeResult(){
    if(this->m_result)  mysql_free_result(this->m_result);
    this->m_mysql = nullptr;
}

void Mysqlcon::refreshAliveTime(){
    this->m_aliveTime = steady_clock::now();
}

long long Mysqlcon::getIdleTime(){
    nanoseconds nansec = steady_clock::now() - this->m_aliveTime;
    milliseconds milsec = duration_cast<milliseconds>(nansec);
    return milsec.count();
}