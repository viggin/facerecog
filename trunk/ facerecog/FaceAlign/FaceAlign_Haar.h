/*
	利用OpenCV 2.0自带的Haar Cascade函数进行人脸检测，将其结果作为对齐结果
	使用方法类似FaceAlign_ASM，效果略差。
	注意人脸检测需要的 FDFN 文件.
*/

#pragma once

#define ALIGN_HEADER

#include "FaceAlign.h"

#ifdef COMPILE_ALIGN_HAAR

#include "../tools/tools.h"

#define FDFN	INPUT_PATH "haarcascade_frontalface_alt.xml"


class DLLEXP CFaceAlign
{
public:
	CFaceAlign(void);
	~CFaceAlign(void);

private:
	CvHaarClassifierCascade	*m_FDcascade;
	CvMemStorage			*m_FDstorage;

public:
	CvRect	m_rcCurFace;
	CvSize	m_faceSz; // 预定的face size，但GetFace输入的faceImg可为任意size
	int		m_dis; // 估测的两眼距离
	static const int m_nFiltLevel = 3; // 对人脸定位结果进行平滑的阶数

public:
	bool Init();


	// 不用人眼定位结果进行对齐，直接将Haar人脸检测结果按faceImg大小存入faceImg
	// 用OpenCV 2.0中的函数进行人脸检测，如果检测到，返回true，人脸Rect存入m_rcCurFace
	// 如果bUseBuf = true，将对前后m_nFiltLevel次人脸定位结果进行平滑
	// 用同一个xml文件时，OpenCV 2.0的检测效果略优于1.0，但Rect似乎略大一些
	// only support 8-bit unsigned input
	bool GetFace(IplImage *input, CvMat *faceImg, bool bUseBuf);


	void Release();
};


#endif
