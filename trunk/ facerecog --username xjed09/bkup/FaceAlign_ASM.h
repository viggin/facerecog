#pragma once

/*
	利用asmlibrary进行人脸、人眼检测。
	使用方法：
	调用 Init，之后调用 GetFace。自动Release。
	注意如果输入图片太大，时间可能会较长，应适当缩小图片大小。
	注意人脸检测需要的 FDFN 文件和 asmlibrary 需要的 ASMFN 文件
*/

#include "../tools/tools.h"
#include "asmfitting.h"

#define INPUT_PATH	"..\\input\\"

#define FDFN	INPUT_PATH "haarcascade_frontalface_alt.xml"
#define ASMFN	INPUT_PATH "AsmModel.amf"


class DLLEXP CFaceAlign
{
public:
	CFaceAlign(void);
	 ~CFaceAlign(void);

private:
	asmfitting			m_AsmFit;
	asm_vjfacedetect	m_FDinAsm;
	asm_shape			m_shape;

public:
	CvSize	m_faceSz;
	float	normRow, normDis; // 归一化后眼睛的距离图像上边界的距离和两眼距离
	int		m_nFiltLevel; // 用“滤波”方式去除人眼定位的噪声时，平均滤波器的宽度

	CvRect	m_rcCurFace;
	float	m_angle, m_dis; // 两眼角度和距离

	int idx;
	CvPoint2D32f leftEye, rightEye;
	CvPoint2D32f *lefta;
	CvPoint2D32f *righta;

private:
	bool InitAsm();

public:
	 bool Init(CvSize faceSz = cvSize(0,0));


	// 进行人眼检测后，对input人脸图像进行旋转、缩放、剪裁
	 bool EyeCoord2FaceCrop( IplImage * pic8, CvMat * faceImg, bool useBuf );


	// 如果faceIdx = -1则返回最大的人脸，否则选第faceIdx(=0,1,...)个人脸，若没有这么多脸则返回false
	// 为提高效率，faceIdx>0时不重新检测人脸，因此要一次检测多个人脸时应连续调用
	// 调用asmlibrary中提供的方法进行人脸检测(实际用的是OpenCV 1.0中的Haar Cascade)和人脸特征点定位
	 bool AsmDetectFeaturePoints(IplImage *pic8, int faceIdx = -1);

	// 调用 AsmDetectFeaturePoints 和 EyeCoord2FaceCrop
	// 参数类似 HaarFacesCropByEye
	 bool GetFace(IplImage *pic8, CvMat *faceImg, bool useBuf);

};

#endif