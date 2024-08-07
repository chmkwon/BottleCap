#include <time.h>
#include "framework.h"
#include "CameraManager.h"


CCameraManager::CCameraManager(void)
{
	// �ʱ�ȭ
	Pylon::PylonInitialize();

	// �ʱ�ȭ, ���� �����ִ�
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
	fprintf(log, "[%d��_%d��_%d��] [ CAM : %d ] Error \n", Hour, Min, Sec);
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
		// ���� �ð� �α����� ����
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
		// ī�޶� ã�� ����
		{
			if (bLogUse == true)
			{
				fprintf(log, "[%d��_%d��_%d��] [FindCamera:: No Camera ]\n", Hour, Min, Sec);
			}
			return -1;
		}
		else
		// ī�޶� ã��
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
		if (m_pCamera.IsOpen()) // ī�޶� �̹� open �Ȱ�� ��ȣ
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


		// ī�޶� �ý��ۿ��� ���Žÿ� ȣ��Ǵ� Callback ���
		// Cable ���� �ҷ����� ���� ī�޶� ���� ���� ����, ī�޶� �۵� �ҷ��� ���
		// **Hearbeat Time �����ð� �Ŀ� ��ϵ� �Լ� ȣ���
		m_pCamera[nCamIndex].RegisterConfiguration(this, RegistrationMode_Append, Ownership_ExternalOwnership);

		// Grab �Ϸ�ÿ�(PC�޸𸮿� Image �ܼ� �Ϸ�) ȣ��Ǵ� Event �Լ�			

		m_pCamera[nCamIndex].RegisterImageEventHandler(this, RegistrationMode_Append, Ownership_ExternalOwnership);

		m_pCamera[nCamIndex].Open();

		m_pCameraNodeMap[nCamIndex] = &m_pCamera[nCamIndex].GetNodeMap();


		//	Pylon5.0.5������ ���, �Ʒ� ����� HeartbeatTimeOut������ ������� ����!!(���� ����!!)
		//Debug ����� BreakPoint �ɾ �ڵ� Ȯ�ν� HeartbeatTime�ð����� �ڵ� �̵��� ���� ��� ī�޶� ������ ������
		//	//release �����ÿ��� �ð��� 3000ms(3��)�� ����
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