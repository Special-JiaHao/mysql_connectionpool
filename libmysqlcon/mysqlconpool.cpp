#include "mysqlcon/mysqlconpool.h"

bool MysqlconPool::init(){
    ifstream ifs("./dbconfig.json");
    Reader rd;
    Value root;
    rd.parse(ifs, root);
    if(root.isObject()){
        this->m_ip = root["ip"].asString();
        this->m_user = root["userName"].asString();
        this->m_password = root["password"].asString();
        this->m_database = root["database"].asString();
        this->m_port = root["port"].asUInt();
        this->m_minConnection = root["minConnection"].asInt();
        this->m_maxConnection = root["maxConnection"].asInt();
        this->m_timeout = root["timeout"].asInt();
        this->m_maxIdleTime = root["maxIdleTime"].asInt();
        /*
        std::cout << "ip : " << this->m_ip << std::endl;
        std::cout << "user : " << this->m_user << std::endl;
        std::cout << "password : " << this->m_password << std::endl;
        std::cout << "m_database : " << this->m_database << std::endl;
        std::cout << "port : " << this->m_port << std::endl;
        std::cout << "minConnection : " << this->m_minConnection << std::endl;
        std::cout << "maxConnection : " << this->m_maxConnection << std::endl;
        std::cout << "timeout : " << this->m_timeout << std::endl;
        std::cout << "maxIdleTime : " << this->m_maxIdleTime << std::endl;
        */
        return true;
    }
    return false;
}

bool MysqlconPool::addConnection(){
    Mysqlcon *con = new Mysqlcon;
    if(!con->connect(this->m_ip, this->m_user, this->m_password, this->m_database, this->m_port)){
        std::cout << "无法连接数据库服务器, 请检查数据库配置dbconfig.json" << std::endl;
        return false;
    }
    con->refreshAliveTime();
    this->qu.push(con);
    return true;
}

MysqlconPool::MysqlconPool(){
    if(!this->init()){
        std::cout << "配置文件加载失败！" << std::endl;
        return ;
    }   
    this->shutdown = false;
    this->blockThreadCount = 0;
    for(int i = 0; i < this->m_minConnection; i ++ ){
        if(!this->addConnection())   break;
    }    


    thread producer(&MysqlconPool::producerConnection, this);
    thread recycler(&MysqlconPool::recycleConnection, this);
    producer.detach();
    recycler.detach();
}

MysqlconPool* MysqlconPool::getConnectionPool(){
    static MysqlconPool pool;
    return &pool;
}

void MysqlconPool::producerConnection(){
    while(!this->shutdown){
        std::unique_lock<mutex> locker(this->m_mutexQu);
        while(!this->shutdown && this->qu.size() >= this->m_minConnection){
            this->blockThreadCount ++ ;
            this->m_cond.wait(locker);
            this->blockThreadCount -- ;
        }  
        if(this->shutdown)  break;
        this->addConnection();
        this->m_cond.notify_all();
    }
    // std::cout << "producer exit" << std::endl;
}

void MysqlconPool::recycleConnection(){
    while(!this->shutdown){
        std::this_thread::sleep_for(milliseconds(500));
        std::lock_guard<mutex> locker(this->m_mutexQu);
        while(!this->shutdown && this->qu.size() > this->m_minConnection){
            Mysqlcon *mysqlcon = this->qu.front();
            if(mysqlcon->getIdleTime() >= this->m_maxIdleTime){
                std::cout << "recycle" << std::endl;
                this->qu.pop();
                delete mysqlcon;
            }else   break;
        }
    }
    // std::cout << "recycler exit" << std::endl;
}

shared_ptr<Mysqlcon> MysqlconPool::getConnection(){
    std::unique_lock<mutex> locker(this->m_mutexQu);
    while(this->qu.empty()){
        if(this->m_cond.wait_for(locker, microseconds(this->m_timeout)) == std::cv_status::timeout) return nullptr;
    }
    shared_ptr<Mysqlcon> mysqlcon(this->qu.front(), [this](Mysqlcon* con){
        std::lock_guard<mutex> locker(this->m_mutexQu);
        con->refreshAliveTime();
        this->qu.push(con);
    });
    this->qu.pop();
    this->m_cond.notify_all();
    return mysqlcon;
}

MysqlconPool::~MysqlconPool(){
    this->shutdown = true;
    for(int i = 0; i < this->blockThreadCount; i ++ )   this->m_cond.notify_all();
    this->blockThreadCount = 0;
    std::this_thread::sleep_for(milliseconds(500));
    while(!this->qu.empty()){
        Mysqlcon* mysqlcon = this->qu.front();
        this->qu.pop();
        delete mysqlcon;
    }
}