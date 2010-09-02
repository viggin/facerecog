/*
	利用左右眼坐标进行人脸对齐。
	使用方法类似FaceAlign_ASM。
*/
#pragma once

#define ALIGN_HEADER

#include "FaceAlign.h"

#ifdef COMPILE_ALIGN_COORD

#include "../tools/tools.h"


class DLLEXP CFaceAlign
{
public:
	CFaceAlign(void);
	~CFaceAlign(void){}

	bool Init(CvSize faceSz = cvSize(0,0));

	// 这里的leftEye是图像左侧的眼睛坐标，也就是图像中人的右眼
	bool GetFace(IplImage *pic8, CvMat *faceImg, CvPoint2D32f *leftEye, CvPoint2D32f *rightEye);

	CvSize	m_faceSz;
	float	normRow, normDis; // 归一化后眼睛的距离图像上边界的距离和两眼距离

	float	m_angle, m_dis; // 两眼角度和距离
};

#endif