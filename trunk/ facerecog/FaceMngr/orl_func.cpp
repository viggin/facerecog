#define DLLSRC

#include "FaceMngr_fast_fast.h"
#include <iostream>

#ifdef COMPILE_MNGR_FAST_FAST

double CFaceMngr::run_orl( LPCTSTR rp, vector<SFInfo> & trainList, vector<SFInfo> & testList, int * resultList )
{

/* 预处理 */
	cvReleaseMat(&m_models);
	delete []trainIds;
	cout<<"Computing Feature...\t";
	tic();


/* 初始化 */
	m_trainNum = trainList.size();
	CvMat *fts = cvCreateMat(m_featureSz, m_trainNum, CV_FT_FC1);
	CvMat col;

	CString strRp = rp;
	int idx = 0;
	trainIds = new int[m_trainNum];


/* 读入图像并转化为原始特征 */
	sfi_iter iter = trainList.begin();
	for (; iter != trainList.end(); iter++)
	{
		IplImage *img = cvLoadImage(strRp+iter->picPath, CV_LOAD_IMAGE_GRAYSCALE);
		cvResize(img, tfaceImg8, CV_INTER_NN);
		cvConvertScale(tfaceImg8, tfaceImg32, 1.0/255); // orl不用归一化
		cvGetCol(fts, &col, idx);
		feature->GetFeature(tfaceImg32, &col);
		trainIds[idx++] = iter->classId;
		cvReleaseImage(&img);
	}


/* 训练 */
	double t1 = toc();
	cout<<t1<<"s"<<endl<<"Training...\t";
	ss->Train(fts, trainIds);
	m_modelSz = ss->GetSubspaceDim();
	tmodel = cvCreateMat(m_modelSz, 1, CV_MODEL_FC1);


/* 投影得到模板矩阵 */
	m_models = cvCreateMat(m_modelSz, m_trainNum, CV_MODEL_FC1);
	ss->Project(fts, m_models);
	cvReleaseMat(&fts);
	double t2 = toc();
	cout<<t2-t1<<"s"<<endl<<"Recognizing...\t";


/* 识别 */
	int correctNum = 0;
	idx = 0;
	iter = testList.begin();
	for (; iter != testList.end(); iter++)
	{
		IplImage *img = cvLoadImage(strRp + iter->picPath, CV_LOAD_IMAGE_GRAYSCALE);
		cvResize(img, tfaceImg8, CV_INTER_NN);
		cvConvertScale(tfaceImg8, tfaceImg32, 1.0/255);
		resultList[idx] = NormFaceRecog(tfaceImg32);
		if (resultList[idx++] == iter->classId)
			correctNum++;
		cvReleaseImage(&img);
	}


/* OK */
	double t3 = toc();
	cout<<t3-t2<<"s"<<endl;
	return (double)correctNum / testList.size();
}

#endif