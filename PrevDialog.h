#pragma once
#include "afxdialogex.h"
#include <cmath>

#define PI 3.14159265358979323846

// PrevDialog 대화 상자

class PrevDialog : public CDialogEx
{
	DECLARE_DYNAMIC(PrevDialog)

public:
	PrevDialog(CWnd* pParent = nullptr);   // 표준 생성자입니다.
	virtual ~PrevDialog();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PrevDialog };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.
	afx_msg BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
	bool m_bConnected;

	CComboBox m_SelectBox;
	CDateTimeCtrl m_startTimePick;
	CDateTimeCtrl m_endTimePick;
	CStatic m_Chart;
	CSocket m_socket;
	afx_msg void OnBnClickedTodayButton();
	afx_msg void OnBnClickedWeekButton();
	afx_msg void OnBnClickedMonthButton();
	afx_msg void OnPaint();
	CListCtrl m_PrevLogList;

	// TCP/IP
	bool Connect(CString serverIP, int port);
	bool Disconnect();
	bool SendProtocol(unsigned char protocol);
	bool RecvResponse(CString& response);
	bool PrevDialog::SendCString(const CString& message);
	int RecvInt();
	afx_msg void OnBnClickedGetdataButton2();

	// 차트
	int m_nNoData;
	int m_nYesData;
	CRect m_rectChart;
	bool m_bDrawChart;

	void PrevDialog::DrawPieChart(CDC* pDC);
};
