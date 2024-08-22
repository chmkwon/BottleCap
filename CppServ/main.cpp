#include "Bottle.h"

#define PORT_NUM 9934

std::vector<ClientInfo*> clnt_socks;      // 클라이언트 소켓 관리 벡터
pthread_mutex_t mutx;                     // 뮤텍스 선언
                                                   
void * handle_clnt(void * arg);

int main()
{
    int serv_sock;
    struct sockaddr_in serv_adr;
    pthread_mutex_init(&mutx, nullptr);          // 뮤텍스 준비

    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    if (serv_sock == -1)
    // 소켓 함수의 반환값이 -1이면 에러 발생 상황
    {
        error_handling("서버 소켓 생성 실패");
    }

    memset(&serv_adr, 0, sizeof(serv_adr));

    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = INADDR_ANY;
    serv_adr.sin_port = htons(PORT_NUM);

    if (bind(serv_sock, (struct sockaddr *)&serv_adr, sizeof(serv_adr)) < 0)
    {
        error_handling("서버 소켓 바인딩 실패");
    }

    if (listen(serv_sock, 5) == -1)
    {
        error_handling("서버 소켓 리슨 실패");
    }
    
    /** accept 시작 **/    
    while (1)
    {
        int clnt_sock;
        struct sockaddr_in clnt_adr;
        socklen_t clnt_adr_sz = sizeof(clnt_adr_sz);

        clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_adr, &clnt_adr_sz);
        
        if (clnt_sock == -1) 
        {
            std::cerr << "클라이언트 연결 수락 실패";
            close(clnt_sock);
        }

        pthread_mutex_lock(&mutx);

        ClientInfo* clientData = new ClientInfo;
		clientData->clnt_sock = clnt_sock;
		clientData->clnt_adr = clnt_adr;
        clnt_socks.push_back(clientData);

        pthread_mutex_unlock(&mutx);
 
        pthread_t clnt_thread;
        pthread_create(&clnt_thread, nullptr, handle_clnt, (void*)clientData);
        pthread_detach(clnt_thread);
    }

    close(serv_sock);
    return 0;
}

void * handle_clnt(void * arg)
{
    ClientInfo* clientData = static_cast<ClientInfo*>(arg);
    int clnt_sock = clientData->clnt_sock;
	sockaddr_in clnt_adr = clientData->clnt_adr;
    Handler handler(clnt_sock);
    try
    {
        handler.WaitProtocol();
    }
    catch(const std::exception& e)
    {
        // 문제 확인
        std::cerr << e.what() << '\n';
    }
    
    pthread_mutex_lock(&mutx);                    
    for (int i = 0; i < clnt_socks.size(); i++)
    {
		if (clnt_socks[i]->clnt_sock == clnt_sock)
        {
            while (i++ < clnt_socks.size() - 1)
            {
                clnt_socks[i] = clnt_socks[i + 1];
            }
            break;
		}
	}
    pthread_mutex_unlock(&mutx);

    std::cout << clnt_sock << "접속종료\n";
    delete clientData;
    close(clnt_sock);
    return nullptr;
}