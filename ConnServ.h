#pragma once



class CConnServ : public CSocket
{
public:
    CConnServ();
    virtual ~CConnServ();

    virtual void OnReceive(int nErrorCode);
    virtual void OnClose(int nErrorCode);

    CWnd* m_pWnd;
};