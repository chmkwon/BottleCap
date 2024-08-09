
#include "pch.h"
#include "ConnServ.h"
#include "BottleCap.h"

CConnServ::CConnServ()
{
    m_pWnd = NULL;
}

CConnServ::~CConnServ()
{
}

void CConnServ::OnReceive(int nErrorCode)
{
    if (nErrorCode == 0 && m_pWnd != NULL)
    {
        m_pWnd->SendMessage(WM_RECEIVE_MESSAGE, 0, 0);
    }

    CSocket::OnReceive(nErrorCode);
}

void CConnServ::OnClose(int nErrorCode)
{
    if (nErrorCode == 0 && m_pWnd != NULL)
    {
        m_pWnd->SendMessage(WM_SOCKET_CLOSED, 0, 0);
    }

    CSocket::OnClose(nErrorCode);
}