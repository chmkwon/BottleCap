
// BottleCapDlg.h: 헤더 파일
//

#pragma once
#include "CameraManager.h"

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
	CString m_strCamName;
	CString m_strSerialNum;
	CString m_strInterface;
	CCameraManager  m_CameraManager;

	bool m_bSelectCamera;
	bool bStopThread;

	int m_error;
	int m_iListIndex;


	int nFrameCount;
	double time;
	LARGE_INTEGER freq, start, end;

	HDC hdc;
	HWND hWnd;
	CRect rectStaticClient;
	BITMAPFILEHEADER fileheader;
	LPBITMAPINFO  bitmapinfo;

	unsigned char** pImageresizeOrgBuffer;   
protected:
	HICON m_hIcon;

	// 생성된 메시지 맵 함수
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedCheck1();
	afx_msg void OnBnClickedButton3();
	afx_msg void OnBnClickedFindButton();
	CListCtrl m_ctrlCamList;
	
	CListCtrl m_ctrlLogList;
	void AllocImageBuf(void);
	void DisplayCam(void* pImageBuf);
	afx_msg void OnNMClickCameraList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBnClickedOpenButton();
};
