#ifndef DB_H
#define DB_H
#include <string>
#include <iostream>
#include <mariadb/conncpp.hpp>

class DB
{
public:
    DB();
    ~DB();
    void SetAccount(const std::string& id, const std::string& pw);
    bool Connect();
    void Disconnect();
    std::unique_ptr<sql::Connection> mCoon;
private:
    std::string mID;
    std::string mPW;
};

#endif