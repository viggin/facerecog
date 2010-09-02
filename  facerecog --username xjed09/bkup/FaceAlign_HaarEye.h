/*
	利用OpenCV内置Haar Cascade函数进行人脸检测和人眼检测，然后用人眼检测结果进行align
	使用方法：
	调用 Init，之后调用 GetFace。自动Release。
	注意如果输入图片太大，时间可能会较长，应适当缩小图片大小。
	注意需要的 FDFN 和 EDFN 文件.
*/

#pragma once

#include "../tools/CompDefs.h"

#ifdef COMPILE_ALIGN_HAAREYE

#include "../tools/tools.h"

#define FDFN	"haarcascade_frontalface_alt.xml"
#define EDFN	"haarcascade_eye_tree_eyeglasses.xml"


class  DLLEXP CFaceAlign
{
public:
	CFaceAlign(void);
	 ~CFaceAlign(void);

private:

	CvHaarClassifierCascade	*m_FDcascade, *m_EDcascade;
	CvMemStorage			*m_FDstorage, *m_EDstorage;


public:
	CvSize	m_faceSz;
	CvRect	m_rcCurFace;
	CvPoint2D32f leftEye, rightEye;
	float	m_angle, m_dis; // 两眼角度和距离
	float	normRow, normDis; // 归一化后眼睛的距离图像上边界的距离和两眼距离
	static const int m_nFiltLevel = 3; // 用“滤波”方式去除人眼定位的噪声时，平均滤波器的宽度


public:
	 bool Init();
	bool InitHaarFace();
	bool InitHaarEye();
	
	/*
		调用 HaarFaceDetect2 、HaarEyeDetect 和 EyeCoord2FaceCrop
		input:		输入的包含或不包含人脸的8U图像
		faceImg:	尺寸为	m_faceSz 的8UC1矩阵，输出
		useBuf:		如果为真，将对本次和前 filtLevel-1 次人眼检测坐标进行平均，以消除噪声
					建议当光照等条件较差时使用
	*/
	 bool GetFace( IplImage *input, CvMat *faceImg, bool useBuf );


	// 用OpenCV 2.0中的函数进行人脸检测，如果检测到，返回true，人脸Rect存入m_rcCurFace
	// 用同一个xml文件时，OpenCV 2.0的检测效果略优于1.0，但Rect似乎略大一些
	bool HaarFaceDetect2(IplImage *input);


	// 应该保证input中最多只有一只眼睛
	bool HaarEyeDetect(IplImage *input, CvPoint2D32f *eye);


	// 进行人眼检测后，对input人脸图像进行旋转、缩放、剪裁
	 bool EyeCoord2FaceCrop( IplImage * input, CvMat * faceImg, bool useBuf );


	 void Release();
};

extern "C" DLLEXP CFaceAlign *GetFaceAlign();

#endif