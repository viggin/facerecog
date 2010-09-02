#define DLLSRC

#include "FaceMngr_fast.h"
#include <iostream>

#ifdef COMPILE_MNGR_FAST


CFaceMngr::CFaceMngr(void)
{
	tfaceImg8 = tfaceImg32 = tfeature = tmodel = m_models = NULL;
	m_ids = NULL;
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

	delete []m_ids;

	delete align;
	delete light;
	delete feature;
	delete ss;

}

bool CFaceMngr::Init( CvSize faceSz /*= cvSize(0,0)*/ )
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
	tfaceImg32 = cvCreateMat(m_faceSz.height, m_faceSz.width, CV_FT_FC1);
	tfeature = cvCreateMat(m_featureSz, 1, CV_FT_FC1);

	return true;
}

bool CFaceMngr::Pic2NormFace( CvArr *pic, CvMat *faceImg8, DWORD flag /*= FM_DO_NORM*/, 
							 CvPoint2D32f *leftEye /*= NULL*/, CvPoint2D32f *rightEye /*= NULL*/ )
{
	if (flag & FM_DO_FACE_ALIGN)
	{ // 这个大括号去不得！
		if ( ! align->GetFace((IplImage*)pic, faceImg8, leftEye, rightEye) ) // 当使用FaceAlign_Coord时
			return false;
	}
	else cvResize(pic, faceImg8, CV_INTER_NN); // if flag & FM_DO_FACE_ALIGN

	if (flag & FM_DO_LIGHT_PREP)
		light->RunLightPrep(faceImg8);

	return true;
}

bool CFaceMngr::NormFace2Model( CvArr *faceImg8, CvMat *model )
{
	cvConvertScale(faceImg8, tfaceImg32, 1.0/255);
	feature->GetFeature(tfaceImg32, tfeature);
	ss->Project(tfeature, model);
	return true;
}

bool CFaceMngr::BatchPicEnroll( LPCTSTR rootPath, vector<SFInfo> &enrList, DWORD flag )
{

	/* 预处理 */
	if ( ! HasTrained() || m_models != NULL ) return false;
	if (flag & FM_SHOW_DETAIL) cout<<"Enrolling..."<<endl;

	/* 初始化 */
	m_enrNum = enrList.size();
	CvMat *fts = cvCreateMat(m_featureSz, m_enrNum, CV_FT_FC1);
	CvMat col;

	CString strRp = rootPath;
	int idx = 0;
	m_ids = new int[m_enrNum];


	/* 读入图像并转化为原始特征 */
	sfi_iter iter = enrList.begin();
	for (; iter != enrList.end(); iter++)
	{
		CString path = strRp + iter->picPath;
		IplImage *img = cvLoadImage(path, CV_LOAD_IMAGE_GRAYSCALE);
		if (!img)
		{
			::MessageBox1(path+" not found.");
			exit(1);
		}
		Pic2NormFace(img, tfaceImg8, flag, &(iter->leye), &(iter->reye));

		cvConvertScale(tfaceImg8, tfaceImg32, 1.0/255);
		cvGetCol(fts, &col, idx);
		feature->GetFeature(tfaceImg32, &col);
		m_ids[idx++] = iter->classId;
		cvReleaseImage(&img);
	}


	/* 投影得到模板矩阵 */
	m_models = cvCreateMat(m_modelSz, m_enrNum, CV_MODEL_FC1);
	ss->Project(fts, m_models);
	cvReleaseMat(&fts);
	return true;
}

bool CFaceMngr::Train( LPCTSTR rootPath, vector<SFInfo> &trainList, DWORD flag )
{

	/* 预处理 */
	cvReleaseMat(&m_models);
	delete []m_ids;
	if (flag & FM_SHOW_DETAIL) cout<<"Training..."<<endl<<"\tComputing Feature..."<<endl;
	//tic();


	/* 初始化 */
	m_trainNum = trainList.size();
	CvMat *fts = cvCreateMat(m_featureSz, m_trainNum, CV_FT_FC1);
	CvMat col;

	CString strRp = rootPath;
	int idx = 0;
	m_ids = new int[m_trainNum];


	/* 读入图像并转化为原始特征 */
	sfi_iter iter = trainList.begin();
	for (; iter != trainList.end(); iter++)
	{
		CString path = strRp + iter->picPath;
		IplImage *img = cvLoadImage(path, CV_LOAD_IMAGE_GRAYSCALE);
		if (!img)
		{
			::MessageBox1(path+" not found.");
			exit(1);
		}
		Pic2NormFace(img, tfaceImg8, flag, &(iter->leye), &(iter->reye));

		cvConvertScale(tfaceImg8, tfaceImg32, 1.0/255);
		cvGetCol(fts, &col, idx);
		feature->GetFeature(tfaceImg32, &col);
		m_ids[idx++] = iter->classId;
		cvReleaseImage(&img);
	}


	/* 训练 */
	//double t1 = toc();
	if (flag & FM_SHOW_DETAIL) cout<<"\tComputing Subspace..."<<endl;
	ss->Train(fts, m_ids);
	m_modelSz = ss->GetSubspaceDim();
	tmodel = cvCreateMat(m_modelSz, 1, CV_MODEL_FC1);


	/* 投影得到模板矩阵 */
	if (flag & FM_TRAIN_SAVE2MODEL)
	{
		m_models = cvCreateMat(m_modelSz, m_trainNum, CV_MODEL_FC1);
		ss->Project(fts, m_models);
	}
	else 
		delete []m_ids;
	cvReleaseMat(&fts);
	m_trainclsNum = ss->classNum;

	return true;
}

bool CFaceMngr::HasTrained()
{
	return ss->m_bTrained;
}

bool CFaceMngr::ModelRecognize( CvMat *model, SMatch *info)
{
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
	info->classId = m_ids[curMatch];
	info->dist = minDist;
	return true;
}

bool CFaceMngr::PicRecognize( CvArr *pic, DWORD flag, SMatch *info, CvPoint2D32f *leye /*= NULL*/, CvPoint2D32f *reye /*= NULL*/ )
{
	Pic2NormFace(pic, tfaceImg8, flag, leye, reye);
	NormFace2Model(tfaceImg8, tmodel);
	ModelRecognize(tmodel, info);
	return true;
}

double CFaceMngr::BatchPicRecog( LPCTSTR rootPath, vector<SFInfo> &testList, vector<SMatch> &resList, DWORD flag )
{
	if (flag & FM_SHOW_DETAIL) cout<<"Recognizing..."<<endl;
	sfi_iter testIter = testList.begin();
	int correctNum = 0;
	SMatch info;

	for (; testIter != testList.end(); testIter++)
	{
		CString path = rootPath;
		path += testIter->picPath;
		IplImage *img = cvLoadImage(path, CV_LOAD_IMAGE_GRAYSCALE);
		if (!img)
		{
			::MessageBox1(path+" not found.");
			exit(1);
		}
		PicRecognize(img, flag, &info, &(testIter->leye), &(testIter->reye));
		resList.push_back(info);
		if (info.classId == testIter->classId) correctNum++;
	}
	return (double)correctNum/testList.size();
}

bool CFaceMngr::WriteMatToFile( ofstream &os )
{
	ss->WriteDataToFile(os);
	return true;
}

bool CFaceMngr::ReadMatFromFile( ifstream &is )
{
	bool ret = ss->ReadDataFromFile(is);
	if (ss->inputDim != m_featureSz)
	{
		ss->Release();
		MessageBox1("Feature size not match!");
		return false;
	}
	m_modelSz = ss->GetSubspaceDim();
	cvReleaseMat(&tmodel);
	tmodel = cvCreateMat(m_modelSz, 1, CV_MODEL_FC1);
	return ret;
}

#endif