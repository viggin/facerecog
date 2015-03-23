// EnrollDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "FRDemo.h"
#include "EnrollDlg.h"
#include "Utils.h"
#include "ProjDefs.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// CEnrollDlg 对话框

IMPLEMENT_DYNAMIC(CEnrollDlg, CDialog)

CEnrollDlg::CEnrollDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CEnrollDlg::IDD, pParent)
	, m_strName(_T(""))
	, m_bSaveSmallImg(true)
{
	m_thrdFace = NULL;
	m_noface = cvLoadImage("noface.bmp");
}

CEnrollDlg::~CEnrollDlg()
{
	cvReleaseImage(&m_noface);
}

void CEnrollDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_NAME, m_strName);
	//DDX_Text(pDX, IDC_CLASSID, m_strClassId);
	DDX_Check(pDX, IDC_SAVE_SMALL, m_bSaveSmallImg);
}

BOOL CEnrollDlg::OnInitDialog() // Excutes before each DoModal
{
	CDialog::OnInitDialog();

	faceSz = g_faceMngr->m_faceSz;
	showSz = faceSz;

	if (showSz.height != 160) // (160,130) is the preset show size, if faceSz != (160,130), then adjust showSz
		showSz *= (160.0/showSz.height);
	if (showSz.width > 130)
		showSz *= (130.0/showSz.width);

	EmbedCvWindow(GetDlgItem(IDC_FACE)->m_hWnd, "face", showSz.width, showSz.height);
	if (m_noface) cvShowImage("face", m_noface);
	RECT rc;
	GetWindowRect(&rc);
	SetWindowPos(NULL, rc.left + 15, rc.top + 300, 0,0, SWP_NOSIZE | SWP_NOZORDER);

	m_nShootNum = 0;

	m_strName = "";
	UpdateData(FALSE);
	CWnd *pBtn = GetDlgItem(IDC_SHOOT);
	RECT rc1;
	pBtn->GetClientRect(&rc1);
	pBtn->SetWindowText("拍摄");
	pBtn->SetWindowPos(NULL, 0,0, 35, rc1.bottom-rc1.top, SWP_NOMOVE | SWP_NOZORDER);
	GetDlgItem(IDC_DEL_SHOOT)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_ENROLL_PIC)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_IMPORT_MODELS)->ShowWindow(SW_SHOW);
	if (g_bHasWebcam)
	{
		m_frame = cvCreateImage(g_webcam.GetFrameSize(), IPL_DEPTH_8U, 3);
		SetDlgItemText(IDC_SHOOT_GUIDE, 
			"若要从摄像头注册，请等画面较正时点“拍摄”。\n"
			"如果画面定住,说明人脸检测及对准未成功，请改善姿势、光照。\n"
			"为保证识别率，建议以正常表情拍摄3~4张。");
		m_bFaceStarted = true;
		m_thrdFace = ::AfxBeginThread(ShowFaceProc, this);
	}
	else
	{
		m_frame = NULL;
		SetDlgItemText(IDC_SHOOT_GUIDE, "");
		CWnd *pWnd = GetDlgItem(IDC_SHOOT);
		pWnd->EnableWindow(FALSE);
	}
	
	return TRUE;
}

BEGIN_MESSAGE_MAP(CEnrollDlg, CDialog)
	ON_BN_CLICKED(IDC_SHOOT, &CEnrollDlg::OnBnClickedShoot)
	ON_BN_CLICKED(IDCANCEL, &CEnrollDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDOK, &CEnrollDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_ENROLL_PIC, &CEnrollDlg::OnBnClickedEnrollPic)
	ON_BN_CLICKED(IDC_IMPORT_MODELS, &CEnrollDlg::OnBnClickedImportModels)
	ON_BN_CLICKED(IDC_ADV, &CEnrollDlg::OnBnClickedAdv)
	ON_BN_CLICKED(IDC_DEL_SHOOT, &CEnrollDlg::OnBnClickedDelShoot)
END_MESSAGE_MAP()


// CEnrollDlg 消息处理程序

void CEnrollDlg::OnBnClickedShoot()
{
	// TODO: 在此添加控件通知处理程序代码
	if (!m_bFoundFace) return;

	SFacePose fp = g_faceMngr->f.GetFacePose();
	CvRect rc = fp.rc;
	float eyeDis = fp.eyeDis;
	CvSize fsz = g_webcam.GetFrameSize();

	// 确保注册时脸不要太偏，大小合适
	if (rc.x < fsz.width * 5 / 100				||
		rc.x + rc.width > fsz.width * 98 / 100	||
		rc.y < fsz.height * 5 / 100				||
		rc.y + eyeDis*16/6 > fsz.height*95/100||
		rc.width < fsz.width / 6	||
		rc.width > fsz.width * .7	||
		rc.height < fsz.height *.3	||
		rc.height > fsz.height*9/10)
	{
		::AfxMessageBox("请适当调整，确保脸在画面中央，距离半米左右。");
		return;
	}

	CWnd *pBtn = GetDlgItem(IDC_SHOOT);
	RECT rc1;
	pBtn->GetClientRect(&rc1);
	pBtn->SetWindowText("拍摄下一张");
	pBtn->SetWindowPos(NULL, 0,0, 70, rc1.bottom-rc1.top, SWP_NOMOVE | SWP_NOZORDER);
	GetDlgItem(IDC_DEL_SHOOT)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_ENROLL_PIC)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_IMPORT_MODELS)->ShowWindow(SW_HIDE);

	CString title;
	title.Format("pic %d", m_nShootNum++);
	cvNamedWindow(title, 0);
	cvResizeWindow(title, showSz.width, showSz.height);
	cvShowImage(title, faceImg8);

	IplImage *pic1 = cvCloneImage(m_frame);
	m_lstPic.AddTail(pic1);
	CvMat *face = cvCloneMat(faceImg8);
	m_lstFace.AddTail(face);
}

UINT CEnrollDlg::ShowFace()
{
	faceImg8 = cvCreateMat(faceSz.height, faceSz.width, CV_8UC1);
	m_bFoundFace = false;

	while(m_bFaceStarted)
	{

		g_webcam.GetFrame(m_frame);
		m_bFoundFace = g_faceMngr->Pic2NormFace(m_frame, faceImg8, FM_DO_NORM | FM_ALIGN_USE_BUF);
		if (m_bFoundFace)
		{
			cvShowImage("face", faceImg8);
			//m_bFoundFace = true;
		}
		else cvShowImage("face", m_noface);
		cvWaitKey(1);
		Sleep(10);
	}

	cvReleaseMat(&faceImg8);
	// Don't use cvDestroyWindow here
	TRACE("return");
	return 0;
}

UINT ShowFaceProc( LPVOID pParam )
{
	CEnrollDlg *pDlg = (CEnrollDlg *)pParam;
	return pDlg->ShowFace();
}

void CEnrollDlg::OnBnClickedCancel()
{
	// TODO: 在此添加控件通知处理程序代码
	//m_nClassId--; // 取消保存
	Release();
	OnCancel();
}

void CEnrollDlg::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);
	if (m_nShootNum > 0) 
	{
		if (m_strName.IsEmpty())
		{
			::AfxMessageBox("请输入一个名字。");
			return;
		}
		if (!g_faceMngr->HasTrained())
		{
			CString msg;
			/*msg.Format("您还没有训练，照片将被保存到 %s ,归一化人脸被保存到其子文件夹 %s , 同名文件将被覆盖。\n"
				"您可以稍后选择文件夹进行训练。是否保存？", g_strPicPath, FACE_REL_PATH);
			if (::AfxMessageBox(msg, MB_YESNO) == IDYES)*/ SaveShootPics(false);
		}
		else SaveShootPics(true);
	}
	
	Release();
	OnOK();
}

void CEnrollDlg::SaveShootPics( bool bSave2Model )
{
	CString fn;
	::CreateDirectory(g_strPicPath, NULL);
	POSITION pos = m_lstPic.GetHeadPosition(), pos1 = m_lstFace.GetHeadPosition();

	for (int i = 0; i < m_nShootNum; i++)
	{
		fn.Format("%s_%d.bmp", m_strName, i);
		cvSaveImage(g_strPicPath+fn, m_lstPic.GetNext(pos)); // 若遇同名文件则覆盖。注意！
		if (m_bSaveSmallImg>0) 
		{
			::CreateDirectory(g_strFacePath, NULL);
			cvSaveImage(g_strFacePath+fn, m_lstFace.GetNext(pos1));
		}

		if (bSave2Model) 
			if (m_bSaveSmallImg>0) 
				g_faceMngr->SavePicToModel(g_strFacePath+fn, FM_UNKNOWN_CLASS_ID, FM_DO_NOT_NORM);
			else g_faceMngr->SavePicToModel(g_strPicPath+fn, FM_UNKNOWN_CLASS_ID, FM_DO_NORM);
	}

	CString msg;
	msg.Format("共加入%d个人脸到模板库，库中现有%d个模板。", m_nShootNum, g_faceMngr->GetModelCount());
	::AfxMessageBox(msg, MB_OK | MB_ICONINFORMATION);
}

void CEnrollDlg::Release()
{
	if (g_bHasWebcam)
	{
		m_bFaceStarted = false;
		if (m_thrdFace) ::WaitForSingleObject(m_thrdFace->m_hThread, 1000); // 等待线程结束
	}
	TRACE("release");
	CString title;
	POSITION pos = m_lstPic.GetHeadPosition(), pos1 = m_lstFace.GetHeadPosition();
	for (int i = 0; i < m_nShootNum; i++)
	{	
		title.Format("pic %d", i);
		cvDestroyWindow(title);
		cvReleaseImage(& m_lstPic.GetNext(pos));
		cvReleaseMat(& m_lstFace.GetNext(pos1));
	}
	m_lstPic.RemoveAll();
	m_lstFace.RemoveAll();

	cvReleaseImage(&m_frame);
	cvDestroyWindow("face");

}

void CEnrollDlg::OnBnClickedEnrollPic()
{
	// TODO: 在此添加控件通知处理程序代码
	if (! g_faceMngr->HasTrained())
	{
		::AfxMessageBox("请先训练。");
		return;
	}
	if (g_bHasWebcam)
	{
		m_bFaceStarted = false;
		if (m_thrdFace) ::WaitForSingleObject(m_thrdFace->m_hThread, 1000);
	}

	UpdateData(TRUE);
	CFileDialog dlgFile(TRUE, 0, 0, OFN_ALLOWMULTISELECT | OFN_NOCHANGEDIR | OFN_HIDEREADONLY);
	const int bufLen = 8000;
	dlgFile.m_ofn.lpstrFile = new TCHAR[bufLen];   // redefine lpstrFile buffer size
	memset(dlgFile.m_ofn.lpstrFile, 0, bufLen);  // initialze the buffer
	dlgFile.m_ofn.nMaxFile = bufLen;           // redefine nMaxFile

	if (dlgFile.DoModal() == IDOK)
	{
		POSITION pos = dlgFile.GetStartPosition();
		int enrollNum = 0;
		while (pos)
		{
			CString path = dlgFile.GetNextPathName(pos);
			DWORD flag = FM_DO_NORM;
			if (m_bSaveSmallImg) flag |= FM_SAVE_NORM_FACE;
			if (g_faceMngr->SavePicToModel(path, FM_UNKNOWN_CLASS_ID, flag)) enrollNum++;
		}
		delete [] dlgFile.m_ofn.lpstrFile; // release space
		CString msg;
		msg.Format("共加入%d个人脸到模板库，库中现有%d个模板。", enrollNum, g_faceMngr->GetModelCount());
		::AfxMessageBox(msg, MB_OK | MB_ICONINFORMATION);
	}

	if (g_bHasWebcam)
	{
		m_bFaceStarted = true;
		m_thrdFace = ::AfxBeginThread(ShowFaceProc, this);
	}
}

void CEnrollDlg::OnBnClickedImportModels()
{
	// TODO: 在此添加控件通知处理程序代码
	CFileDialog dlgFile(TRUE, "frmodel", 0, OFN_NOCHANGEDIR | OFN_HIDEREADONLY, 
		"人脸模板文件 (*.frmodel)|*.frmodel||");
	if (dlgFile.DoModal() == IDOK)
	{
		CString path = dlgFile.GetPathName();
		//setlocale(LC_ALL, "Chinese-simplified"); //设置中文环境
		ifstream is(path, ios::binary);
		if (!is)
		{
			::AfxMessageBox(path+" : 无法读取。");
			return;
		}

		int readNum = g_faceMngr->ReadModelFromFile(is);
		CString msg;
		msg.Format("共导入%d个模板，现有%d个模板。\n请注意模板和子空间降维矩阵的对应。", 
			readNum, g_faceMngr->GetModelCount());
		::AfxMessageBox(msg, MB_OK | MB_ICONINFORMATION);

		is.close();
		// setlocale(LC_ALL, "C");// 还原

		if (m_nShootNum == 0) OnBnClickedCancel();
	}
}

void CEnrollDlg::OnBnClickedAdv()
{
	// TODO: 在此添加控件通知处理程序代码
	m_dlgAdv.DoModal();
}

void CEnrollDlg::OnBnClickedDelShoot()
{
	// TODO: 在此添加控件通知处理程序代码
	CString title;
	title.Format("pic %d", --m_nShootNum);
	cvDestroyWindow(title);

	m_lstPic.RemoveTail();
	m_lstFace.RemoveTail();

	if(m_nShootNum == 0)
	{
		CWnd *pBtn = GetDlgItem(IDC_SHOOT);
		RECT rc1;
		pBtn->GetClientRect(&rc1);
		pBtn->SetWindowText("拍摄");
		pBtn->SetWindowPos(NULL, 0,0, 35, rc1.bottom-rc1.top, SWP_NOMOVE | SWP_NOZORDER);
		GetDlgItem(IDC_DEL_SHOOT)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_ENROLL_PIC)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_IMPORT_MODELS)->ShowWindow(SW_SHOW);
	}
}
