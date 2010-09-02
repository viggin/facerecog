/*
	提取Gabor特征，并进行均匀抽样，在频域进行计算，速度比空域快大约1倍。
	用法：
		Init -> GetFeature
*/
#pragma once

#define FT_HEADER

#include "FaceFeature.h"

#ifdef COMPILE_FT_GABORF

#include "../tools/tools.h"


class DLLEXP CFaceFeature
{
public:
	CFaceFeature(void);

	 ~CFaceFeature(void);


	void InitGaborKernel();

	int InitSamplePoints(CvMat *mask);

	int Init(CvSize imgSize, CvMat *mask);

	// ft32 必须是列向量
	void GetFeature(CvArr *faceImg32, CvMat *ft32);

	void ShowGaborFace(CvArr *faceImg32);

	void Release();

	bool	m_bInited;

	// Gabor相关参数和变量
	double	kmax, f, sigma, th;
	int		scaleNum, angleNum, kernelRadius;
	int		*scales;

	CvSize	m_faceSz;
	int		kernelWidth, fsz;
	CvMat	***G;
	CvMat	*tPicR, *tPicI, *tPicF0, *tPicF;


	// 抽样相关参数和变量
	int		sampleIntv;

	int		smpPtNum;
	int		*smpPts; // 抽样点的序号，便于直接指针操作
};

#endif