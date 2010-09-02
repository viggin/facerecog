
// frtestDlg.h : 头文件
//

#pragma once

#include "Resource.h"
#include "EnrollDlg.h"
#include "TrainDlg.h"
#include "RecogDlg.h"
#include "../FaceMngr/FaceMngr.h"

#pragma comment(linker, "/NODEFAULTLIB:libcmt.lib")


// CfrtestDlg 对话框，主界面
class CfrtestDlg : public CDialog
{
// 构造
public:
	CfrtestDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_FRTEST_DIALOG };

	protected:
	 void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

	//////////////////////////////////////////////////////////////////////////
	// User defined
public:
	bool InitSys();
	UINT ShowFrame();
	void ReadIni();


	CWinThread	*m_thrdFrame;
	bool		m_bFrameStarted;
	CEnrollDlg	m_dlgEnroll;
	CTrainDlg	m_dlgTrain;
	CRecogDlg	m_dlgRecog;
	IplImage	*m_frame; // 存储摄像头拍到的画面


// 实现
protected:
	HICON m_hIcon;
	// 生成的消息映射函数
	 BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedEnroll();
	afx_msg void OnBnClickedTrain();
	afx_msg void OnBnClickedRecog();
	afx_msg void OnBnClickedExport();
};

UINT ShowFrameProc(LPVOID pParam);
UINT InitMngrProc(LPVOID pParam);