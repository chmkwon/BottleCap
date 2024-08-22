#include "Bottle.h"

std::string DELIMITER = ";";

std::string recvMessage(int sock)
{
    int bufferSize = 1024;  
    char buffer[bufferSize];
    std::memset(buffer, 0, bufferSize);  

    int bytesRead = recv(sock, buffer, bufferSize - 1, 0);
    if (bytesRead < 0)
    {
        std::cerr << "메세지 수신 오류" << std::endl;
        return "";
    }
    else
    {
        std::string msg(buffer, bytesRead);
        std::cout << "받은 메세지: " << buffer << std::endl;
        return msg;
    }
}

cv::Mat recvImage(int sock)
{
    int totalBytes;
    recv(sock, &totalBytes, sizeof(int), 0);
    std::cout << "받은 이미지 크기: " << totalBytes << std::endl;
    std::vector<uchar> imageData(totalBytes);

    int recvBytes = 0;
    do
    {
        int nowBytes = recv(sock, imageData.data() + recvBytes, totalBytes - recvBytes, 0);
        recvBytes += nowBytes;
    } while (recvBytes < totalBytes);

    cv::Mat img = cv::imdecode(imageData, cv::IMREAD_COLOR);
    if (img.empty()) 
    {
        std::cout << "이미지 불러오기 실패" << std::endl;
    }
    else
    {
        std::cout << "이미지 불러오기 성공" << std::endl;
    }
    return img;
}

void sendProtocol(int sock, u_char p)
{
    int bytesSend = send(sock, &p, sizeof(p), 0);
    if (bytesSend < 0)
    {
        std::cerr << "프로토콜 송신 오류\n";
    }
}

void sendMessage(int sock, std::string msg)
{
    const char* cstr = msg.c_str();
    int bytesSent = send(sock, cstr, strlen(cstr), 0);
    if (bytesSent < 0)
    {
        std::cerr << "메세지 송신 오류" << std::endl;
    }
    else
    {
        std::cout << "보낸 메세지: " << msg << std::endl;
    }
}

void sendInt(int sock, int num)
{
    int bytesSend = send(sock, &num, sizeof(num), 0);
    if (bytesSend < 0)
    {
        std::cerr << "넘버 송신 오류\n";
    }
}

void joinStr(std::string& base, std::string add)
{
    base += DELIMITER;
    base += add;
}

void joinStr(std::string& base, int add)
{
    base += DELIMITER;
    base += std::to_string(add);
}

bool strToBool(std::string str)
{
    if (str == "True")
    {
        return 1;
    }
    return 0;
}

std::string boolToStr(bool boolean)
{
    if (boolean == true)
    {
        return "True";
    }
    return "False";
}

std::vector<std::string> parseStr(std::string& base)
{
    std::vector<std::string> temp;
    std::stringstream ss(base);
    std::string buf;

    while (std::getline(ss, buf, ';'))
    {
        temp.push_back(buf);
    }
    return temp;
}

void error_handling(const char* message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}