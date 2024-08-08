#pragma once

#include "pch.h"
#include "CameraManager.h"

#include <pylon/PylonIncludes.h>
#include <pylon/PylonGUI.h>
#include <pylon/gige/BaslerGigECamera.h>
#include <pylon/gige/GigETransportLayer.h>
#include <pylon/gige/BaslerGigEDeviceInfo.h>

using namespace Pylon;
using namespace GenApi;

#define BUF_NUM   3

class CCameraManager : public CImageEventHandler,
    public CConfigurationEventHandler
{

public:
    CCameraManager();
    ~CCameraManager();

    // 카메라 관련 변수
    CTlFactory* m_tlFactory;
    CInstantCamera m_Camera;
    CPylonImage Image;
    CImageFormatConverter converter;
    CIntegerPtr m_pHeartbeatTimeout;
    IGigETransportLayer* m_pIP;

    INodeMap* m_pCameraNodeMap;
    DeviceInfoList_t devices;

    CEnumerationPtr	ptrEnumeration;
    CIntegerPtr ptrInteger;
    CBooleanPtr ptrBoolean;
    CFloatPtr ptrFloat;
    CCommandPtr ptrCommand;

    // 로그 변수
    bool  bLogUse;
    bool  bMessageBoxUse;
    bool  bTraceUse;
    bool  bStopLiveThread;
    bool  bStopFlag;
    FILE* log;
    char filename[256];
    time_t t;
    int Hour, Min, Sec;

    // 그 외
    int m_iCM_OffsetX;
    int m_iCM_OffsetY;
    int m_iCM_Width;
    int m_iCM_Height;
    int m_iCM_reSizeWidth;

    bool m_bCaptureEnd;

    bool m_bRemoveCamera;

    bool m_bCamOpenFlag;
    bool m_bCamConnectFlag;

    long m_iSkippiedFrame;
    long m_iGrabbedFrame;

    // 멤버 함수
    void WriteLog(CString strTemp, CString strTemp2);
    bool CheckCaptureEnd();
    void ReadEnd();

    int FindCamera(CString& szCamName, CString& szCamSerialNumber, CString& szInterfacName);
    int OpenCamera();
    //int ConnectCamera();
    //int CloseCamera();

    //int SingleGrab();
    //int GrabLive();
    //int LiveStop();
    //int SaveImage();

    //static UINT LiveThread(void* lParam);

    //virtual void OnImageGrabbed(CInstantCamera& camera, const CGrabResultPtr& ptrGrabResult);
    //virtual void OnImagesSkipped(CInstantCamera& camera, size_t countOfSkippedImages);
    //virtual void OnCameraDeviceRemoved(CInstantCamera& camera);
};