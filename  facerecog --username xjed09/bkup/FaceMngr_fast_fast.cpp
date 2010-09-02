#define DLLSRC

#include "FaceMngr_fast.h"

#ifdef COMPILE_MNGR_FAST


CFaceMngr::CFaceMngr(void)
{
	tfaceImg8 = tfaceImg32 = tfeature = tmodel = m_models = NULL;
	trainIds = NULL;
	align = NULL;
	light = NULL;
	feature = NULL;
	ss = NULL;
}

CFaceMngr::~CFaceMngr(void)
{
	cvReleaseMat(&tfaceImg8);
	cvReleaseMat(&tfaceImg32);
	cvReleaseMat(&tfeature);
	cvReleaseMat(&tmodel);
	cvReleaseMat(&m_models);

	delete []trainIds;

	delete align;
	delete light;
	delete feature;
	delete ss;
}

bool CFaceMngr::Init( CvSize faceSz )
{
	align = new CFaceAlign;
	if (! align->Init(faceSz)) return false;
	m_faceSz = align->m_faceSz;

	light = new CLightPrep;
	if (! light->Init(m_faceSz, true)) return false;

	feature = new CFaceFeature;
	if (! (m_featureSz = feature->Init(m_faceSz, light->m_mask))) return false;

	ss = new CSubspace;

	tfaceImg8 = cvCreateMat(m_faceSz.height, m_faceSz.width, CV_8UC1);
	tfaceImg32 = cvCreateMat(m_faceSz.height, m_faceSz.width, CV_32FC1);
	tfeature = cvCreateMat(m_featureSz, 1, CV_FT_FC1);

	return true;
}

int CFaceMngr::NormFaceRecog( CvMat *faceImg32 )
{
	feature->GetFeature(faceImg32, tfeature);
	ss->Project(tfeature, tmodel);

	double	minDist = 1e9, curVal; // minDist should be among -1~1 for angle metric
	int curMatch = 0;
	CvMat col;

	// 试图向Matlab那样用矩阵操作，把tmodel Repeat成矩阵，然后Mul,Reduce,MinMaxLoc，发现反倒略慢，所以还是用循环
	for (int i = 0; i < m_trainNum; i++) 
	{
		cvGetCol(m_models, &col, i);
		curVal = ss->CalcVectorDist(&col, tmodel);
		if (curVal < minDist)
		{
			minDist = curVal;
			curMatch = i;
		}
	}

	return trainIds[curMatch];
}

#endif