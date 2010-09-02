/*
	将图像resample成指定大小后，将像素作为特征。
	用法：
		Init -> GetFeature
*/
#pragma once

#define FT_HEADER

#include "FaceFeature.h"

#ifdef COMPILE_FT_PIXEL

#include "../tools/tools.h"


class DLLEXP CFaceFeature
{
public:
	CFaceFeature(void);

	 ~CFaceFeature(void);

	int Init(CvSize imgSize, CvMat *mask);

	void GetFeature(CvArr *faceImg32, CvMat *ft32);

	int m_ftSz;
};

#endif