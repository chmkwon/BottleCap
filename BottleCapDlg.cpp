
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

	if (pMainDlg == nullptr)
	{
		AfxMessageBox(_T("에러!!"));
		return 1; 
	}

	QueryPerformanceCounter(&(pMainDlg->start[nCamIndex]));
	pMainDlg->nFrameCount[nCamIndex] = 0;
	CString Info;
	while (pMainDlg->bStopThread[nCamIndex] == true)
	{
		if (pMainDlg->m_CameraManager.m_bRemoveCamera[nCamIndex] == true)
		{
			if (pMainDlg->m_szSerialNum[nCamIndex] == pMainDlg->m_ctrlCamList.GetItemText(0, 2))
			{
				pMainDlg->m_CameraManager.m_bRemoveCamera[nCamIndex] = false;
				pMainDlg->m_ctrlCamList.SetItemText(0, 3, _T("LostConnection"));
			}
			else if (pMainDlg->m_szSerialNum[nCamIndex] == pMainDlg->m_ctrlCamList.GetItemText(1, 2))
			{
				pMainDlg->m_CameraManager.m_bRemoveCamera[nCamIndex] = false;
				pMainDlg->m_ctrlCamList.SetItemText(1, 3, _T("LostConnection"));
			}
		}
		else
		{
			if (pMainDlg->m_CameraManager.CheckCaptureEnd(nCamIndex)) //exposure end true일때 
			{
				pMainDlg->nFrameCount[nCamIndex]++;
				QueryPerformanceCounter(&(pMainDlg->end[nCamIndex]));
				if (pMainDlg->m_CameraManager.m_strCM_ImageForamt[nCamIndex] == "Mono8")
				{
					for (int y = 0; y < pMainDlg->m_CameraManager.m_iCM_Height[nCamIndex]; y++)          // widht가 4의 배수가 아닌 경우 ex)659  - > 660으로 memcpy
					{
						memcpy(&pMainDlg->pImageresizeOrgBuffer[nCamIndex][nindex][y * pMainDlg->m_CameraManager.m_iCM_reSizeWidth[nCamIndex]], &pMainDlg->m_CameraManager.pImage8Buffer[nCamIndex][y * pMainDlg->m_CameraManager.m_iCM_Width[nCamIndex]], pMainDlg->m_CameraManager.m_iCM_Width[nCamIndex]);
					}
					pMainDlg->m_CameraManager.ReadEnd(nCamIndex);  // exposure end flag 변경 
					switch (nCamIndex)
					{
					case 0:
						pMainDlg->DisplayCam(pMainDlg->pImageresizeOrgBuffer[0][nindex]);

						if (pMainDlg->m_bGrabDisplay)
						{
							pMainDlg->DisplayGrab(pMainDlg->pImageresizeOrgBuffer[0][nindex]);
							pMainDlg->m_bGrabDisplay = false;
						}
						break;
					}
				}
				nindex++;
				if (nindex == BUF_NUM)
				{
					nindex = 0;
				}
				if (pMainDlg->end[nCamIndex].QuadPart / (pMainDlg->freq[nCamIndex].QuadPart / 1000.0) > pMainDlg->start[nCamIndex].QuadPart / (pMainDlg->freq[nCamIndex].QuadPart / 1000.0) + 1000)
				{
					CString temp;
					temp.Format(_T("%d fps"), pMainDlg->nFrameCount[nCamIndex]);
					if (nCamIndex == 0)
					{
						pMainDlg->SetDlgItemText(CAMERA_STATS, temp);
					}
					pMainDlg->nFrameCount[nCamIndex] = 0;
					QueryPerformanceCounter(&(pMainDlg->start[nCamIndex]));
				}
				Info.Format(_T("Grabbed Frame = %d , SkippedFrame = %d"), pMainDlg->m_CameraManager.m_iGrabbedFrame[nCamIndex], pMainDlg->m_CameraManager.m_iSkippiedFrame[nCamIndex]);
				switch (nCamIndex)
				{
				case 0:
					pMainDlg->SetDlgItemText(CAM_INFO, Info);
					break;
				}
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
	for (int i = 0; i < CAM_NUM; i++)
	{
		pImageresizeOrgBuffer[i] = NULL;
		bitmapinfo[i] = NULL;
		bStopThread[i] = false;
		nFrameCount[i] = 0;
		time[i] = 0;
		m_CameraManager.m_iCM_Width[i] = 1;
		m_CameraManager.m_iCM_Height[i] = 1;
		QueryPerformanceFrequency(&freq[i]);
		m_nCamIndexBuf[i] = i;
	}
	m_bGrabDisplay = false;
	m_bConnected = false;
	m_bSendImage = false;
	m_LogIndex = 0;
	m_iCameraIndex = -1;
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
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
	ON_BN_CLICKED(IDC_CLOSE_BUTTON, &CBottleCapDlg::OnBnClickedCloseButton)
	ON_BN_CLICKED(IDC_CONNECT_BUTTON, &CBottleCapDlg::OnBnClickedConnectButton)
	ON_BN_CLICKED(IDC_GRAB_BUTTON, &CBottleCapDlg::OnBnClickedGrabButton)
	ON_BN_CLICKED(IDC_CHECK_CAM, &CBottleCapDlg::OnBnClickedCheckCam)
	ON_BN_CLICKED(IDC_SEND_BUTTON, &CBottleCapDlg::OnBnClickedSendButton)
	ON_BN_CLICKED(IDC_PREV_BUTTON, &CBottleCapDlg::OnBnClickedPrevButton)
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
	m_ctrlLogList.InsertColumn(1, _T("Result"), LVCFMT_CENTER, 240, -1);
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

void CBottleCapDlg::DisplayCam(void* pImageBuf)
{
	SetStretchBltMode(hdc[0], COLORONCOLOR);
	StretchDIBits(hdc[0], 0, 0, rectStaticClient[0].Width(), rectStaticClient[0].Height(), 0, 0, (int)m_CameraManager.m_iCM_reSizeWidth[0], (int)m_CameraManager.m_iCM_Height[0], pImageBuf, bitmapinfo[0], DIB_RGB_COLORS, SRCCOPY);
}

void CBottleCapDlg::OnBnClickedFindButton()
{
	m_error = m_CameraManager.FindCamera(m_szCamName, m_szSerialNum, m_szInterface, &m_iCamNumber);

	if (m_error == 0)
	{
		m_ctrlCamList.DeleteAllItems();
		int  nCount = 0;
		CString strSerialNum;
		for (int i = 0; i < m_iCamNumber; i++)
		{
			nCount++;
			CString strcamname = (CString)m_szCamName[i];
			strSerialNum = (CString)m_szSerialNum[i];

			if (nCount == 1)
			{
				m_iCamPosition[0] = 1;
			}

			m_ctrlCamList.InsertItem(i, strcamname);
			m_ctrlCamList.SetItemText(i, 1, strSerialNum);
			m_ctrlCamList.SetItemText(i, 2, _T("Find_Success"));
			insertLog(_T("Find_Success"));
			m_bSelectCamera = true;
		}
	}
	else if (m_error == -1)
	{
		AfxMessageBox(_T("연결된 카메라가 없습니다."));
		insertLog(_T("No_Camera"));
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
	m_iCameraIndex = 0;
	*pResult = 0;
}


void CBottleCapDlg::OnBnClickedOpenButton()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	if (m_iCameraIndex != -1)
	{/*
		int a = m_iCameraIndex;
		int b = m_iCamPosition[m_iCameraIndex];

		CString message;
		message.Format(_T("%d %d"), a, b);
		AfxMessageBox(message);*/

		int error = m_CameraManager.Open_Camera(m_iCameraIndex, m_iCamPosition[m_iCameraIndex]);
		if (error == 0)
		{
			m_ctrlCamList.SetItemText(m_iListIndex, 2, _T("Open_Success"));
			insertLog(_T("Open_Success"));

		}
		else if (error == -1)
		{
			m_ctrlCamList.SetItemText(m_iListIndex, 2, _T("Alread_Open"));
			insertLog(_T("Alread_Open"));
		}
		else
		{
			m_ctrlCamList.SetItemText(m_iListIndex, 2, _T("Open_Fail"));
			insertLog(_T("Open_Fail"));
		}
	}
	else
	{
		AfxMessageBox(_T("리스트에서 카메라를 먼저 선택하세요."));
	}
}


void CBottleCapDlg::OnBnClickedCloseButton()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	if (m_CameraManager.m_bCamOpenFlag[m_iCameraIndex] == true)
	{
		if (m_CameraManager.Close_Camera(m_iCameraIndex) == 0)
		{
			m_ctrlCamList.SetItemText(m_iListIndex, 2, _T("Close_Success"));
			insertLog(_T("Close_Success"));
			if (bitmapinfo[m_iCameraIndex])
			{
				delete bitmapinfo[m_iCameraIndex];
				bitmapinfo[m_iCameraIndex] = NULL;
			}
			if (pImageresizeOrgBuffer[m_iCameraIndex])
			{
				for (int i = 0; i < BUF_NUM; i++)
				{
					free(pImageresizeOrgBuffer[m_iCameraIndex][i]);
				}
				free(pImageresizeOrgBuffer[m_iCameraIndex]);
				pImageresizeOrgBuffer[m_iCameraIndex] = NULL;
			}
		}
		else
		{
			m_ctrlCamList.SetItemText(m_iListIndex, 2, _T("Close_Fail"));

		}
	}
}

void CBottleCapDlg::OnBnClickedConnectButton()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	if (m_CameraManager.m_bCamOpenFlag[m_iCameraIndex] == true)
	{
		if (m_CameraManager.Connect_Camera(m_iCameraIndex, 0, 0, 1984, 1264, _T("Mono8")) == 0)    //BayerBG8   YUV422Packed  Mono8 , Mono16
		{
			m_ctrlCamList.SetItemText(m_iListIndex, 2, _T("Connect_Success"));
			insertLog(_T("Connect_Success"));
			AllocImageBuf();
			InitBitmap(m_iCameraIndex);
		}
		else
		{
			m_ctrlCamList.SetItemText(m_iListIndex, 2, _T("Connect_Fail"));
			insertLog(_T("Connect_Fail"));
		}
	}
	else
	{
		AfxMessageBox(_T("사용할 카메라 Open을 하세요!!"));
	}
}
void CBottleCapDlg::OnBnClickedGrabButton()
{
	if (bLiveFlag[0] = true)
	{
		m_CameraManager.SingleGrab(m_iCameraIndex);
		m_bGrabDisplay = true;
	}
	else
	{
		AfxMessageBox(_T("라이브 부터 키세요!!"));
	}
}

void CBottleCapDlg::DisplayGrab(void* pImageBuf)
{
	CWnd* pWnd = GetDlgItem(IDC_GRABBED_DISPLAY);

	cv::Mat image(m_CameraManager.m_iCM_Height[m_iCameraIndex],
		m_CameraManager.m_iCM_reSizeWidth[m_iCameraIndex],
		CV_8UC1);

	memcpy(image.data, pImageBuf, image.total() * image.elemSize());

	// 윤곽선 검출
	cv::Mat temp;
	cv::cvtColor(image, temp, cv::COLOR_GRAY2BGR); // 컬러 이미지로 변환
	cv::Mat gray;
	cv::cvtColor(temp, gray, cv::COLOR_BGR2GRAY);
	cv::GaussianBlur(gray, gray, cv::Size(5, 5), 0);
	cv::Canny(gray, gray, 50, 150);
	std::vector<std::vector<cv::Point>> contours;
	cv::findContours(gray, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

	if (!contours.empty())
	{
		// ROI 계산
		int minX = image.cols, minY = image.rows, maxX = 0, maxY = 0;
		for (const auto& contour : contours)
		{
			for (const auto& point : contour)
			{
				minX = std::min(minX, point.x);
				minY = std::min(minY, point.y);
				maxX = std::max(maxX, point.x);
				maxY = std::max(maxY, point.y);
			}
		}

		// 병 전체 ROI
		cv::Rect fullRoi(minX, minY, maxX - minX, maxY - minY);
		// 뚜껑 부분 ROI (5분의 1)
		cv::Rect capRoi(fullRoi.x, fullRoi.y, fullRoi.width, fullRoi.height / 5);

		// 뚜껑 부분 ROI 추출
		m_roiImage = image(capRoi);

		// ROI 그리기
		cv::rectangle(temp, fullRoi, cv::Scalar(255, 0, 0), 2);
		cv::rectangle(temp, capRoi, cv::Scalar(0, 0, 255), 2);

		std::vector<uchar> buf;
		cv::imencode(".bmp", temp, buf);
		BYTE* roiImageBuf = buf.data();

		// 캡처 컨트롤에 표시
		CDC* pDC = pWnd->GetDC();
		CRect rect;
		pWnd->GetClientRect(&rect);

		BITMAPINFO updatedBitmapInfo = *bitmapinfo[m_iCameraIndex];
		updatedBitmapInfo.bmiHeader.biWidth = temp.cols;
		updatedBitmapInfo.bmiHeader.biHeight = temp.rows;
		updatedBitmapInfo.bmiHeader.biBitCount = 24;

		SetStretchBltMode(pDC->GetSafeHdc(), COLORONCOLOR);
		StretchDIBits(pDC->GetSafeHdc(), 0, 0, rect.Width(), rect.Height(),
			0, 0, temp.cols, temp.rows,
			roiImageBuf, &updatedBitmapInfo, DIB_RGB_COLORS, SRCCOPY);
		pWnd->ReleaseDC(pDC);

		// 전송버튼 초기화
		m_bSendImage = false;
	}
	else
	{
		// 캡처 컨트롤에 표시
		CDC* pDC = pWnd->GetDC();
		CRect rect;
		pWnd->GetClientRect(&rect);

		BITMAPINFO updatedBitmapInfo = *bitmapinfo[m_iCameraIndex];
		updatedBitmapInfo.bmiHeader.biWidth = temp.cols;
		updatedBitmapInfo.bmiHeader.biHeight = temp.rows;
		updatedBitmapInfo.bmiHeader.biBitCount = 24;

		SetStretchBltMode(pDC->GetSafeHdc(), COLORONCOLOR);
		StretchDIBits(pDC->GetSafeHdc(), 0, 0, rect.Width(), rect.Height(),
			0, 0, temp.cols, temp.rows,
			pImageBuf, &updatedBitmapInfo, DIB_RGB_COLORS, SRCCOPY);
		pWnd->ReleaseDC(pDC);
	}
}

void CBottleCapDlg::OnBnClickedSendButton()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	//// 통신
	if (m_bSendImage == true)
	{
		insertLog(_T("Already_Send"));
		return;
	}
	if (m_roiImage.empty())
	{
		AfxMessageBox(_T("캡쳐부터 해주세요!!"));
		return;
	}
	if (!m_bConnected)
	{
		if (Connect(_T("10.10.21.110"), 9934))
		{
			insertLog(_T("Sever_Connect"));
		}
		else
		{
			insertLog(_T("Connect_Fail"));
			return;
		}
	}
	SendProtocol(Protocol::CHECK_RESULT);
	if (SendImage(m_roiImage))
	{
		AfxMessageBox(_T("전송 완료"));
		insertLog(_T("Image_Send_Success"));
		m_bSendImage = true;
		CString temp;
		RecvResponse(temp);
		insertLog(temp);

		Disconnect();
	}
	else
	{
		insertLog(_T("Image_Send_Fail"));
	}
}

void CBottleCapDlg::OnBnClickedPrevButton()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
}

void CBottleCapDlg::OnBnClickedCheckCam()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	if (m_CameraManager.m_bCamConnectFlag[0] == true)
	{
		// 비트연산토글
		bStopThread[0] = (bStopThread[0] + 1) & 0x01;

		if (bStopThread[0])
		{
			bLiveFlag[0] = true;
			m_CameraManager.GrabLive(0, 0);
			AfxBeginThread(LiveGrabThreadCam, &m_nCamIndexBuf[0]);
		}
		else
		{
			bLiveFlag[0] = false;			
			m_CameraManager.LiveStop(0, 0);
		}
	}
	else
	{
		AfxMessageBox(_T("Camera0 Connect를 하세요!!"));
		CButton* pButton = (CButton*)pMainDlg->GetDlgItem(IDC_CHECK_CAM);
		pButton->SetCheck(0);
	}
}

void CBottleCapDlg::AllocImageBuf(void)
{
	UpdateData();
	if (m_CameraManager.m_strCM_ImageForamt[m_iCameraIndex] == "Mono8")   // Mono
	{

		pImageresizeOrgBuffer[m_iCameraIndex] = (unsigned char**)malloc(BUF_NUM * sizeof(unsigned char*));
		for (int i = 0; i < BUF_NUM; i++)
		{
			pImageresizeOrgBuffer[m_iCameraIndex][i] = (unsigned char*)malloc(m_CameraManager.m_iCM_reSizeWidth[m_iCameraIndex] * m_CameraManager.m_iCM_Height[m_iCameraIndex]);
		}
	}

	// Display 핸들
	switch (m_iCameraIndex)
	{
	case 0:
		hWnd[0] = GetDlgItem(IDC_CAM_DISPLAY)->GetSafeHwnd();
		GetDlgItem(IDC_CAM_DISPLAY)->GetClientRect(&rectStaticClient[0]);
		hdc[0] = ::GetDC(hWnd[0]);
		break;
	}
}

void CBottleCapDlg::InitBitmap(int nCamIndex)
{
	if (m_CameraManager.m_strCM_ImageForamt[m_iCameraIndex] == "Mono8" || m_CameraManager.m_strCM_ImageForamt[m_iCameraIndex] == "Mono16")
	{
		if (bitmapinfo[nCamIndex])
		{
			delete bitmapinfo[nCamIndex];
			bitmapinfo[nCamIndex] = NULL;
		}
		bitmapinfo[nCamIndex] = (BITMAPINFO*)(new char[sizeof(BITMAPINFOHEADER) +
			256 * sizeof(RGBQUAD)]);

		bitmapinfo[nCamIndex]->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bitmapinfo[nCamIndex]->bmiHeader.biWidth = (int)m_CameraManager.m_iCM_reSizeWidth[nCamIndex];
		bitmapinfo[nCamIndex]->bmiHeader.biHeight = -(int)m_CameraManager.m_iCM_Height[nCamIndex];
		bitmapinfo[nCamIndex]->bmiHeader.biPlanes = 1;
		bitmapinfo[nCamIndex]->bmiHeader.biCompression = BI_RGB;
		bitmapinfo[nCamIndex]->bmiHeader.biBitCount = 8;
		bitmapinfo[nCamIndex]->bmiHeader.biSizeImage = (int)(m_CameraManager.m_iCM_reSizeWidth[nCamIndex] * m_CameraManager.m_iCM_Height[nCamIndex]);
		bitmapinfo[nCamIndex]->bmiHeader.biXPelsPerMeter = 0;
		bitmapinfo[nCamIndex]->bmiHeader.biYPelsPerMeter = 0;
		bitmapinfo[nCamIndex]->bmiHeader.biClrUsed = 256;
		bitmapinfo[nCamIndex]->bmiHeader.biClrImportant = 0;

		for (int j = 0;j < 256;j++)
		{
			bitmapinfo[nCamIndex]->bmiColors[j].rgbRed = (unsigned char)j;
			bitmapinfo[nCamIndex]->bmiColors[j].rgbGreen = (unsigned char)j;
			bitmapinfo[nCamIndex]->bmiColors[j].rgbBlue = (unsigned char)j;
			bitmapinfo[nCamIndex]->bmiColors[j].rgbReserved = 0;
		}
	}
}

// TCP/IP
bool CBottleCapDlg::Connect(CString serverIP, int port)
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

bool CBottleCapDlg::SendImage(cv::Mat& image)
{
	if (!m_bConnected)
	{
		return false;
	}
	else
	{
		std::vector<uchar> buffer;
		cv::imencode(".jpg", image, buffer);

		int size = buffer.size();
		if (m_socket.Send(&size, sizeof(size)) != sizeof(size))
			return false;

		int totalSent = 0;
		while (totalSent < size)
		{
			int sent = m_socket.Send(buffer.data() + totalSent, size - totalSent);
			if (sent == SOCKET_ERROR)
				return false;
			totalSent += sent;
		}
		return true;
	}
}

bool CBottleCapDlg::RecvResponse(CString& response)
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

bool CBottleCapDlg::Disconnect()
{
	if (m_bConnected)
	{
		m_socket.Close();
		m_bConnected = false;
		return true;
	}
	return false;
}

bool CBottleCapDlg::SendProtocol(uchar protocol)
{
	if (!m_bConnected)
	{
		return false;
	}
	else
	{
		int sent = m_socket.Send(&protocol, sizeof(uchar));
		if (sent != sizeof(uchar))
		{
			return false;
		}
		return true;
	}
}

void CBottleCapDlg::insertLog(CString log)
{
	CTime currentTime = CTime::GetCurrentTime();
	CString strCurrentTime = currentTime.Format("%H:%M:%S");
	m_ctrlLogList.InsertItem(m_LogIndex, strCurrentTime);
	m_ctrlLogList.SetItemText(m_LogIndex, 1, log);
	m_LogIndex++;
}