#include "stdafx.h"
#include "WebCam.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


CWebCam::CWebCam(void)
{
	m_capture		= 0;
	m_bStarted		= true;
	m_interval		= 0;
	m_bMirrorLike	= true;
	m_thrdShow		= NULL;
	prtHwnd			= NULL;
	m_hEvntPause	= ::CreateEvent(NULL, TRUE, TRUE, NULL);
}

CWebCam::~CWebCam(void)
{
	Release();
}

bool CWebCam::Init( int camIndex /*= 0*/, bool bMirrorLike /*= true*/ )
{
	m_thrdShow = NULL;
	m_bMirrorLike = bMirrorLike;
	m_capture = cvCaptureFromCAM(camIndex); // must be released!
	m_fLastFrameTime = (double)cvGetTickCount();
	//if (! m_capture)
		//::AfxMessageBox("Webcam not found.");
	return (m_capture != 0);
}

void CWebCam::Release()
{
	m_bStarted = false;
	if (m_thrdShow)
		::WaitForSingleObject(m_thrdShow, 1000);
	cvReleaseCapture(&m_capture);
}

IplImage * CWebCam::GetFrame(IplImage *frame/* = NULL*/)
{
	assert(m_capture);
	m_csCap.Lock(); // 两个线程同时获取图像时，防止冲突
	IplImage *frame0 = cvQueryFrame(m_capture);
	if (frame) cvCopy(frame0, frame);
	else frame = frame0;
	m_csCap.Unlock();

	if(frame->origin == IPL_ORIGIN_BL) cvFlip(frame, frame, 0);
	if(m_bMirrorLike) cvFlip(frame, frame, 1);
	double t = (double)cvGetTickCount();
	m_fFrameFreq = (double)cvGetTickFrequency()*1e6/(t-m_fLastFrameTime);
	m_fLastFrameTime = t;

	return frame;
}

void CWebCam::RealtimeShow( int interval /*=0*/, HWND parentHwnd  /*= NULL*/ )
{
	m_interval = interval;
	m_bStarted = true;
	m_thrdShow = ::AfxBeginThread(WebCamShowProc, this);
}

UINT CWebCam::RunShow()
{
	IplImage *frame = cvCreateImage(GetFrameSize(), IPL_DEPTH_8U, 3);
	cvNamedWindow("webcam", 0);

	if (prtHwnd)
	{
		HWND hWnd = (HWND) cvGetWindowHandle("webcam");
		HWND hParent = ::GetParent(hWnd);
		::SetParent(hWnd, prtHwnd); // 嵌入到一个窗口prtHwnd
		::ShowWindow(hParent, SW_HIDE);
	}

	while(m_bStarted)
	{
		::WaitForSingleObject(m_hEvntPause, 1000);
		GetFrame(frame);
		cvShowImage("webcam", frame);
		cvWaitKey(1);
		Sleep(m_interval);
	}

	cvDestroyWindow("webcam");
	cvReleaseImage(&frame);
	return 0;
}

void CWebCam::RealtimePause( bool pause )
{
	if (pause) 
		::ResetEvent(m_hEvntPause);
	else 
		::SetEvent(m_hEvntPause);
}

void CWebCam::RealtimeShut()
{
	m_bStarted = false;
	::WaitForSingleObject(m_thrdShow, 1000);
}

double CWebCam::GetRealFrameFreq()
{
	return m_fFrameFreq;
}

CvSize CWebCam::GetFrameSize()
{
	assert(m_capture);
	CvSize sz;
	sz.width = (int)cvGetCaptureProperty(m_capture, CV_CAP_PROP_FRAME_WIDTH);
	sz.height = (int)cvGetCaptureProperty(m_capture, CV_CAP_PROP_FRAME_HEIGHT);
	return sz;
}

UINT WebCamShowProc( LPVOID pParam )
{
	CWebCam *wc = (CWebCam *)pParam;
	return wc->RunShow();
}
