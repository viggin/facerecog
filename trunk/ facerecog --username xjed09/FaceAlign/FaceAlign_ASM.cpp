#define DLLSRC

#include "FaceAlign_ASM.h"

#ifdef COMPILE_ALIGN_ASM

#pragma comment(lib, "asmlibrary.lib")


CFaceAlign::CFaceAlign(void)
{
	// 可以设定的参数
	m_faceSz = cvSize(65,80);
	normRow = 24;
	normDis = 30;
	m_nFiltLevel = 3;
}

CFaceAlign::~CFaceAlign(void)
{
	delete []lefta;
	delete []righta;
}

bool CFaceAlign::Init(CvSize faceSz /*= cvSize(0,0)*/)
{
	if(faceSz.height > 0 && faceSz.width > 0)
		m_faceSz = faceSz;
	lefta = new CvPoint2D32f[m_nFiltLevel];
	righta = new CvPoint2D32f[m_nFiltLevel];
	return InitAsm();
}

bool CFaceAlign::InitAsm()
{
	if(! m_AsmFit.Read(ASMFN)) {
		CString msg;
		msg.Format("Can't load ASM model file %s.", ASMFN);
		::MessageBox1(msg);
		return false;
	}
	m_FDinAsm.LoadCascade(FDFN);
	return true;
}

bool CFaceAlign::GetFace( IplImage *pic8, CvMat *faceImg, bool useBuf )
{
	if (! AsmDetectFeaturePoints(pic8, -1)) return false;
	leftEye = m_shape[31],	rightEye = m_shape[36]; // 特征点编号见ASM.tif
	return EyeCoord2FaceCrop(pic8, faceImg, useBuf);
}

bool CFaceAlign::AsmDetectFeaturePoints( IplImage *pic8, int faceIdx /*= -1*/ )
{
	int nFaces;
	static asm_shape *detshapes = 0;
	bool hasFace;
	if (faceIdx < 1) hasFace = m_FDinAsm.DetectFace(&detshapes, nFaces, pic8);
	if (!hasFace || faceIdx > nFaces) return false;
	
	double	maxWidth = 0; // 目前只找最大的人脸
	if (faceIdx == -1)
	{	
		int		maxIdx = 0;
		for(int i = 0; i < nFaces; i++)
		{
			if (detshapes[i].GetWidth() > maxWidth)
			{
				maxWidth = detshapes[i].GetWidth();
				maxIdx = i;
			}
		}
		faceIdx = maxIdx;
	}

	m_rcCurFace = cvRect(int(detshapes[faceIdx].MinX()), int(detshapes[faceIdx].MinY()), 
		(int)maxWidth, int(detshapes[faceIdx].GetHeight()));
	InitShapeFromDetBox(m_shape, detshapes[faceIdx],
		m_AsmFit.GetMappingDetShape(), m_AsmFit.GetMeanFaceWidth());

	m_AsmFit.Fitting(m_shape, pic8); // 匹配ASM模型
	//DrawPoints(input, m_shape);
	return true;
}

bool CFaceAlign::EyeCoord2FaceCrop( IplImage * pic8, CvMat * faceImg, bool useBuf )
{
	static int idx = 0;
	CvPoint2D32f l1 = cvPoint2D32f(0,0), r1 = cvPoint2D32f(0,0);
	if (useBuf)
	{
		lefta[idx] = leftEye;
		righta[idx++] = rightEye;
		idx %= m_nFiltLevel;
		for (int i = 0; i < m_nFiltLevel; i++)
		{
			l1.x += lefta[i].x/m_nFiltLevel;
			l1.y += lefta[i].y/m_nFiltLevel;
			r1.x += righta[i].x/m_nFiltLevel;
			r1.y += righta[i].y/m_nFiltLevel;
		}
	}
	else
	{
		l1 = leftEye;
		r1 = rightEye;
	}
	
	float xDis = r1.x - l1.x,
		yDis = r1.y - l1.y;

	m_angle = cvFastArctan(yDis, xDis);
	m_dis = sqrt(xDis*xDis + yDis*yDis);
	

	CvMat *map = cvCreateMat(2, 3, CV_32FC1);
	CvMat *tmpDst = cvCreateMat(pic8->height, pic8->width, CV_8UC1);
	cv2DRotationMatrix(l1, m_angle, normDis/m_dis, map); // 相似变换
	//DispCvArr(map, "map");
	cvWarpAffine(pic8, tmpDst, map);

	int		leftEyeXNew = cvRound((m_faceSz.width - normDis)/2);
	int		left = cvRound(l1.x - leftEyeXNew),
		top = cvRound(l1.y - normRow);
	CvMat	tmpHeader, *sub = 0;

	if (left >= 0 && top >= 0 &&
		left + m_faceSz.width <= tmpDst->width &&
		top + m_faceSz.height <= tmpDst->height)
	{	
		sub = cvGetSubRect(tmpDst, &tmpHeader, cvRect(left, top, m_faceSz.width, m_faceSz.height));
		cvCopy(sub, faceImg);
	}

	cvReleaseMat(&map);
	cvReleaseMat(&tmpDst);
	return (sub != 0);
}

#endif