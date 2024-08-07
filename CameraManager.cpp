#include <time.h>
#include "framework.h"
#include "CameraManager.h"


CCameraManager::CCameraManager(void)
{
	// 초기화
	Pylon::PylonInitialize();

	// 초기화, 해제 도와주는
	Pylon::PylonAutoInitTerm autoInitTerm;

	m_bCaptureEnd = false;
	m_bCaptureEnd = false;
	m_bRemoveCamera = false;
	m_bCamConnectFlag = false;
	m_bCamOpenFlag = false;
	m_iGrabbedFrame = 0;
	m_iSkippiedFrame = 0;
	m_pCameraNodeMap = NULL;

	bLogUse = false;
};

CCameraManager::~CCameraManager(void)
{
	Pylon::PylonTerminate();
}

void CCameraManager::WriteLog(CString strTemp1, CString strTemp2)
{
	CTime t = CTime::GetCurrentTime();
	Hour = t.GetHour();
	Min = t.GetMinute();
	Sec = t.GetSecond();
	fprintf(log, "---------------------------------------------------------------------------\n");
	fprintf(log, "[%d시_%d분_%d초] [ CAM : %d ] Error \n", Hour, Min, Sec);
	fprintf(log, "---------------------------------------------------------------------------\n");
	fprintf(log, "[Error position]\n");
	fprintf(log, CT2A(strTemp1));
	fprintf(log, "---------------------------------------------------------------------------\n");
	fprintf(log, "[Error Detail]\n");
	fprintf(log, CT2A(strTemp2));
	fprintf(log, "\n---------------------------------------------------------------------------\n");
}

int CCameraManager::FindCamera()
{
	try
	{
		// 현재 시간 로그파일 생성
		time(&t);
		strftime(filename, sizeof(filename), "CameraManager_%Y_%m_%d_%H_%M_%S.log", localtime(&t));
		fopen_s(&log, filename, "w");
		CTime t = CTime::GetCurrentTime();
		Hour = t.GetHour();
		Min = t.GetMinute();
		Sec = t.GetSecond();

		m_tlFactory = &CTlFactory::GetInstance();
		devices.clear();

		if (m_tlFactory->EnumerateDevices(devices) == 0)
		// 카메라 찾지 못함
		{
			if (bLogUse == true)
			{
				fprintf(log, "[%d시_%d분_%d초] [FindCamera:: No Camera ]\n", Hour, Min, Sec);
			}
			return -1;
		}
		else
		// 카메라 찾음
		{
			
			return 0;
		}
	}
	catch (GenICam::GenericException& e)
	{
		CString error = (CString)e.GetDescription();
		if (bLogUse == true)
		{
			WriteLog(_T("[ FindCamera ]\n"), error);
		}
		return -2;
	}
}

int CCameraManager::OpenCamera()
{
	try
	{
		if (m_pCamera.IsOpen()) // 카메라가 이미 open 된경우 보호
		{
			return -1;
		}
		else
		{
			m_bRemoveCamera = false;
			m_bCamConnectFlag = false;
			m_bCamOpenFlag = false;
		}

		m_pCamera[0].Attach(m_tlFactory->CreateDevice(devices[nPosition - 1]));


		// 카메라 시스템에서 제거시에 호출되는 Callback 등록
		// Cable 접촉 불량으로 인한 카메라 연결 끊김 현상, 카메라 작동 불량의 경우
		// **Hearbeat Time 설정시간 후에 등록된 함수 호출됨
		m_pCamera[nCamIndex].RegisterConfiguration(this, RegistrationMode_Append, Ownership_ExternalOwnership);

		// Grab 완료시에(PC메모리에 Image 잔송 완료) 호출되는 Event 함수			

		m_pCamera[nCamIndex].RegisterImageEventHandler(this, RegistrationMode_Append, Ownership_ExternalOwnership);

		m_pCamera[nCamIndex].Open();

		m_pCameraNodeMap[nCamIndex] = &m_pCamera[nCamIndex].GetNodeMap();


		//	Pylon5.0.5버전의 경우, 아래 방식의 HeartbeatTimeOut설정은 허용하지 않음!!(버그 주의!!)
		//Debug 모드중 BreakPoint 걸어서 코드 확인시 HeartbeatTime시간동안 코드 이동이 없는 경우 카메라 연결이 끊어짐
		//	//release 배포시에는 시간을 3000ms(3초)로 설정
		//m_pHeartbeatTimeout[ nCamIndex ] = m_pCamera[ nCamIndex ].GetTLNodeMap().GetNode("HeartbeatTimeout");
		//if(IsAvailable(m_pHeartbeatTimeout[ nCamIndex ])){
		//	m_pHeartbeatTimeout[ nCamIndex ]->SetValue(10000);
		//}else{
		//	CString str;
		//	str.Format(_T("Camera%d, It is not available to set %s\n"),nCamIndex, _T("HeartbeatTimeout"));
		//	AfxMessageBox(str);
		//}


		m_bCamOpenFlag[nCamIndex] = true;

		//CFeaturePersistence::Load( "D:\\acA2500-14gc_21219426.pfs", &m_pCamera[ nCamIndex ].GetNodeMap(), true );

		return 0;
	}
	catch (GenICam::GenericException& e)
	{
		// Error handling
		CString error = (CString)e.GetDescription();
		if (bLogUse == true)
		{
			WriteLog(nCamIndex, _T("[ Open_Camera ]\n"), error);
		}
		return -2;
	}
}