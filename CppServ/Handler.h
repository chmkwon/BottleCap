#ifndef Handler_h
#define Handler_h
#include "Bottle.h"
#include "Protocol.h"
#include "PyClnt.h"

class Handler
{
public:
    Handler(int sock);
    ~Handler();
    void WaitProtocol();
private:
    bool checkResult();
    bool chartData();
    int mSock;
    DB mDB;
};


#endif