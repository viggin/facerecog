#pragma once

#include "../FaceMngr/FaceMngr.h"

// CRecogDlg 对话框，识别界面

class CRecogDlg : public CDialog
{
	DECLARE_DYNAMIC(CRecogDlg)

public:
	CRecogDlg(CWnd* pParent = NULL);   // 标准构造函数
	 ~CRecogDlg();

	//////////////////////////////////////////////////////////////////////////
	// user defined

	BOOL OnInitDialog();

	UINT ShowMatch();
	void ShowMatchRun();
	void InitWindows(); // 初始化OpenCV图像显示窗口，并嵌入对话框

	void Release();


	bool		m_bMatchStarted;
	bool		m_bFoundFace;

	CWinThread	*m_thrdMatch;

	IplImage	*m_frame;
	CvMat		*faceImg8;
	IplImage	*noface, *resss; // 这两个图像用于告诉用户一些信息

	CvSize		faceSz;
	SMatch		matchInfo;
	CString		strFd;

	BOOL		m_bUseBuf; // “稳定瞳孔定位”复选框
	CvSize		showSz;

// 对话框数据
	enum { IDD = IDD_RECOG };

protected:
	 void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedPicRecog();
	afx_msg void OnBnClickedUsebuf();
	//afx_msg void OnNMClickFilepath(NMHDR *pNMHDR, LRESULT *pResult);

};

UINT ShowMatchProc(LPVOID pParam);
