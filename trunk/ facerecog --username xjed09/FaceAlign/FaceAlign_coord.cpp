#define DLLSRC

#include "FaceAlign_coord.h"

#ifdef COMPILE_ALIGN_COORD


CFaceAlign::CFaceAlign(void)
{
	m_faceSz = cvSize(65,80);
	normRow = 24;
	normDis = 30;
}

bool CFaceAlign::Init( CvSize faceSz /* = cvSize(0,0) */ )
{
	if(faceSz.height > 0 && faceSz.width > 0)
		m_faceSz = faceSz;
	return true;
}

bool CFaceAlign::GetFace( IplImage *pic8, CvMat *faceImg, CvPoint2D32f *leftEye, CvPoint2D32f *rightEye )
{
	if (!leftEye || !rightEye) return false; // FaceAlign_coord 必须提供双眼坐标
	float xDis = rightEye->x - leftEye->x,
		yDis = rightEye->y - leftEye->y;

	m_angle = cvFastArctan(yDis, xDis);
	m_dis = sqrt(xDis*xDis + yDis*yDis);


	CvMat *map = cvCreateMat(2, 3, CV_32FC1);
	CvMat *tmpDst = cvCreateMat(pic8->height, pic8->width, CV_8UC1);
	cv2DRotationMatrix(*leftEye, m_angle, normDis/m_dis, map); // 相似变换
	//DispCvArr(map, "map");
	cvWarpAffine(pic8, tmpDst, map);

	int		leftEyeXNew = cvRound((m_faceSz.width - normDis)/2);
	int		left = cvRound(leftEye->x - leftEyeXNew),
		top = cvRound(leftEye->y - normRow);
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