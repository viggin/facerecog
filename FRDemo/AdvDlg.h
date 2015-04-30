#pragma once


// CAdvDlg 对话框，注册界面->高级

class CAdvDlg : public CDialog
{
	DECLARE_DYNAMIC(CAdvDlg)

public:
	CAdvDlg(CWnd* pParent = NULL);   // 标准构造函数
	 ~CAdvDlg();

// 对话框数据
	enum { IDD = IDD_ADV };

protected:
	 void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedClearLib();
	BOOL OnInitDialog();
};
