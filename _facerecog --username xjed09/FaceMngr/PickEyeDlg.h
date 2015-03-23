/*
	show an OpenCV image window, allow user to pick two coordinates by mouse
	the user should select the eye in the left of the picture (the person's right eye) first
	press ESC to undo and ENTER to submit.
*/

#pragma once

#include "../tools/tools.h"

#pragma comment(lib, "../bin/tools.lib")


class __declspec(dllexport) CPickEyeDlg
{
public:
	CPickEyeDlg():m_haveNow(0),
		m_leftEye(cvPoint(0,0)),
		m_rightEye(cvPoint(0,0)),
		m_pic(NULL),
		m_picCopy(NULL){};
	~CPickEyeDlg()
	{ cvReleaseImage(&m_picCopy);};

public:
	CvPoint m_leftEye, m_rightEye;
	CString m_title;
private:
	int m_haveNow;
	IplImage *m_pic, *m_picCopy;

public:
	bool Pick( IplImage * pic );
	void Show();
	void Clear();
	void Add( int x, int y );
};

void on_mouse(int event, int x, int y, int flags, void* param);