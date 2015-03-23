#pragma once


// CTrainDlg 对话框，训练界面

class CTrainDlg : public CDialog
{
	DECLARE_DYNAMIC(CTrainDlg)

public:
	CTrainDlg(CWnd* pParent = NULL);   // 标准构造函数
	 ~CTrainDlg();

// 对话框数据
	enum { IDD = IDD_TRAIN };

protected:
	 void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	BOOL m_bHasNormed;
	BOOL m_bSave2Model;
	CString m_strTrainFd;
	CString m_strTrainFilelist;

	BOOL OnInitDialog();
	afx_msg void OnBnClickedBrowseTrainfd();
	afx_msg void OnBnClickedImportMat();
	afx_msg void OnBnClickedFromlist();
};
