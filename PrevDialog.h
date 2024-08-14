#pragma once
#include "afxdialogex.h"


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
	CComboBox m_SelectBox;
	CDateTimeCtrl m_startTimePick;
	CDateTimeCtrl m_endTimePick;
	CStatic m_Chart;
	afx_msg void OnBnClickedTodayButton();
	afx_msg void OnBnClickedWeekButton();
	afx_msg void OnBnClickedMonthButton();
	CListCtrl m_PrevLogList;
};
