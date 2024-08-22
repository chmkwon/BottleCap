#include "Handler.h"

Handler::Handler(int sock)
    : mSock(sock)
{
    mDB.SetAccount("Tester", "1234");
}

Handler::~Handler() { }

void Handler::WaitProtocol()
{
    bool flag = false;
    do
    {
        u_char protocol;
        int bytesRead = recv(mSock, &protocol, sizeof(protocol), 0);
        if (bytesRead < 0)
        {
            std::cerr << "프로토콜 오류\n";
            return;
        }
        else if (bytesRead == 0)
        {
            return;
        }
       
        switch (protocol)
        {
        case Error_Protocol::CONNECT_ERROR:
            // 연결 종료
            return;
        case Protocol::CHECK_RESULT:
            std::cout << "결과확인\n";
            checkResult();
            return;
        case Protocol::CHAR_DATA:
            std::cout << "차트데이터요청\n";
            chartData();
            return;
        default:
            std::cerr << "프로토콜 타입 오류\n";
            return;
        }
    } while (flag);
}

bool Handler::checkResult()
{

    cv::Mat imgData = recvImage(mSock);
    bool flag = false;
    std::string message;

    if (imgData.empty())
    {
        std::cout << "이미지 수신 실패" << std::endl;
        return false;
    }
    else
    {
        cv::imwrite("recv-image.jpg",imgData);
        // cv::imshow("h", imgData);
        // cv::waitKey(0);
        flag = true;
    }

    if (!flag)
    {
        return false;
    }
    else
    {
        // 파이썬 연결
        PyClnt py("127.0.0.1", 9935);
        py.Connect();
        py.SendProtocol(Protocol::CHECK_RESULT);

        cv::Mat img = imgData;
        if (img.empty())
        {
            std::cout << "이미지를 불러올 수 없습니다." << std::endl;
            return -1;
        }
        py.SendImage(".jpg", img);
        message = py.RecvMessage();
        py.SendProtocol(Protocol::DISCONNECT);
        py.Disconnect();

        // 클라이언트에 전달
        sendMessage(mSock, message);
        std::vector<std::string> inputInfo =  parseStr(message);

        // DB연결
        mDB.Connect();
        bool isYesCap;
        int errorNum;

        if (inputInfo[0] == "YesCap")
        {
            isYesCap = true;
            errorNum = 0;
        }
        else if(inputInfo[0] == "NoCap")
        {
            isYesCap = false;
            errorNum = 1;
        }
        else if(inputInfo[0] == "BrokenCap")
        {
            isYesCap = false;
            errorNum = 2;
        }

        try
        {
            // 인서트 쿼리
            std::unique_ptr<sql::PreparedStatement> insertStmt(
                mDB.mCoon->prepareStatement("CALL INSERT_INSPECTION(?, ?)")
            );

            // 파라미터 설정
            insertStmt->setBoolean(1, isYesCap);
            insertStmt->setInt(2, errorNum);

            // 쿼리 실행
            std::unique_ptr<sql::ResultSet> insertRes(insertStmt->executeQuery());
            std::cout << "쿼리 실행 완료\n";
            if (insertRes->next())
            {
                int result = insertRes->getInt("V_RESULT");

                if (result == 1)
                {
                    // insert성공
                    mDB.Disconnect();
                    return true;
                }
                else if(result == 0)
                {
                    // insert실패
                    mDB.Disconnect();
                    return false;
                }
            }
        }
        catch(const std::exception& e)
        {
            sendProtocol(mSock, Error_Protocol::DB_ERROR);
            std::cerr << "DB 오류 " << e.what() << '\n';
            mDB.Disconnect();
            return false;
        }
        return false;
    }
}

bool Handler::chartData()
{
    std::string message = recvMessage(mSock);
    std::vector<std::string> inputInfo =  parseStr(message);

    // DB연결
    mDB.Connect();

    try
    {
        // 인서트 쿼리
        std::unique_ptr<sql::PreparedStatement> selectStmt(
            mDB.mCoon->prepareStatement("CALL SELECT_CHART_DATA(?, ?, ?)")
        );

        std::string mod = inputInfo[0];
        std::string startDay = inputInfo[1];
        std::string endDay = inputInfo[2];

        // 파라미터 설정
        selectStmt->setString(1, mod);
        selectStmt->setString(2, startDay);
        selectStmt->setString(3, endDay);

        // 쿼리 실행
        std::unique_ptr<sql::ResultSet> selectRes(selectStmt->executeQuery());
        std::cout << "쿼리 실행 완료\n";

        std::vector<std::string> sendData;

        while (selectRes->next())
        {
            std::string tempResult;
            std::string tempIntime;

            tempResult = selectRes->getString(1);
            tempIntime = selectRes->getString(2);

            sendData.push_back(tempResult + ";" + tempIntime);
        }

        mDB.Disconnect();

        sendInt(mSock, sendData.size());

        std::string sendsendString = sendData[0];

        for (int i = 1; i < sendData.size(); i++)
        {
            sendsendString = sendsendString + ";" + sendData[i];
        }

        sendMessage(mSock, sendsendString);

        return true;
    }
    catch(const std::exception& e)
    {
        sendProtocol(mSock, Error_Protocol::DB_ERROR);
        std::cerr << "DB 오류 " << e.what() << '\n';
        mDB.Disconnect();
        return false;
    }
    return false;
}