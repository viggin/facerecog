/*
	提取Gabor特征，并进行均匀抽样，利用空域卷积进行计算。
	Gabor小波：
			  u^2+v^2         -(u^2+v^2)*(x^2+y^2)                               -sigma^2
	G(x,y) = --------- * exp(----------------------) * [ exp(i*(u*x+v*y)) - exp(----------)]
			  sigma^2               2*sigma^2                                       2
				  u         kmax         pi*MU         kmax         pi*MU
	in which, k=(   ), u = ------ * cos(-------), v = ------ * sin(-------).
				  v         f^NU           8           f^NU           8
	in which, kmax = pi/2,f = sqrt(2).
	sigma↑，G变扁平；NU↑，频率↓，尺度↑；MU↑，角度旋转
	ref: Gabor Feature Based Classification Using the Enhanced Fisher Linear 
	Discriminant Model for Face Recognition, Chengjun Liu et al.

	用法：
		Init -> GetFeature
*/
#pragma once

#define FT_HEADER

#include "FaceFeature.h"

#ifdef COMPILE_FT_GABOR

#include "../tools/tools.h"

#define PI		3.1415926535897932384626433832795
#define CV_FT_FC1	CV_32FC1 // depth = 32 or 64
#define CV_FT_FC2	CV_32FC2


class DLLEXP CFaceFeature
{
public:
	CFaceFeature(void);

	 ~CFaceFeature(void);


	void InitGaborKernel();


	int InitSamplePoints(CvMat *mask, CvSize imgSize);


	/*
		mask:		掩模图像，不为0的区域为有效人脸区域
		imgSize:	人脸图像尺寸，也就是mask尺寸
		返回:		原始特征向量长度（抽样点数 * scaleNum * angleNum）
	*/
	int Init(CvSize imgSize, CvMat *mask);


	/*
		faceImg32:	输入浮点图像
		ft32:	输出原始特征列向量
	*/
	void GetFeature(CvArr *faceImg32, CvMat *ft32);


	void ShowGaborKernel();
	void GaborConv(CvArr *src, CvMat *dst, int scale, int angle);

	void Release();


	bool	m_bInited;

	/* Gabor相关参数
			scaleNum:	尺度抽样数，即NU的抽样数
			angleNum:	角度抽样数，即MU的抽样数，MU = 0,1,...,angleNum-1
			kernelRadius: Gabor小波的窗长（像素数），如果为0，则自动计算
			scales:		如果为NULL，则尺度抽样NU = 0,1,...,scaleNum-1; 否则按scales指定
	*/
	double	kmax, f, sigma, th;
	int		scaleNum, angleNum, kernelRadius;
	int		*scales;

	int		kernelWidth;
	CvMat	***G;
	CvMat	*tI, *tC, *tD;


	// 抽样相关参数和变量
	int		sampleIntv; // 抽样间隔

	CvSize	ssz, padSize;
	CvPoint	topleft;
	CvMat	*padded;
	int		*startPts, *endPts;
};

#endif