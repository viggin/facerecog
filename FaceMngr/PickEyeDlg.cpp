#include "PickEyeDlg.h"


bool CPickEyeDlg::Pick( IplImage * pic )
{
	m_pic = pic;
	m_haveNow = 0;
	Show();

	while(1)
	{
		int key = cvWaitKey();
		if (key == VK_ESCAPE)
		{
			m_haveNow--;
			if (m_haveNow == -1)
			{
				//cvDestroyAllWindows();
				cvDestroyWindow(m_title.GetBuffer());
				Clear();
				return false;
			}
			else
				Show();
		}
		else if (key == VK_RETURN)
		{
			if (m_haveNow == 2)
			{
				//cvDestroyAllWindows();
				cvDestroyWindow(m_title.GetBuffer());
				return true;
			}
		}
	}
}

void CPickEyeDlg::Show()
{
	m_title = "right eye first"; // ask the user to select the eye in the left of the picture (the person's right eye) first
	cvNamedWindow(m_title.GetBuffer(), CV_WINDOW_NORMAL|CV_WINDOW_KEEPRATIO);
	cvResizeWindow(m_title.GetBuffer(), m_pic->width, m_pic->height);
	cvSetMouseCallback(m_title.GetBuffer(), on_mouse, this);

	cvReleaseImage(&m_picCopy);
	m_picCopy = cvCloneImage(m_pic);

	int s = 3;
	CvScalar color = CV_RGB(255,255,255);
	if (m_haveNow >= 1)
	{
		cvLine(m_picCopy, cvPoint(m_leftEye.x-s,m_leftEye.y), cvPoint(m_leftEye.x+s,m_leftEye.y), color);
		cvLine(m_picCopy, cvPoint(m_leftEye.x,m_leftEye.y-s), cvPoint(m_leftEye.x,m_leftEye.y+s), color);
	}
	if (m_haveNow == 2)
	{
		cvLine(m_picCopy, cvPoint(m_rightEye.x-s,m_rightEye.y), cvPoint(m_rightEye.x+s,m_rightEye.y), color);
		cvLine(m_picCopy, cvPoint(m_rightEye.x,m_rightEye.y-s), cvPoint(m_rightEye.x,m_rightEye.y+s), color);
	}
	cvShowImage(m_title.GetBuffer(), m_picCopy);
}

void CPickEyeDlg::Add( int x, int y )
{
	if (m_haveNow == 2)
	{
		return;
	}
	else
	{
		if (m_haveNow == 0)
		{
			m_leftEye.x = x;
			m_leftEye.y = y;
		}
		else
		{
			m_rightEye.x = x;
			m_rightEye.y = y;
		}
		m_haveNow++;
		Show();
	}
}

void CPickEyeDlg::Clear()
{
	m_leftEye = cvPoint(0,0);
	m_rightEye = cvPoint(0,0);
}

void on_mouse( int event, int x, int y, int flags, void* param )
{
	CPickEyeDlg *pInst = (CPickEyeDlg *)param;
	if (event == CV_EVENT_LBUTTONDOWN)
	{
		pInst->Add(x,y);
	}
}