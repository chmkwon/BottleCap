#include <time.h>
#include "pch.h"
#include "framework.h"
#include "CameraManager.h"


CCameraManager::CCameraManager(void)
{
	// �ʱ�ȭ
	Pylon::PylonInitialize();

	// �ʱ�ȭ, ���� �����ִ�
	Pylon::PylonAutoInitTerm autoInitTerm;

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

bool CCameraManager::CheckCaptureEnd()
{
	return m_bCaptureEnd;
}

void CCameraManager::ReadEnd()
{
	m_bCaptureEnd = false;
}

int CCameraManager::FindCamera(CString& szCamName, CString& szCamSerialNumber, CString& szInterfacName)
{
	try
	{
		// ���� �ð� �α����� ����
		time(&t);
		//strftime(filename, sizeof(filename), "CameraManager_%Y_%m_%d_%H_%M_%S.log", localtime(&t));
		fopen_s(&log, filename, "w");
		CTime t = CTime::GetCurrentTime();
		Hour = t.GetHour();
		Min = t.GetMinute();
		Sec = t.GetSecond();

		m_tlFactory = &CTlFactory::GetInstance();
		devices.clear();

		//CInstantCamera camera(CTlFactory::GetInstance().CreateFirstDevice());

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
			for (DeviceInfoList_t::iterator it = devices.begin(); it != devices.end(); it++)
			{
				szInterfacName = (*it).GetDeviceClass();
				szCamName = (*it).GetModelName();
				szCamSerialNumber = (*it).GetSerialNumber();
			}
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
		if (m_Camera.IsOpen()) // ī�޶� �̹� open �Ȱ�� ��ȣ
		{
			return -1;
		}
		else
		{
			m_bRemoveCamera = false;
			m_bCamConnectFlag = false;
			m_bCamOpenFlag = false;
		}

		m_Camera.Attach(m_tlFactory->CreateFirstDevice());

		// Grab �Ϸ�ÿ�(PC�޸𸮿� Image �ܼ� �Ϸ�) ȣ��Ǵ� Event �Լ�			
		m_Camera.RegisterImageEventHandler(this, RegistrationMode_Append, Ownership_ExternalOwnership);

		m_Camera.Open();
		m_pCameraNodeMap = &m_Camera.GetNodeMap();

		m_bCamOpenFlag = true;

		return 0;
	}
	catch (GenICam::GenericException& e)
	{
		CString error = (CString)e.GetDescription();
		if (bLogUse == true)
		{
			WriteLog(_T("[ Open_Camera ]\n"), error);
		}
		return -2;
	}
}