// AdvDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "FRDemo.h"
#include "AdvDlg.h"
#include "ProjDefs.h"
#include "Utils.h"

// CAdvDlg 对话框

IMPLEMENT_DYNAMIC(CAdvDlg, CDialog)

CAdvDlg::CAdvDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAdvDlg::IDD, pParent)
{

}

CAdvDlg::~CAdvDlg()
{
}

void CAdvDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CAdvDlg, CDialog)
	ON_BN_CLICKED(IDC_CLEAR_LIB, &CAdvDlg::OnBnClickedClearLib)
END_MESSAGE_MAP()


// CAdvDlg 消息处理程序

void CAdvDlg::OnBnClickedClearLib()
{
	// TODO: 在此添加控件通知处理程序代码
	g_faceMngr->ClearList();
	//g_lstName.clear();
	::AfxMessageBox("OK.", MB_OK | MB_ICONINFORMATION);
}

BOOL CAdvDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	GetDlgItem(IDCANCEL)->EnableWindow(FALSE);
	return TRUE;
}