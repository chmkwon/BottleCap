#include "PyClnt.h"

PyClnt::PyClnt(const char *ip, int port)
    : mIp(ip), mPort(port), mSock(-1) {}

PyClnt::~PyClnt()
{
    Disconnect();
}

void PyClnt::Connect()
{
    if ((mSock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        std::cout << "소켓 생성 실패";
    }

    mAddr.sin_family = AF_INET;
    mAddr.sin_port = htons(mPort);
    if (inet_pton(AF_INET, mIp, &mAddr.sin_addr) <= 0)
    {
        std::cout << "유효하지 않은 주소";
    }

    if (::connect(mSock, (struct sockaddr *)&mAddr, sizeof(mAddr)) < 0)
    {
        std::cout << "연결 실패";
    }
}

std::string PyClnt::RecvMessage()
{
    int bufferSize = 1024;
    char buffer[bufferSize];
    std::memset(buffer, 0, bufferSize);
    int bytesRead = recv(mSock, buffer, bufferSize, 0);
    if (bytesRead < 0)
    {
        std::cerr << "메세지 수신 오류" << std::endl;
        return "";
    }
    else
    {
        std::cout << "받은 메세지: " << buffer << std::endl;
        return buffer;
    }
}

cv::Mat PyClnt::RecvImage()
{
    int totalBytes;
    recv(mSock, &totalBytes, sizeof(int), 0);
    std::cout << "받은 이미지 크기: " << totalBytes << std::endl;
    std::vector<uchar> imageData(totalBytes);
    int recvBytes = 0;
    do
    {
        int nowBytes = recv(mSock, imageData.data() + recvBytes, totalBytes - recvBytes, 0);
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

void PyClnt::SendProtocol(u_char p)
{
    int bytesSend = send(mSock, &p, sizeof(p), 0);
    if (bytesSend < 0)
    {
        std::cerr << "프로토콜 송신 오류\n";
    }
}

void PyClnt::SendMessage(const std::string& msg)
{
    int length = msg.length();
    send(mSock, &length, sizeof(length), 0);
    int bytesSent = send(mSock, msg.data(), msg.length(), 0);
    if (bytesSent < 0)
    {
        std::cerr << "메시지 송신 오류" << std::endl;
    }
    else
    {
        std::cout << "보낸 메시지: " << msg << std::endl;
    }
}

void PyClnt::SendInt(int num)
{
    int bytesSend = send(mSock, &num, sizeof(num), 0);
    if (bytesSend < 0)
    {
        std::cerr << "넘버 송신 오류\n";
    }
}

void PyClnt::SendImage(const std::string& ext, const cv::Mat& img)
{
    std::vector<uchar> buffer;
    cv::imencode(ext, img, buffer);
    int totalBytes = buffer.size();
    send(mSock, &totalBytes, sizeof(int), 0);
    std::cout << "보낸 바이트: " << totalBytes << std::endl;
    size_t sentBytes = 0;
    while (sentBytes < buffer.size())
    {
        ssize_t nowBytes = send(mSock, buffer.data() + sentBytes, buffer.size() - sentBytes, 0);
        if (nowBytes < 0)
        {
            std::cerr << "이미지 전송 실패" << std::endl;
            return;
        }
        sentBytes += nowBytes;
    }
}

void PyClnt::Disconnect()
{
    if (mSock != -1)
    {
        close(mSock);
        mSock = -1;
    }
}