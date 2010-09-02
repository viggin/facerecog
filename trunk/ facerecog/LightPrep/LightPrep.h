#pragma once

/*
	进行光照预处理。目前方法为：同态滤波+直方图规定化
	使用方法：
	Init->RunLightPrep
	初始化需要掩模图像文件 MASK_FN （人脸之外的区域为0）和拥有标准直方图的图像文件 HISTMD_FN
*/


#include "../tools/tools.h"

#define INPUT_PATH	""
#define MASK_FN		INPUT_PATH "mask.bmp"
#define HISTMD_FN	INPUT_PATH "histModel.bmp"

class DLLEXP CLightPrep
{
public:
	CLightPrep();
	 ~CLightPrep();

	CvMat		*h; // 用于同态滤波的高斯高通核
	int			h_radius;

	CvMat		*m_mask, *m_invMask;
	CvMat		*tfaceImg32;

	// 用于直方图规定化
	CvHistogram	*m_histdst, *m_histsrc;
	CvMat		*lutEq2Dst, *m_lutSrc2Eq;
	double		m_scale;

	void InitFilterKernel(CvSize imgSz);
	bool InitMask(CvSize imgSz, bool useMask);
	bool InitHistNorm(CvSize imgSz);
	bool Init(CvSize imgSz, bool useMask);

	// input is 8u, calls HomographicFilter and HistNorm
	void RunLightPrep(CvMat *faceImg8);

	 // img must be CV_F32C1
	void HomographicFilter(CvMat *img);

	// derived from http://www.opencv.org.cn/forum/viewtopic.php?f=1&t=7055
	// only for 8UC1
	void HistNorm(CvArr *src);

	 // temporarily not in use
	void MaskFace(CvArr *src);

	void Release();
};
