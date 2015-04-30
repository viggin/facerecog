//#include "../tools/tools.h"
#include "stdafx.h"
#include "../FaceMngr/FaceMngr.h"

/*
	OpenCV2.0摄像头操作的封装
	用法：用Init初始化摄像头序号和是否左右镜像，用GetFrame获取当前帧，自动Release
	若两线程获取同一摄像头数据，最好使用一个本类实例，使用GetFrame时传进来一个初始化好的IplImage
	这样，GetFrame将会将图像复制到IplImage，以防多线程冲突
*/
class CWebCam
{
public:
	CWebCam(void);
	~CWebCam(void);

private:
	CvCapture	*m_capture;
	int			m_interval;
	bool		m_bStarted, m_bMirrorLike;
	double		m_fLastFrameTime, m_fFrameFreq;
	CWinThread	*m_thrdShow;
	HWND		prtHwnd;
	HANDLE		m_hEvntPause;
	CCriticalSection	m_csCap;

public:
	bool Init(int camIndex = 0, bool bMirrorLike = true);
	void Release();

	IplImage	*GetFrame(IplImage *frame = NULL); // 若frame=NULL,则返回内置的IplImage指针
	double		GetRealFrameFreq(); // 前后两次GetFrame的间隔的倒数
	CvSize		GetFrameSize();

	// 用于单开一个线程，以周期interval将摄像头捕捉到的画面显示到parentHwnd
	void		RealtimeShow(int interval = 0, HWND parentHwnd = NULL);
	void		RealtimePause(bool pause);
	void		RealtimeShut();

	UINT RunShow();
};

UINT WebCamShowProc(LPVOID pParam); // CWebCam所用的形式线程函数