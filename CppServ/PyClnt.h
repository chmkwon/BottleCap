#ifndef PyClnt_h
#define PyClnt_h
#include "Bottle.h"

class PyClnt
{
public:
    PyClnt(const char *ip, int port);
    ~PyClnt();

    void Connect();
    std::string RecvMessage();
    cv::Mat RecvImage();
    void SendProtocol(u_char p);
    void SendMessage(const std::string& msg);
    void SendInt(int num);
    void SendImage(const std::string& ext, const cv::Mat& img);
    void Disconnect();

private:
    const char* mIp;
    int mPort;
    int mSock;
    struct sockaddr_in mAddr;
};

#endif 