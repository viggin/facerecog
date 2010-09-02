/*
	利用OpenCV内置Haar Cascade函数进行人脸检测
	使用方法：
	调用 Init，之后调用 GetFace。自动Release。
	注意如果输入图片太大，时间可能会较长，应适当缩小图片大小。
	注意人脸检测需要的 FDFN 文件.
*/
#pragma once


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