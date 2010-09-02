
// frtestDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "frtest.h"
#include "frtestDlg.h"
#include "WndUtils.h"
#include "ProjDefs.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CFaceMngr	*g_faceMngr;
CWebCam		g_webcam;
bool		g_bHasWebcam;

CString		g_strPicPath, g_strFacePath;
CString		g_strDefMat, g_strDefModel;


bool CfrtestDlg::InitSys()
{
	RECT rc;
	GetWindowRect(&rc);
	SetWindowPos(NULL, rc.left, 100, 0,0, SWP_NOSIZE | SWP_NOZORDER);

	ReadIni();
	g_bHasWebcam = g_webcam.Init();

	if (g_bHasWebcam)
	{
		CWnd *pWnd = GetDlgItem(IDC_VIDEO);
		CvSize frameSz = g_webcam.GetFrameSize(), showSz = frameSz;
		if (showSz.height != 240) // (240*320)是预定显示大小，如果frameSz不是则进行调整
			showSz *= (240.0/showSz.height);
		if (showSz.width > 320)
			showSz *= (320.0/showSz.width);
		EmbedCvWindow(pWnd->m_hWnd, "video", showSz.width, showSz.height);
		pWnd->SetWindowPos(NULL, 0,0, showSz.width+10, showSz.height+10, SWP_NOZORDER | SWP_NOMOVE);

		m_frame = cvCreateImage(frameSz, IPL_DEPTH_8U, 3);
		m_bFrameStarted = true;
		m_thrdFrame = ::AfxBeginThread(ShowFrameProc, this);
	}

	m_dlgTrain.m_strTrainFd = g_strFacePath;
	::AfxBeginThread(InitMngrProc, this);
	return true;
}

void CfrtestDlg::ReadIni()
{
	CString fn = ".\\config.ini", sn = "folders"; // 文件名必须加‘.\\’
	::GetPrivateProfileString(sn, "default_pic_path", DEF_PIC_PATH, 
		g_strPicPath.GetBuffer(MAX_PATH), MAX_PATH, fn);
	g_strPicPath.ReleaseBuffer();
	::GetPrivateProfileString(sn, "default_frmat", "", 
		g_strDefMat.GetBuffer(MAX_PATH), MAX_PATH, fn);
	g_strDefMat.ReleaseBuffer();
	::GetPrivateProfileString(sn, "default_frmodel", "", 
		g_strDefModel.GetBuffer(MAX_PATH), MAX_PATH, fn);
	g_strDefModel.ReleaseBuffer();
	g_strFacePath = g_strPicPath;
	g_strFacePath.TrimRight('\\');
	g_strFacePath += '\\';
	g_strFacePath += FACE_REL_PATH;
}

UINT CfrtestDlg::ShowFrame()
{
	while(m_bFrameStarted)
	{
		g_webcam.GetFrame(m_frame);
		cvShowImage("video", m_frame);
		cvWaitKey(1);
		Sleep(100);
	}

	return 0;
}

void CfrtestDlg::OnBnClickedEnroll()
{
	// TODO: 在此添加控件通知处理程序代码
	m_dlgEnroll.DoModal();
}

void CfrtestDlg::OnBnClickedTrain()
{
	// TODO: 在此添加控件通知处理程序代码
	int nRet = m_dlgTrain.DoModal();
	ShowWindow(SW_SHOWNORMAL);
	if (nRet == IDOK)
	{
		SetDlgItemText(IDC_STATE, "训练中...");
		tic();//double t = (double)cvGetTickCount();
		vector<SFInfo> paths;

		::GenFileList(m_dlgTrain.m_strTrainFd, paths);
		DWORD flag = FM_DO_NOT_NORM;
		if (m_dlgTrain.m_bSave2Model > 0) flag |= FM_TRAIN_SAVE2MODEL;
		if (m_dlgTrain.m_bHasNormed == 0) flag |= FM_DO_NORM;
		bool ret = g_faceMngr->Train(m_dlgTrain.m_strTrainFd, paths, flag);

		SetDlgItemText(IDC_STATE, "");
		if (ret)
		{
			double t1 = toc(); // ((double)cvGetTickCount() - t) / ((double)cvGetTickFrequency()*1e6);
			CString msg;
			msg.Format("共有%d人的%d张图片参加训练，得到特征%d维，降维后为%d维。训练完成，耗时%fs。",
				g_faceMngr->m_trainclsNum, g_faceMngr->m_trainNum, 
				g_faceMngr->m_featureSz, g_faceMngr->m_modelSz, t1);
			::AfxMessageBox(msg, MB_OK | MB_ICONINFORMATION);
		}
		paths.clear();
	}
}

void CfrtestDlg::OnBnClickedRecog()
{
	// TODO: 在此添加控件通知处理程序代码
	if (! g_faceMngr->HasTrained())
	{
		::AfxMessageBox("请先训练。");
		return;
	}
	if (g_faceMngr->GetModelCount() == 0)
	{
		::AfxMessageBox("请先注册。");
		return;
	}
	m_dlgRecog.DoModal();
	ShowWindow(SW_SHOWNORMAL);
}

void CfrtestDlg::OnBnClickedExport()
{
	// TODO: 在此添加控件通知处理程序代码
	bool ret1 = false;
	setlocale(LC_ALL, "Chinese-simplified"); //设置中文环境

	if (g_faceMngr->HasTrained())
	{
		ret1 = true;
		CFileDialog dlgFile(FALSE, "frmat", "test", 
			OFN_NOCHANGEDIR | OFN_OVERWRITEPROMPT , "人脸识别矩阵文件 (*.frmat)|*.frmat||");
		if (dlgFile.DoModal() == IDOK)
		{
			CString path = dlgFile.GetPathName();
			ofstream os(path, ios::binary);
			if (!os)
			{
				::AfxMessageBox(CString(path)+"：无法读取。");
				return;
			}
			else
			{
				g_faceMngr->WriteMatToFile(os);
				os.close();
			}
		}
	}

	if (g_faceMngr->GetModelCount() > 0)
	{
		ret1 = true;
		CFileDialog dlgFile1(FALSE, "frmodel", "test", 
			OFN_NOCHANGEDIR | OFN_OVERWRITEPROMPT, "人脸模板文件 (*.frmodel)|*.frmodel||");
		if (dlgFile1.DoModal() == IDOK)
		{
			CString path = dlgFile1.GetPathName();
			ofstream os(path, ios::binary);
			if (!os)
			{
				::AfxMessageBox(CString(path)+"：无法读取。");
				return;
			}
			else
			{
				g_faceMngr->WriteModelToFile(os);
				os.close();
			}
		}
	}

	setlocale(LC_ALL, "C");// 还原

	if ( !ret1 )
	{
		::AfxMessageBox("没什么要存的！");
		return;
	}

	::AfxMessageBox("导出完成。\n每个人脸模板文件都应该有唯一对应的矩阵文件，导入时请不要弄混。", MB_OK | MB_ICONINFORMATION);
}

void CfrtestDlg::OnBnClickedCancel()
{
	// TODO: 在此添加控件通知处理程序代码

	//_CrtDumpMemoryLeaks(); // 会在你的释放函数之前就把所有空间释放掉，还告诉你Memory leak!
	if (g_bHasWebcam)
	{
		m_bFrameStarted = false;
		if (m_thrdFrame) ::WaitForSingleObject(m_thrdFrame->m_hThread, INFINITE);
		cvReleaseImage(&m_frame);
		cvDestroyWindow("video");
	}
	delete g_faceMngr;

	OnCancel();
}

UINT ShowFrameProc( LPVOID pParam )
{
	CfrtestDlg *pDlg = (CfrtestDlg *)pParam;
	return pDlg->ShowFrame();
}

UINT InitMngrProc( LPVOID pParam )
{
	g_faceMngr = new CFaceMngr;

	CString msg;
	if (! g_faceMngr->Init()) // 初始化失败
	{
		CfrtestDlg *pdlg = (CfrtestDlg *)pParam;
		pdlg->GetDlgItem(IDC_ENROLL)->EnableWindow(FALSE);
		pdlg->GetDlgItem(IDC_TRAIN)->EnableWindow(FALSE);
		pdlg->GetDlgItem(IDC_RECOG)->EnableWindow(FALSE);
		pdlg->GetDlgItem(IDC_EXPORT)->EnableWindow(FALSE);
		return 1;
	}

	setlocale(LC_ALL, "Chinese-simplified"); //设置中文环境
	if (! g_strDefMat.IsEmpty())
	{
		ifstream is(g_strDefMat, ios::binary);
		if (!is)
			::AfxMessageBox(g_strDefMat + "：无法读取。");
		else
		{
			if (! g_faceMngr->ReadMatFromFile(is)) 
				::AfxMessageBox("默认frmat文件读取失败。");
			is.close();
		}
	}

	if (! g_strDefModel.IsEmpty())
	{
		ifstream is(g_strDefModel, ios::binary);
		if (!is)
			::AfxMessageBox(g_strDefModel + "：无法读取。");
		else
		{		
			int readNum = g_faceMngr->ReadModelFromFile(is);
			CString msg;
			msg.Format("共导入%d个模板，现有%d个模板。\n请注意模板和子空间降维矩阵的对应。", 
				readNum, g_faceMngr->GetModelCount());
			::AfxMessageBox(msg, MB_OK | MB_ICONINFORMATION);
			is.close();
		}
	}

	setlocale(LC_ALL, "C");// 还原
	return 0;
}