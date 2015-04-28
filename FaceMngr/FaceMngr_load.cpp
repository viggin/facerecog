#define DLLSRC

#include "FaceMngr.h"
#include "../tools/tools.h"
#include "../tools/AlgorithmSelect.h"


//#ifdef _DEBUG
//#define new DEBUG_NEW
//#undef THIS_FILE
//static TCHAR THIS_FILE[] = __FILE__;
//#endif


CFaceMngr::CFaceMngr(void)
{
	tfaceImg8 = tfaceImg32 = tfeature = tmodel = NULL;
	m_bAllLoaded = false;
}

bool CFaceMngr::InitMngr(vector<LPTSTR> dllList)
{
	m_bTrained = false;

	ha = ::LoadLibrary(dllList[0]);
	hl = ::LoadLibrary(dllList[1]);
	hf = ::LoadLibrary(dllList[2]);
	hs = ::LoadLibrary(dllList[3]);

	if ( !(ha&&hl&&hf&&hs) ) 
	{
		CString msg;
		msg.Format("Missing one of these dlls:%s,%s,%s,%s",
			dllList[0],dllList[1],dllList[2],dllList[3]);
		MessageBox1(msg);
		return false;
	}
	m_bAllLoaded = true;
	m_dllList = dllList;
	//for (int i = 0; i < 4; i++)
	//{
	//	m_dllList.push_back(CString(dllList[i]));
	//}

	f.InitAlign		= (tPfnInitAlign)GetProcAddress(ha,"InitAlign");
	f.GetFace		= (tPfnGetFace)GetProcAddress(ha,"GetFace");
	f.GetFacePose	= (tPfnGetFacePose)GetProcAddress(ha,"GetFacePose");
	f.ReleaseAlign	= (tPfnRelease)GetProcAddress(ha,"ReleaseAlign");

	f.InitLight		= (tPfnInit)GetProcAddress(hl,"InitLight");
	f.RunLightPrep	= (tPfnRunLightPrep)GetProcAddress(hl,"RunLightPrep");
	f.ReleaseLight	= (tPfnRelease)GetProcAddress(hl,"ReleaseLight");

	f.InitFeature	= (tPfnInitFeature)GetProcAddress(hf,"InitFeature");
	f.GetFeature	= (tPfnGetFeature)GetProcAddress(hf,"GetFeature");
	f.ReleaseFeature= (tPfnRelease)GetProcAddress(hf,"ReleaseFeature");

	f.CalcSubspace	= (tPfnCalcSubspace)GetProcAddress(hs,"CalcSubspace");
	f.Project		= (tPfnProject)GetProcAddress(hs,"Project");
	f.GetModelSize= (tPfnGetInt)GetProcAddress(hs,"GetModelSize");
	f.GetFtDim		= (tPfnGetInt)GetProcAddress(hs,"GetFtDim");
	f.CalcVectorDist= (tPfnCalcVectorDist)GetProcAddress(hs,"CalcVectorDist");
	f.WriteDataToFile = (tPfnWrite)GetProcAddress(hs,"WriteDataToFile");
	f.ReadDataFromFile= (tPfnRead)GetProcAddress(hs,"ReadDataFromFile");
	f.ReleaseSubspace = (tPfnRelease)GetProcAddress(hs,"ReleaseSubspace");

	//static asm_shape detshape;
	if(!f.InitAlign(&m_faceSz)) return false;
	if(!f.InitLight(m_faceSz)) return false;
	m_featureSz = f.InitFeature(m_faceSz);
	if(m_featureSz == 0) return false;

	tfaceImg8 = cvCreateMat(m_faceSz.height, m_faceSz.width, CV_8UC1);
	tfaceImg32 = cvCreateMat(m_faceSz.height, m_faceSz.width, CV_32FC1);
	tfeature = cvCreateMat(m_featureSz, 1, CV_FT_FC1);

	m_stamp = 0;

	return true;
}

CFaceMngr::~CFaceMngr(void)
{
	if (!m_bAllLoaded) return;
	cvReleaseMat(&tfaceImg8);
	cvReleaseMat(&tfaceImg32);
	cvReleaseMat(&tfeature);
	cvReleaseMat(&tmodel);

	f.ReleaseAlign();
	f.ReleaseLight();
	f.ReleaseFeature();
	f.ReleaseSubspace();

	::FreeLibrary(ha);
	::FreeLibrary(hl);
	::FreeLibrary(hf);
	::FreeLibrary(hs);

	ClearList();
}