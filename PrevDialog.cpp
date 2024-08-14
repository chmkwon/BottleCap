// PrevDialog.cpp: 구현 파일
//

#include "pch.h"
#include "BottleCap.h"
#include "afxdialogex.h"
#include "PrevDialog.h"
#include "Protocol.h"


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
	ON_BN_CLICKED(IDC_GETDATA_BUTTON2, &PrevDialog::OnBnClickedGetdataButton2)
	ON_WM_PAINT()
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

	m_bConnected = false;

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

// TCP/IP
bool PrevDialog::Connect(CString serverIP, int port)
{
	if (!AfxSocketInit())
	{
		AfxMessageBox(_T("소켓 초기화 실패"));
		return false;
	}

	if (!m_socket.Create())
	{
		AfxMessageBox(_T("소켓 생성 실패"));
		return false;
	}

	if (!m_socket.Connect(serverIP, port))
	{
		AfxMessageBox(_T("서버 연결 실패"));
		m_socket.Close();
		return false;
	}

	m_bConnected = true;
	return true;
}

bool PrevDialog::RecvResponse(CString& response)
{
	if (!m_bConnected)
	{
		return false;
	}
	else
	{
		char buffer[1024];
		int nBytes = m_socket.Receive(buffer, sizeof(buffer) - 1);
		if (nBytes > 0)
		{
			buffer[nBytes] = '\0';
			response = CString(buffer);
			return true;
		}
		return false;
	}
}

bool PrevDialog::Disconnect()
{
	if (m_bConnected)
	{
		m_socket.Close();
		m_bConnected = false;
		return true;
	}
	return false;
}

bool PrevDialog::SendProtocol(unsigned char protocol)
{
	if (!m_bConnected)
	{
		return false;
	}
	else
	{
		int sent = m_socket.Send(&protocol, sizeof(unsigned char));
		if (sent != sizeof(unsigned char))
		{
			return false;
		}
		return true;
	}
}

void PrevDialog::OnPaint()
{
	CPaintDC dc(this);

	if (m_bDrawChart)
	{
		DrawPieChart(&dc);
	}
}

void PrevDialog::OnBnClickedGetdataButton2()
{
	if (!m_bConnected)
	{
		if (!Connect(_T("10.10.21.110"), 9934))
		{
			return;
		}

		SendProtocol(Protocol::CHAR_DATA);
		CString temp;
		RecvResponse(temp);
		Disconnect();

		m_nNoData = 34;
		m_nYesData = 55;

		CWnd* pWnd = GetDlgItem(IDC_Chart);
		if (pWnd)
		{
			pWnd->GetWindowRect(&m_rectChart);
			ScreenToClient(&m_rectChart);
		}

		m_bDrawChart = true;

		Invalidate();
	}
}

void PrevDialog::DrawPieChart(CDC* pDC)
{
	COLORREF colorRed = RGB(255, 50, 50);
	COLORREF colorBlue = RGB(50, 50, 255);

	int total = m_nNoData + m_nYesData;
	if (total == 0) return;
	double angle1 = 360.0 * m_nNoData / total;
	int cx = m_rectChart.CenterPoint().x;
	int cy = m_rectChart.CenterPoint().y;
	int radius = min(m_rectChart.Width(), m_rectChart.Height()) / 2;

	CBrush brushRed(colorRed);
	CBrush* pOldBrush = pDC->SelectObject(&brushRed);
	pDC->Pie(cx - radius, cy - radius, cx + radius, cy + radius,
		cx + radius, cy,
		cx + static_cast<int>(radius * cos(angle1 * PI / 180)),
		cy - static_cast<int>(radius * sin(angle1 * PI / 180)));

	CBrush brushBlue(colorBlue);
	pDC->SelectObject(&brushBlue);
	pDC->Pie(cx - radius, cy - radius, cx + radius, cy + radius,
		cx + static_cast<int>(radius * cos(angle1 * PI / 180)),
		cy - static_cast<int>(radius * sin(angle1 * PI / 180)),
		cx + radius, cy);

	pDC->SelectObject(pOldBrush);

	// 텍스트 추가
	pDC->SetBkMode(TRANSPARENT);
	pDC->SetTextColor(RGB(255, 255, 255));

	CString str1;
	str1.Format(_T("NoCap\r\n%d%%"), static_cast<int>(m_nNoData * 100.0 / total + 0.5));
	double textAngle1 = angle1 / 2 * PI / 180;
	int textX1 = cx + static_cast<int>(radius * 0.1 * cos(textAngle1) - 40);
	int textY1 = cy - static_cast<int>(radius * 0.6 * sin(textAngle1));

	CRect textRect1(textX1, textY1, textX1 + 100, textY1 + 40);
	pDC->DrawText(str1, &textRect1, DT_CENTER | DT_VCENTER);

	CString str2;
	str2.Format(_T("YesCap\r\n%d%%"), static_cast<int>(m_nYesData * 100.0 / total + 0.5));
	double textAngle2 = (angle1 + (360 - angle1) / 2) * PI / 180;
	int textX2 = cx + static_cast<int>(radius * 0.1 * cos(textAngle2) - 50);
	int textY2 = cy - static_cast<int>(radius * 0.3 * sin(textAngle2));

	CRect textRect2(textX2, textY2, textX2 + 100, textY2 + 40);
	pDC->DrawText(str2, &textRect2, DT_CENTER | DT_VCENTER);
}