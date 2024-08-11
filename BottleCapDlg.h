
// BottleCapDlg.h: 헤더 파일
//

#pragma once
#include "CameraManager.h"
#include "Protocol.h"
#include "afxcmn.h"
#include "afxwin.h"
#include <opencv2/opencv.hpp>

// CBottleCapDlg 대화 상자
class CBottleCapDlg : public CDialogEx
{
// 생성입니다.
public:
	CBottleCapDlg(CWnd* pParent = nullptr);	// 표준 생성자입니다.

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_BOTTLECAP_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.


// 구현입니다.
public:
	char m_szCamName[CAM_NUM][100];     
	char m_szSerialNum[CAM_NUM][100];      
	char m_szInterface[CAM_NUM][100];      
	CCameraManager  m_CameraManager;
	CSocket m_socket;

	int m_iCamNumber;                   
	int m_iCamPosition[CAM_NUM];            
	int m_iCameraIndex;                    
	int m_LogIndex;
	
	bool m_bConnected;
	bool m_bGrabDisplay;
	bool m_bSelectCamera;
	bool bStopThread[CAM_NUM];
	bool bLiveFlag[CAM_NUM];

	int m_error;
	int m_iListIndex;

	int nFrameCount[CAM_NUM];
	int m_nCamIndexBuf[CAM_NUM];
	double time[CAM_NUM];
	LARGE_INTEGER freq[CAM_NUM], start[CAM_NUM], end[CAM_NUM];

	HDC hdc[CAM_NUM];
	HWND hWnd[CAM_NUM];
	CRect rectStaticClient[CAM_NUM];
	BITMAPFILEHEADER fileheader;
	LPBITMAPINFO bitmapinfo[CAM_NUM];

	unsigned char** pImageresizeOrgBuffer[CAM_NUM];
protected:
	HICON m_hIcon;

	// 생성된 메시지 맵 함수
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedFindButton();
	CListCtrl m_ctrlCamList;
	
	CListCtrl m_ctrlLogList;
	void AllocImageBuf(void);
	void DisplayCam(void* pImageBuf);
	void DisplayGrab(void* pImageBuf);
	void InitBitmap(int nCamIndex);
	afx_msg void OnNMClickCameraList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBnClickedOpenButton();
	afx_msg void OnBnClickedCloseButton();
	afx_msg void OnBnClickedConnectButton();
	afx_msg void OnBnClickedGrabButton();
	afx_msg void OnBnClickedButton6();
	afx_msg void OnBnClickedButton7();
	afx_msg void OnBnClickedCheckCam();

	// TCP/IP
	bool Connect(CString serverIP, int port);
	bool Disconnect();
	bool SendProtocol(uchar protocol);
	bool SendImage(cv::Mat& image);
	bool RecvResponse(CString& response);

private:
	// 로그
	void CBottleCapDlg::insertLog(CString log);
};
