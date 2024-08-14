// PrevDialog.cpp: 구현 파일
//

#include "pch.h"
#include "BottleCap.h"
#include "afxdialogex.h"
#include "PrevDialog.h"


// PrevDialog 대화 상자

IMPLEMENT_DYNAMIC(PrevDialog, CDialogEx)

PrevDialog::PrevDialog(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_PrevDialog, pParent)
{

}

PrevDialog::~PrevDialog()
{
}

void PrevDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SELECT_COMBOX, m_SelectBox);
	DDX_Control(pDX, IDC_START_DATETIME, m_startTimePick);
	DDX_Control(pDX, IDC_END_DATETIME, m_endTimePick);
	DDX_Control(pDX, IDC_Chart, m_Chart);
	DDX_Control(pDX, IDC_PREV_LOG_LIST, m_PrevLogList);
}


BEGIN_MESSAGE_MAP(PrevDialog, CDialogEx)
	ON_BN_CLICKED(IDC_TODAY_BUTTON, &PrevDialog::OnBnClickedTodayButton)
	ON_BN_CLICKED(IDC_WEEK_BUTTON, &PrevDialog::OnBnClickedWeekButton)
	ON_BN_CLICKED(IDC_MONTH_BUTTON, &PrevDialog::OnBnClickedMonthButton)
END_MESSAGE_MAP()


// PrevDialog 메시지 처리기
BOOL PrevDialog::OnInitDialog()
{
	BOOL bResult = CDialogEx::OnInitDialog();

	// 여기에 초기화 코드를 작성하세요
	m_SelectBox.AddString(_T("전체보기"));
	m_SelectBox.AddString(_T("YesCap"));
	m_SelectBox.AddString(_T("NoCap"));
	m_SelectBox.SetCurSel(0);

	CTime today = CTime::GetCurrentTime();
	m_startTimePick.SetTime(&today);
	m_endTimePick.SetTime(&today);

	m_PrevLogList.InsertColumn(0, _T("Time"), LVCFMT_CENTER, 103, -1);
	m_PrevLogList.InsertColumn(1, _T("Result"), LVCFMT_CENTER, 229, -1);
	m_PrevLogList.SetExtendedStyle(
		LVS_EX_FULLROWSELECT |		// 아이템 선택시 전체행 선택
		LVS_EX_GRIDLINES |			// 그리드라인
		LVS_EX_ONECLICKACTIVATE |	// 핫트래킹 Or 클릭으로 아이템 선택
		LVS_EX_HEADERDRAGDROP
	);

	return bResult;
}

void PrevDialog::OnBnClickedTodayButton()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CTime today = CTime::GetCurrentTime();
	m_startTimePick.SetTime(&today);
	m_endTimePick.SetTime(&today);
}


void PrevDialog::OnBnClickedWeekButton()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CTime today = CTime::GetCurrentTime();
	CTime temp = today - CTimeSpan(7, 0, 0, 0);
	m_startTimePick.SetTime(&temp);
	m_endTimePick.SetTime(&today);
}


void PrevDialog::OnBnClickedMonthButton()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CTime today = CTime::GetCurrentTime();
	CTime temp = today - CTimeSpan(30, 0, 0, 0);
	m_startTimePick.SetTime(&temp);
	m_endTimePick.SetTime(&today);
}
