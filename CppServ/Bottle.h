#ifndef Bottle_h
#define Bottle_h
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <vector>
#include <cstring>
#include <sstream>
#include <map>
#include <opencv2/opencv.hpp>
#include "DB.h"
#include "Handler.h"

struct ClientInfo
{
	int clnt_sock;
	sockaddr_in clnt_adr;
};

std::string recvMessage(int sock);
cv::Mat recvImage(int sock);

void sendProtocol(int sock, u_char p);
void sendInt(int sock, int num);
void sendMessage(int sock, std::string msg);

void joinStr(std::string& base, std::string add);
void joinStr(std::string& base, int add);
std::vector<std::string> parseStr(std::string& base);
void error_handling(const char* message);
bool strToBool(std::string str);
std::string boolToStr(bool boolean);

#endif