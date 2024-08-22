#include "DB.h"

DB::DB(): mCoon(nullptr)
{ }

DB::~DB()
{
    Disconnect();
}

void DB::SetAccount(const std::string& id, const std::string& pw)
{
    mID = id;
    mPW = pw;
}

bool DB::Connect()
{
    try
    {
        sql::Driver* driver = sql::mariadb::get_driver_instance();
        sql::SQLString url = "jdbc:mariadb://10.10.21.110:3306/BOTTLE";
        sql::Properties properties({{"user", mID}, {"password", mPW}});
        mCoon = std::unique_ptr<sql::Connection>(driver->connect(url, properties));
        std::cout << "DB 접속 성공\n";
        return true;
    }
    catch(sql::SQLException& e)
    {
        std::cerr << "DB 접속 실패: " << e.what() << std::endl;
        return false;
    }
}

void DB::Disconnect()
{
    if (mCoon && !mCoon->isClosed())
    {
        mCoon->close();
        std::cout << "DB 접속 해제\n";
    }
}