#define DLLSRC

#include "FaceFeature_pixel.h"

#ifdef COMPILE_FT_PIXEL


CFaceFeature::CFaceFeature(void)
{
}

CFaceFeature::~CFaceFeature(void)
{
}

int CFaceFeature::Init( CvSize imgSize, CvMat *mask )
{
	m_ftSz = imgSize.height * imgSize.width;
	return m_ftSz;
}

void CFaceFeature::GetFeature( CvArr *faceImg32, CvMat *ft32 )
{
	CvMat header;
	cvReshape(faceImg32, &header, 0, m_ftSz);
	cvCopy(&header, ft32);
}

#endif