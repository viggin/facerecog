// TrainDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "frtest.h"
#include "TrainDlg.h"
#include "ProjDefs.h"
#include "WndUtils.h"

// CTrainDlg 对话框

IMPLEMENT_DYNAMIC(CTrainDlg, CDialog)

CTrainDlg::CTrainDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CTrainDlg::IDD, pParent)
	, m_bHasNormed(TRUE)
	, m_bSave2Model(TRUE)
{
}

CTrainDlg::~CTrainDlg()
{
}

void CTrainDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_NORMED, m_bHasNormed);
	DDX_Check(pDX, IDC_SAVE2MODEL, m_bSave2Model);
	DDX_Text(pDX, IDC_TRAINFD, m_strTrainFd);
}


BEGIN_MESSAGE_MAP(CTrainDlg, CDialog)
	ON_BN_CLICKED(IDC_BROWSE_TRAINFD, &CTrainDlg::OnBnClickedBrowseTrainfd)
	ON_BN_CLICKED(IDC_IMPORT_MAT, &CTrainDlg::OnBnClickedImportMat)
END_MESSAGE_MAP()

BOOL CTrainDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	return TRUE;
}

// CTrainDlg 消息处理程序

void CTrainDlg::OnBnClickedBrowseTrainfd()
{
	// TODO: 在此添加控件通知处理程序代码
	//char str[501];
	//::GetCurrentDirectory(500, str);
	//m_strTrainFd = m_strTrainFd;
	bool ret = SelDirectory(NULL, "选择训练图片所在文件夹", m_strTrainFd);
	if (!ret) return;
	UpdateData(FALSE);
}

void CTrainDlg::OnBnClickedImportMat()
{
	// TODO: 在此添加控件通知处理程序代码
	CFileDialog dlgFile(TRUE, "frmat", "test", 
		OFN_NOCHANGEDIR | OFN_HIDEREADONLY, 
		"人脸识别矩阵文件 (*.frmat)|*.frmat||");
	if (dlgFile.DoModal() == IDOK)
	{
		CString path = dlgFile.GetPathName();
		setlocale(LC_ALL, "Chinese-simplified"); //设置中文环境
		ifstream is(path, ios::binary);
		if (!is)
		{
			::AfxMessageBox(CString(path)+"：无法读取。");
			return;
		}

		if (g_faceMngr->ReadMatFromFile(is))
			::AfxMessageBox("导入完成。\n请注意模板和子空间降维矩阵的对应。", MB_OK | MB_ICONINFORMATION);

		is.close();
		setlocale(LC_ALL, "C");// 还原
	}
	OnCancel();
}
