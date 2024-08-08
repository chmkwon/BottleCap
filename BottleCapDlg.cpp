
// BottleCapDlg.cpp: 구현 파일
//

#include "pch.h"
#include "framework.h"
#include "BottleCap.h"
#include "BottleCapDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CBottleCapDlg* pMainDlg;

UINT LiveGrabThreadCam(LPVOID pParam)
{
	int nindex = 0;
	int nCamIndex = *(int*)pParam;

	QueryPerformanceCounter(&(pMainDlg->start));
	pMainDlg->nFrameCount = 0;
	CString Info;
	while (pMainDlg->bStopThread == true)
	{
		if (pMainDlg->m_CameraManager.m_bRemoveCamera == true)
		{
			if (pMainDlg->m_strSerialNum == pMainDlg->m_ctrlCamList.GetItemText(0, 1))
			{
				pMainDlg->m_CameraManager.m_bRemoveCamera = false;
				pMainDlg->m_ctrlCamList.SetItemText(0, 2, _T("LostConnection"));
			}
		}
		else
		{
			if (pMainDlg->m_CameraManager.CheckCaptureEnd()) //exposure end true일때 
			{
				pMainDlg->nFrameCount++;
				QueryPerformanceCounter(&(pMainDlg->end));
				pMainDlg->m_CameraManager.ReadEnd();  // exposure end flag 변경 
				pMainDlg->DisplayCam(pMainDlg->pImageresizeOrgBuffer[0]);
				
				nindex++;
				if (nindex == BUF_NUM)
				{
					nindex = 0;
				}
				if (pMainDlg->end.QuadPart / (pMainDlg->freq.QuadPart / 1000.0) > pMainDlg->start.QuadPart / (pMainDlg->freq.QuadPart / 1000.0) + 1000)
				{
					CString temp;
					temp.Format(_T("%d fps"), pMainDlg->nFrameCount);

					pMainDlg->SetDlgItemText(CAMERA_STATS, temp);
					pMainDlg->nFrameCount = 0;
					QueryPerformanceCounter(&(pMainDlg->start));
				}

				Info.Format(_T("Grabbed Frame = %d , SkippedFrame = %d"), pMainDlg->m_CameraManager.m_iGrabbedFrame, pMainDlg->m_CameraManager.m_iSkippiedFrame);
				
				pMainDlg->SetDlgItemText(CAM_INFO, Info);
			}
		}
		//   Sleep(1);
	}

	return 0;
}

// 응용 프로그램 정보에 사용되는 CAboutDlg 대화 상자입니다.

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

// 구현입니다.
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CBottleCapDlg 대화 상자



CBottleCapDlg::CBottleCapDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_BOTTLECAP_DIALOG, pParent)
{

	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	 
	bStopThread = false;
	m_bSelectCamera = false;
	/*for (int i = 0; i < CAM_NUM; i++)
	{
		pImageresizeOrgBuffer[i] = NULL;

		pImageColorDestBuffer[i] = NULL;
		bitmapinfo[i] = NULL;
		bStopThread[i] = false;
		nFrameCount[i] = 0;
		time[i] = 0;
		m_CameraManager.m_iCM_Width[i] = 1;
		m_CameraManager.m_iCM_Height[i] = 1;
		QueryPerformanceFrequency(&freq[i]);
		m_nCamIndexBuf[i] = i;
	}*/
}

void CBottleCapDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CAMERA_LIST, m_ctrlCamList);
	DDX_Control(pDX, IDC_LOG_LIST, m_ctrlLogList);
}

BEGIN_MESSAGE_MAP(CBottleCapDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_FIND_BUTTON, &CBottleCapDlg::OnBnClickedFindButton)
	ON_NOTIFY(NM_CLICK, IDC_CAMERA_LIST, &CBottleCapDlg::OnNMClickCameraList)
	ON_BN_CLICKED(IDC_OPEN_BUTTON, &CBottleCapDlg::OnBnClickedOpenButton)
END_MESSAGE_MAP()


// CBottleCapDlg 메시지 처리기

BOOL CBottleCapDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 시스템 메뉴에 "정보..." 메뉴 항목을 추가합니다.

	// IDM_ABOUTBOX는 시스템 명령 범위에 있어야 합니다.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 이 대화 상자의 아이콘을 설정합니다.  응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	// TODO: 여기에 추가 초기화 작업을 추가합니다.

	m_ctrlCamList.InsertColumn(0, _T("Model Name"), LVCFMT_CENTER, 103, -1);
	m_ctrlCamList.InsertColumn(2, _T("Serial Num"), LVCFMT_CENTER, 100, -1);
	m_ctrlCamList.InsertColumn(3, _T("Stats"), LVCFMT_CENTER, 140, -1);
	m_ctrlCamList.SetExtendedStyle(	
		LVS_EX_FULLROWSELECT |		// 아이템 선택시 전체행 선택
		LVS_EX_GRIDLINES |			// 그리드라인
		LVS_EX_ONECLICKACTIVATE |	// 핫트래킹 Or 클릭으로 아이템 선택
		LVS_EX_HEADERDRAGDROP
	);

	m_ctrlLogList.InsertColumn(0, _T("Time"), LVCFMT_CENTER, 103, -1);
	m_ctrlLogList.InsertColumn(2, _T("Result"), LVCFMT_CENTER, 100, -1);
	m_ctrlLogList.InsertColumn(3, _T("Confidence"), LVCFMT_CENTER, 140, -1);
	m_ctrlLogList.SetExtendedStyle(
		LVS_EX_FULLROWSELECT |		// 아이템 선택시 전체행 선택
		LVS_EX_GRIDLINES |			// 그리드라인
		LVS_EX_ONECLICKACTIVATE |	// 핫트래킹 Or 클릭으로 아이템 선택
		LVS_EX_HEADERDRAGDROP
	);

	pMainDlg = this;

	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

void CBottleCapDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다.  문서/뷰 모델을 사용하는 MFC 애플리케이션의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void CBottleCapDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트입니다.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 아이콘을 그립니다.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR CBottleCapDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CBottleCapDlg::OnBnClickedCheck1()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
}

void CBottleCapDlg::DisplayCam(void* pImageBuf)
{
	SetStretchBltMode(hdc, COLORONCOLOR);
	StretchDIBits(hdc, 0, 0, rectStaticClient.Width(), rectStaticClient.Height(), 0, 0, (int)m_CameraManager.m_iCM_reSizeWidth, (int)m_CameraManager.m_iCM_Height, pImageBuf, bitmapinfo, DIB_RGB_COLORS, SRCCOPY);
}

void CBottleCapDlg::OnBnClickedFindButton()
{
	m_error = m_CameraManager.FindCamera(m_strCamName, m_strSerialNum, m_strInterface);

	if (m_error == 0)
	{
		m_ctrlCamList.DeleteAllItems();
		m_ctrlCamList.InsertItem(0, m_strCamName);
		m_ctrlCamList.SetItemText(0, 1, m_strSerialNum);
		m_ctrlCamList.SetItemText(0, 2, _T("Find_Success"));

		m_bSelectCamera = true;
	}
	else if (m_error == -1)
	{
		AfxMessageBox(_T("연결된 카메라가 없습니다."));
	}
	else if (m_error == -2)
	{
		AfxMessageBox(_T("Pylon Function Error"));
	}
}


void CBottleCapDlg::OnNMClickCameraList(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);

	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	m_iListIndex = pNMListView->iItem;
	SetDlgItemText(IDC_SELECT_CAM, m_ctrlCamList.GetItemText(m_iListIndex, 0));

	*pResult = 0;
}


void CBottleCapDlg::OnBnClickedOpenButton()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	if (m_bSelectCamera)
	{
		int error = m_CameraManager.OpenCamera();
		if (error == 0)
		{
			m_ctrlCamList.SetItemText(m_iListIndex, 2, _T("Open_Success"));

		}
		else if (error == -1)
		{
			m_ctrlCamList.SetItemText(m_iListIndex, 2, _T("Alread_Open"));
		}
		else
		{
			m_ctrlCamList.SetItemText(m_iListIndex, 2, _T("Open_Fail"));
		}
	}
	else
	{
		AfxMessageBox(_T("리스트에서 카메라를 먼저 선택하세요."));
	}
}
