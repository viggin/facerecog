#define DLLSRC

#include "FaceAlign_Haar.h"



CFaceAlign::CFaceAlign(void)
{
	m_FDcascade  = NULL;
	m_FDstorage = NULL;
}

CFaceAlign::~CFaceAlign(void)
{
	Release();
}

void CFaceAlign::Release()
{
	cvReleaseHaarClassifierCascade(&m_FDcascade);
	cvReleaseMemStorage(&m_FDstorage);
}

bool CFaceAlign::Init()
{
	m_FDcascade = (CvHaarClassifierCascade*)cvLoad(FDFN, 0, 0, 0);
	if (!m_FDcascade) {
		CString msg;
		msg.Format("Can't load %s.", _T(FDFN));
		::MessageBox1(msg);
		return false;
	}

	m_FDstorage = cvCreateMemStorage(0);
	if (!m_FDstorage) {
		::MessageBox1("Can't create memory storage for face detection.");
		return false;
	}

	m_faceSz = cvSize(100, 100);
	m_dis = m_faceSz.width / 2;
	return true;
}

#if 0 // using opencv 1.0 by asmlibrary

bool CFaceAlign::HaarFaceCrop( IplImage *input, CvMat *faceImg )
{
	asm_shape	*detshapes = 0;
	int			nFaces;
	CvRect		faceRc;
	bool hasFace = m_FDinAsm.DetectFace(&detshapes, nFaces, input);
	if(hasFace)
	{
		double	maxWidth = 0;
		int		maxIdx = 0;
		for(int i = 0; i < nFaces; i++)
		{
			if (detshapes[i].GetWidth() > maxWidth)
			{
				maxWidth = detshapes[i].GetWidth();
				maxIdx = i;
			}
		}
		faceRc.x = cvRound(detshapes[maxIdx][0].x);
		faceRc.y = cvRound(detshapes[maxIdx][0].y);
		faceRc.width = cvRound(detshapes[maxIdx][1].x - faceRc.x);
		faceRc.height = cvRound(detshapes[maxIdx][1].y - faceRc.y);

		CvMat tmpHead, *sub = 0;
		sub = cvGetSubRect(input, &tmpHead, faceRc);
		cvResize(sub, faceImg);
		return true;
	}

	return false;
}

#endif

bool CFaceAlign::GetFace( IplImage *input, CvMat *faceImg, bool bUseBuf )
{
	CvSeq *pFaces = cvHaarDetectObjects(
		input, m_FDcascade, m_FDstorage,
		1.3, // 以下4个参数可以调节
		3,
		0 |
		CV_HAAR_DO_CANNY_PRUNING	|	// 跳过平滑区域
		CV_HAAR_FIND_BIGGEST_OBJECT	|	// 只返回最大的目标
		CV_HAAR_DO_ROUGH_SEARCH		|	// 只能和BIGGEST_OBJECT一起使用,在任何窗口,只要第一个候选者被发现则结束搜寻
		0,
		cvSize(20, 20));

	if (!pFaces || pFaces->total == 0)
		return false;
	m_rcCurFace = *(CvRect *)cvGetSeqElem(pFaces, 0);

	static CvRect		history[m_nFiltLevel];
	static int			idx;
	if (bUseBuf)
	{
		history[idx++] = m_rcCurFace;
		m_rcCurFace = cvRect(0,0,0,0);
		idx %= m_nFiltLevel;
		for (int i = 0; i < m_nFiltLevel; i++)
		{
			m_rcCurFace.x += history[i].x/m_nFiltLevel;
			m_rcCurFace.y += history[i].y/m_nFiltLevel;
			m_rcCurFace.width += history[i].width/m_nFiltLevel;
			m_rcCurFace.height += history[i].height/m_nFiltLevel;
		}
	}

	CvMat sub;
	cvGetSubRect(input, &sub, m_rcCurFace);
	cvResize(&sub, faceImg);
	return true;
}


#endif