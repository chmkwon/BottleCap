#include <time.h>
#include "pch.h"
#include "framework.h"
#include "CameraManager.h"
//

CCameraManager::CCameraManager(void)
{
	Pylon::PylonInitialize();

	Pylon::PylonAutoInitTerm autoInitTerm;

	for (int i = 0; i < CAM_NUM; i++)
	{
		m_bCaptureEnd[i] = false;
		m_bRemoveCamera[i] = false;
		m_bCamConnectFlag[i] = false;
		m_bCamOpenFlag[i] = false;
		m_iGrabbedFrame[i] = 0;
		m_iSkippiedFrame[i] = 0;
		m_pCameraNodeMap[i] = NULL;
	}
	m_imgNjm = 0;
	

	bLogUse = false;   // Log  사용시 true로 변경
}

CCameraManager::~CCameraManager(void)
{

	Pylon::PylonTerminate();

}
int CCameraManager::FindCamera(char szCamName[CAM_NUM][100], char szCamSerialNumber[CAM_NUM][100], char szInterfacName[CAM_NUM][100], int* nCamNumber)
{
	try
	{
		if (bLogUse == true)
		{
			time(&t);
			struct tm timeinfo;
			localtime_s(&timeinfo, &t);
			strftime(filename, sizeof(filename), "CameraManager_%Y_%m_%d_%H_%M_%S.log", &timeinfo);

			errno_t err = fopen_s(&log, filename, "w");

			CTime t = CTime::GetCurrentTime();
			Hour = t.GetHour();
			Min = t.GetMinute();
			Sec = t.GetSecond();
		}

		m_tlFactory = &CTlFactory::GetInstance();
		devices.clear();
		int iCamnumber = -1;
		if (m_tlFactory->EnumerateDevices(devices) == 0)
		{
			if (bLogUse == true)
			{
				fprintf(log, "[%d시_%d분_%d초] [FindCamera:: No Camera ]\n", Hour, Min, Sec);
			}
			return -1;
		}
		else
		{
			m_pCamera.Initialize(CAM_NUM);
			for (DeviceInfoList_t::iterator it = devices.begin(); it != devices.end(); it++)
			{
				iCamnumber++;
				strcpy_s(szInterfacName[iCamnumber], (*it).GetDeviceClass().c_str());
				strcpy_s(szCamName[iCamnumber], (*it).GetModelName().c_str());
				strcpy_s(szCamSerialNumber[iCamnumber], (*it).GetSerialNumber().c_str());

			}
			*nCamNumber = iCamnumber + 1;

			return 0;
		}
	}
	catch (GenICam::GenericException& e)
	{
		// Error handling
		CString error = (CString)e.GetDescription();
		if (bLogUse == true)
		{
			WriteLog(100, _T("[ FindCamera ]\n"), error);
		}
		return -2;
	}
}

int CCameraManager::Open_Camera(int nCamIndex, int nPosition)
{
	try
	{
		if (m_pCamera[nCamIndex].IsOpen()) // 카메라가 이미 open 된경우 보호
		{
			return -1;
		}
		else
		{
			m_bRemoveCamera[nCamIndex] = false;
			m_bCamConnectFlag[nCamIndex] = false;
			m_bCamOpenFlag[nCamIndex] = false;
		}

		m_pCamera[nCamIndex].Attach(m_tlFactory->CreateDevice(devices[nPosition - 1]));

		m_pCamera[nCamIndex].RegisterConfiguration(this, RegistrationMode_Append, Ownership_ExternalOwnership);

		m_pCamera[nCamIndex].RegisterImageEventHandler(this, RegistrationMode_Append, Ownership_ExternalOwnership);

		m_pCamera[nCamIndex].Open();

		m_pCameraNodeMap[nCamIndex] = &m_pCamera[nCamIndex].GetNodeMap();

		m_bCamOpenFlag[nCamIndex] = true;

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
int CCameraManager::Close_Camera(int nCamIndex)
{
	try
	{
		m_pCameraNodeMap[nCamIndex] = NULL;
		m_pCamera[nCamIndex].Close();
		m_pCamera[nCamIndex].DestroyDevice();
		m_pCamera[nCamIndex].DetachDevice();
		m_bCamOpenFlag[nCamIndex] = false;
		m_bCamConnectFlag[nCamIndex] = false;

		return 0;
	}
	catch (GenICam::GenericException& e)
	{
		// Error handling
		CString error = (CString)e.GetDescription();
		if (bLogUse == true)
		{
			WriteLog(nCamIndex, _T("[ Close_Camera ]\n"), error);
		}
		return -2;
	}
}

int CCameraManager::Connect_Camera(int nCamIndex, int nOffsetX, int nOffsetY, int nWidth, int nHeight, CString strImgFormat)
{
	try
	{
		m_pCamera[nCamIndex].MaxNumBuffer = BUF_NUM;
		m_iCM_reSizeWidth[nCamIndex] = (((nWidth * 8) + 31) / 32 * 4);

		// AOI 설정 
		int nTemp;
		GetIntegerMax(nCamIndex, &nTemp, "Width");

		if (nWidth > nTemp)
		{
			SetInteger(nCamIndex, nOffsetX, "OffsetX");
			SetInteger(nCamIndex, nWidth, "Width");
			m_iCM_OffsetX[nCamIndex] = nOffsetX;
			m_iCM_Width[nCamIndex] = nWidth;
		}
		else
		{
			SetInteger(nCamIndex, nWidth, "Width");
			m_iCM_Width[nCamIndex] = nWidth;
			SetInteger(nCamIndex, nOffsetX, "OffsetX");
			m_iCM_OffsetX[nCamIndex] = nOffsetX;
		}

		GetIntegerMax(nCamIndex, &nTemp, "Height");
		if (nHeight > nTemp)
		{
			SetInteger(nCamIndex, nOffsetY, "OffsetY");
			m_iCM_OffsetY[nCamIndex] = nOffsetY;
			SetInteger(nCamIndex, nHeight, "Height");
			m_iCM_Height[nCamIndex] = nHeight;
		}
		else
		{
			SetInteger(nCamIndex, nHeight, "Height");
			m_iCM_Height[nCamIndex] = nHeight;
			SetInteger(nCamIndex, nOffsetY, "OffsetY");
			m_iCM_OffsetY[nCamIndex] = nOffsetY;
		}

		// Image Format 설정  
		SetEnumeration(nCamIndex, CT2A(strImgFormat), "PixelFormat");
		m_strCM_ImageForamt[nCamIndex] = strImgFormat;

		m_bCamConnectFlag[nCamIndex] = true;

		return 0;
	}
	catch (GenICam::GenericException& e)
	{
		// Error handling
		CString error = (CString)e.GetDescription();
		if (bLogUse == true)
		{
			WriteLog(nCamIndex, _T("[ Connect_Camera ]\n"), error);
		}
		return -2;
	}
}

int CCameraManager::SingleGrab(int nCamIndex)
{
	try
	{
		m_pCamera[nCamIndex].StartGrabbing(1, GrabStrategy_OneByOne, GrabLoop_ProvidedByInstantCamera);
		return 0;
	}
	catch (GenICam::GenericException& e)
	{
		// Error handling
		CString error = (CString)e.GetDescription();
		if (bLogUse == true)
		{
			WriteLog(nCamIndex, _T("[ SingleGrab ]\n"), error);
		}
		return -2;
	}
}

int CCameraManager::GrabLive(int nCamIndex, int nMode)
{
	try
	{
		bStopFlag = true;
		bStopLiveThread = true;
		if (nMode == 0)
		{
			m_pCamera[nCamIndex].StartGrabbing(GrabStrategy_OneByOne, GrabLoop_ProvidedByInstantCamera);
		}
		else
		{
			m_pCamera.StartGrabbing(GrabStrategy_OneByOne, GrabLoop_ProvidedByInstantCamera);
		}
		return 0;
	}
	catch (GenICam::GenericException& e)
	{
		// Error handling
		CString error = (CString)e.GetDescription();
		if (bLogUse == true)
		{
			WriteLog(nCamIndex, _T("[ GrabLive ]\n"), error);
		}
		return -2;
	}
}

UINT CCameraManager::LiveThread(void* lParam)
{
	CCameraManager* pDlg = (CCameraManager*)lParam;

	CGrabResultPtr ptrGrabResult;
	while (pDlg->bStopLiveThread)
	{

		pDlg->m_pCamera.RetrieveResult(5000, ptrGrabResult, TimeoutHandling_ThrowException);
		if (ptrGrabResult->GrabSucceeded())
		{
			if (pDlg->m_strCM_ImageForamt[ptrGrabResult->GetCameraContext()] == "Mono8")
			{
				pDlg->converter[ptrGrabResult->GetCameraContext()].OutputPixelFormat = PixelType_Mono8;
				pDlg->converter[ptrGrabResult->GetCameraContext()].Convert(pDlg->Image[ptrGrabResult->GetCameraContext()], ptrGrabResult);
				pDlg->pImage8Buffer[ptrGrabResult->GetCameraContext()] = (unsigned char*)pDlg->Image[ptrGrabResult->GetCameraContext()].GetBuffer();
			}
		}
		pDlg->m_bCaptureEnd[ptrGrabResult->GetCameraContext()] = true;
		if (pDlg->bStopFlag == false)
		{
			pDlg->bStopLiveThread = false;
			pDlg->m_pCamera[0].StopGrabbing();
		}
	}

	return 0;
}

int CCameraManager::LiveStop(int nCamIndex, int nMode)
{
	try
	{
		bStopFlag = false;
		if (nMode == 0)
		{
			m_pCamera[nCamIndex].StopGrabbing();
		}
		else
		{
			m_pCamera.StopGrabbing();
		}
		return 0;
	}
	catch (GenICam::GenericException& e)
	{
		// Error handling
		CString error = (CString)e.GetDescription();
		if (bLogUse == true)
		{
			WriteLog(nCamIndex, _T("[ LiveStop ]\n"), error);
		}
		return -2;
	}
}

void CCameraManager::WriteLog(int nCamIdex, CString strTemp1, CString strTemp2)
{
	CTime t = CTime::GetCurrentTime();
	Hour = t.GetHour();
	Min = t.GetMinute();
	Sec = t.GetSecond();
	fprintf(log, "---------------------------------------------------------------------------\n");
	fprintf(log, "[%d시_%d분_%d초] [ CAM : %d ] Error \n", Hour, Min, Sec, nCamIdex);
	fprintf(log, "---------------------------------------------------------------------------\n");
	fprintf(log, "[Error position]\n");
	fprintf(log, CT2A(strTemp1));
	fprintf(log, "---------------------------------------------------------------------------\n");
	fprintf(log, "[Error Detail]\n");
	fprintf(log, CT2A(strTemp2));
	fprintf(log, "\n---------------------------------------------------------------------------\n");
}

bool CCameraManager::CheckCaptureEnd(int nCamIndex)
{
	return m_bCaptureEnd[nCamIndex];
}

void CCameraManager::ReadEnd(int nCamIndex)
{
	m_bCaptureEnd[nCamIndex] = false;
}

int CCameraManager::GetEnumeration(int nCamIndex, char* szValue, char* szNodeName)
{
	try
	{
		if (m_pCameraNodeMap[nCamIndex])
		{
			ptrEnumeration[nCamIndex] = m_pCameraNodeMap[nCamIndex]->GetNode(szNodeName);
			if (IsAvailable(ptrEnumeration[nCamIndex])) {
				strcpy_s(szValue, ptrEnumeration[nCamIndex]->ToString().length() + 1, ptrEnumeration[nCamIndex]->ToString());
				return 0;
			}
			else {
				CString str;
				str.Format(_T("Camera%d, It is not available to get %s\n"), nCamIndex, (CString)szNodeName);
				AfxMessageBox(str);
				return 1;
			}
		}
		else
		{
			return -1;
		}
	}

	catch (GenICam::GenericException& e)
	{
		// Error handling
		CString error = (CString)e.GetDescription();
		if (bLogUse == true)
		{
			WriteLog(nCamIndex, _T("[ GetEnumeration ]\n"), error);
		}
		return -2;
	}
}
int CCameraManager::SetEnumeration(int nCamIndex, char* szValue, char* szNodeName)
{
	try
	{
		if (m_pCameraNodeMap[nCamIndex])
		{
			ptrEnumeration[nCamIndex] = m_pCameraNodeMap[nCamIndex]->GetNode(szNodeName);
			if (IsAvailable(ptrEnumeration[nCamIndex])) {
				ptrEnumeration[nCamIndex]->FromString(szValue);
				return 0;
			}
			else {
				CString str;
				str.Format(_T("Camera%d, It is not available to set %s\n"), nCamIndex, (CString)szNodeName);
				AfxMessageBox(str);
				return 1;
			}
		}
		else
		{
			return -1;
		}
	}
	catch (GenICam::GenericException& e)
	{
		// Error handling
		CString error = (CString)e.GetDescription();
		if (bLogUse == true)
		{
			WriteLog(nCamIndex, _T("[ SetEnumeration ]\n"), error);
		}
		return -2;
	}
}

int CCameraManager::GetInteger(int nCamIndex, int* nValue, char* szNodeName)
{
	try
	{
		if (m_pCameraNodeMap[nCamIndex])
		{
			ptrInteger[nCamIndex] = m_pCameraNodeMap[nCamIndex]->GetNode(szNodeName);
			if (IsAvailable(ptrInteger[nCamIndex])) {
				*nValue = (int)ptrInteger[nCamIndex]->GetValue();
				return 0;
			}
			else {
				CString str;
				str.Format(_T("Camera%d, It is not available to get %s\n"), nCamIndex, (CString)szNodeName);
				AfxMessageBox(str);
				return 1;
			}
		}
		else
		{
			return -1;
		}
	}
	catch (GenICam::GenericException& e)
	{
		// Error handling
		CString error = (CString)e.GetDescription();
		if (bLogUse == true)
		{
			WriteLog(nCamIndex, _T("[ GetInteger ]\n"), error);
		}
		return -2;
	}
}
int CCameraManager::GetIntegerMax(int nCamIndex, int* nValue, char* szNodeName)
{
	try
	{
		if (m_pCameraNodeMap[nCamIndex])
		{
			ptrInteger[nCamIndex] = m_pCameraNodeMap[nCamIndex]->GetNode(szNodeName);
			if (IsAvailable(ptrInteger[nCamIndex])) {
				*nValue = (int)ptrInteger[nCamIndex]->GetMax();
				return 0;
			}
			else {
				CString str;
				str.Format(_T("Camera%d, It is not available to get %s\n"), nCamIndex, (CString)szNodeName);
				AfxMessageBox(str);
				return 1;
			}
		}
		else
		{
			return -1;
		}
	}
	catch (GenICam::GenericException& e)
	{
		// Error handling
		CString error = (CString)e.GetDescription();
		if (bLogUse == true)
		{
			WriteLog(nCamIndex, _T("[ GetIntegerMax ]\n"), error);
		}
		return -2;
	}
}
int CCameraManager::GetIntegerMin(int nCamIndex, int* nValue, char* szNodeName)
{
	try
	{
		if (m_pCameraNodeMap[nCamIndex])
		{
			ptrInteger[nCamIndex] = m_pCameraNodeMap[nCamIndex]->GetNode(szNodeName);
			if (IsAvailable(ptrInteger[nCamIndex])) {
				*nValue = (int)ptrInteger[nCamIndex]->GetMin();
				return 0;
			}
			else {
				CString str;
				str.Format(_T("Camera%d, It is not available to get %s\n"), nCamIndex, (CString)szNodeName);
				AfxMessageBox(str);
				return 1;
			}
		}
		else
		{
			return -1;
		}
	}
	catch (GenICam::GenericException& e)
	{
		// Error handling
		CString error = (CString)e.GetDescription();
		if (bLogUse == true)
		{
			WriteLog(nCamIndex, _T("[ GetIntegerMin ]\n"), error);
		}
		return -2;
	}
}
int CCameraManager::SetInteger(int nCamIndex, int nValue, char* szNodeName)
{
	try
	{
		if (m_pCameraNodeMap[nCamIndex])
		{
			ptrInteger[nCamIndex] = m_pCameraNodeMap[nCamIndex]->GetNode(szNodeName);
			if (IsAvailable(ptrInteger[nCamIndex])) {
				ptrInteger[nCamIndex]->SetValue(nValue);
				return 0;
			}
			else {
				CString str;
				str.Format(_T("Camera%d, It is not available to set %s\n"), nCamIndex, (CString)szNodeName);
				AfxMessageBox(str);
				return 1;
			}
		}
		else
		{
			return -1;
		}
	}
	catch (GenICam::GenericException& e)
	{
		// Error handling
		CString error = (CString)e.GetDescription();
		if (bLogUse == true)
		{
			WriteLog(nCamIndex, _T("[ SetInteger ]\n"), error);
		}
		return -2;
	}
}

int CCameraManager::GetBoolean(int nCamIndex, bool* bValue, char* szNodeName)
{
	try
	{
		if (m_pCameraNodeMap[nCamIndex])
		{
			ptrBoolean[nCamIndex] = m_pCameraNodeMap[nCamIndex]->GetNode(szNodeName);
			if (IsAvailable(ptrInteger[nCamIndex])) {
				*bValue = ptrBoolean[nCamIndex]->GetValue();
				return 0;
			}
			else {
				CString str;
				str.Format(_T("Camera%d, It is not available to get %s\n"), nCamIndex, (CString)szNodeName);
				AfxMessageBox(str);
				return 1;
			}

		}
		else
		{
			return -1;
		}
	}
	catch (GenICam::GenericException& e)
	{
		// Error handling
		CString error = (CString)e.GetDescription();
		if (bLogUse == true)
		{
			WriteLog(nCamIndex, _T("[ GetBoolean ]\n"), error);
		}
		return -2;
	}
}
int CCameraManager::SetBoolean(int nCamIndex, bool bValue, char* szNodeName)
{
	try
	{
		if (m_pCameraNodeMap[nCamIndex])
		{
			ptrBoolean[nCamIndex] = m_pCameraNodeMap[nCamIndex]->GetNode(szNodeName);
			if (IsAvailable(ptrInteger[nCamIndex])) {
				ptrBoolean[nCamIndex]->SetValue(bValue);
				return 0;
			}
			else {
				CString str;
				str.Format(_T("Camera%d, It is not available to set %s\n"), nCamIndex, (CString)szNodeName);
				AfxMessageBox(str);
				return 1;
			}
		}
		else
		{
			return -1;
		}
	}
	catch (GenICam::GenericException& e)
	{
		// Error handling
		CString error = (CString)e.GetDescription();
		if (bLogUse == true)
		{
			WriteLog(nCamIndex, _T("[ SetBoolean ]\n"), error);
		}
		return -2;
	}
}
int CCameraManager::GetFloat(int nCamIndex, float* fValue, char* szNodeName)
{
	try
	{
		if (m_pCameraNodeMap[nCamIndex])
		{
			ptrFloat[nCamIndex] = m_pCameraNodeMap[nCamIndex]->GetNode(szNodeName);
			if (IsAvailable(ptrInteger[nCamIndex])) {
				*fValue = (float)ptrFloat[nCamIndex]->GetValue();
				return 0;
			}
			else {
				CString str;
				str.Format(_T("Camera%d, It is not available to get %s\n"), nCamIndex, (CString)szNodeName);
				AfxMessageBox(str);
				return 1;
			}
		}
		else
		{
			return -1;
		}
	}
	catch (GenICam::GenericException& e)
	{
		// Error handling
		CString error = (CString)e.GetDescription();
		if (bLogUse == true)
		{
			WriteLog(nCamIndex, _T("[ GetFloat ]\n"), error);
		}
		return -2;
	}
}
int CCameraManager::SetFloat(int nCamIndex, float fValue, char* szNodeName)
{
	try
	{
		if (m_pCameraNodeMap[nCamIndex])
		{
			ptrFloat[nCamIndex] = m_pCameraNodeMap[nCamIndex]->GetNode(szNodeName);
			if (IsAvailable(ptrInteger[nCamIndex])) {
				ptrFloat[nCamIndex]->SetValue(fValue);
				return 0;
			}
			else {
				CString str;
				str.Format(_T("Camera%d, It is not available to set %s\n"), nCamIndex, (CString)szNodeName);
				AfxMessageBox(str);
				return 1;
			}
		}
		else
		{
			return -1;
		}
	}
	catch (GenICam::GenericException& e)
	{
		// Error handling
		CString error = (CString)e.GetDescription();
		if (bLogUse == true)
		{
			WriteLog(nCamIndex, _T("[ SetFloat ]\n"), error);
		}
		return -2;
	}
}
int CCameraManager::SetCommand(int nCamIndex, char* szNodeName)
{
	try
	{
		if (m_pCameraNodeMap[nCamIndex])
		{
			ptrCommand[nCamIndex] = m_pCameraNodeMap[nCamIndex]->GetNode(szNodeName);
			if (IsAvailable(ptrInteger[nCamIndex])) {
				ptrCommand[nCamIndex]->Execute();
				return 0;
			}
			else {
				CString str;
				str.Format(_T("Camera%d, It is not available to send cammand %s\n"), nCamIndex, (CString)szNodeName);
				AfxMessageBox(str);
				return 1;
			}
		}
		else
		{
			return -1;
		}
	}
	catch (GenICam::GenericException& e)
	{
		// Error handling
		CString error = (CString)e.GetDescription();
		if (bLogUse == true)
		{
			WriteLog(nCamIndex, _T("[ SetCommand ]\n"), error);
		}
		return -2;
	}
}
int CCameraManager::SaveImage(int nFileFormat, unsigned char* pImage, char* filename, int nPixelType, int width, int height, int nColorband)
{
	EImageFileFormat ImageFileFormat;
	switch (nFileFormat)
	{
	case 0:
		ImageFileFormat = ImageFileFormat_Bmp;
		break;
	case 1:
		ImageFileFormat = ImageFileFormat_Tiff;
		break;
	case 2:
		ImageFileFormat = ImageFileFormat_Jpeg;
		break;
	case 3:
		ImageFileFormat = ImageFileFormat_Png;
		break;
	}
	EPixelType  ImagePixleType;
	switch (nPixelType)
	{
	case 0:
		ImagePixleType = PixelType_Mono8;
		break;
	case 1:
		ImagePixleType = PixelType_BGR8packed;
		break;

	}

	CImagePersistence::Save(ImageFileFormat, filename, pImage, width * height * nColorband, ImagePixleType, width, height, 0, ImageOrientation_TopDown);
	return 0;
}
void CCameraManager::OnImagesSkipped(CInstantCamera& camera, size_t countOfSkippedImages)
{

	m_iSkippiedFrame[CInstantCamera().GetCameraContext()] = countOfSkippedImages;
	if (bLogUse == true)
	{
		fprintf(log, "CAM = %d , SkippedFrame = %d\n", CInstantCamera().GetCameraContext(), m_iSkippiedFrame[CInstantCamera().GetCameraContext()]);

	}
}
void CCameraManager::OnImageGrabbed(CInstantCamera& camera, const CGrabResultPtr& ptrGrabResult)
{
	try
	{

		if (ptrGrabResult->GrabSucceeded())
		{
			if (bLogUse == true)
			{

				fprintf(log, "CAM %d Grab Finishd. Frame number: %d\n", ptrGrabResult->GetCameraContext(), ptrGrabResult->GetImageNumber());

			}
			int nCameraIndex = ptrGrabResult->GetCameraContext();

			if (m_strCM_ImageForamt[ptrGrabResult->GetCameraContext()] == "Mono8")
			{

				converter[ptrGrabResult->GetCameraContext()].OutputPixelFormat = PixelType_Mono8;

				converter[ptrGrabResult->GetCameraContext()].Convert(Image[ptrGrabResult->GetCameraContext()], ptrGrabResult);
				pImage8Buffer[ptrGrabResult->GetCameraContext()] = (unsigned char*)Image[ptrGrabResult->GetCameraContext()].GetBuffer();

			}
			
			m_bCaptureEnd[ptrGrabResult->GetCameraContext()] = true;
			m_iGrabbedFrame[ptrGrabResult->GetCameraContext()] = ptrGrabResult->GetImageNumber();
		}
		else // grab failed
		{
			int k = ptrGrabResult->GetErrorCode();
			CString error = (CString)ptrGrabResult->GetErrorDescription();
			CString t;
			t.Format(_T("lost Frame number=%d"), k);
			error += t;
			if (bLogUse == true)
			{
				WriteLog(ptrGrabResult->GetCameraContext(), _T("[ OnImageGrabbed ]\n"), error);
			}
		}
	}
	catch (GenICam::GenericException& e)
	{

		// Error handling
		CString error = (CString)e.GetDescription();
		if (bLogUse == true)
		{
			WriteLog(ptrGrabResult->GetCameraContext(), _T("[ OnImageGrabbed ]\n"), error);
		}
	}
}
void CCameraManager::OnCameraDeviceRemoved(CInstantCamera& camera)
{
	CTime t = CTime::GetCurrentTime();
	Hour = t.GetHour();
	Min = t.GetMinute();
	Sec = t.GetSecond();

	if ((CString)m_pCamera[0].GetDeviceInfo().GetSerialNumber() == (CString)camera.GetDeviceInfo().GetSerialNumber())
	{
		m_bRemoveCamera[0] = true;
		if (bLogUse == true)
		{
			fprintf(log, "[%d시_%d분_%d초] [OnCameraDeviceRemoved::  Camera 0 removed ]\n", Hour, Min, Sec);
		}
		Close_Camera(0);
	}
	else if ((CString)m_pCamera[1].GetDeviceInfo().GetSerialNumber() == (CString)camera.GetDeviceInfo().GetSerialNumber())
	{

		m_bRemoveCamera[1] = true;
		if (bLogUse == true)
		{
			fprintf(log, "[%d시_%d분_%d초] [OnCameraDeviceRemoved::  Camera 1 removed ]\n", Hour, Min, Sec);
		}
		Close_Camera(1);
	}

}
