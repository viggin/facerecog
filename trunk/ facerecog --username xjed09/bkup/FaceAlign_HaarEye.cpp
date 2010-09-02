#define DLLSRC

#include "FaceAlign_HaarEye.h"

#ifdef COMPILE_ALIGN_HAAREYE

//#ifdef _DEBUG
//#define new DEBUG_NEW
//#undef THIS_FILE
//static char THIS_FILE[] = __FILE__;
//#endif

CFaceAlign *GetFaceAlign() { return new CFaceAlign; }

CFaceAlign::CFaceAlign(void)
{
	m_FDcascade = m_EDcascade = NULL;
	m_FDstorage = m_EDstorage = NULL;
}

CFaceAlign::~CFaceAlign(void)
{
	Release();
}

void CFaceAlign::Release()
{
	cvReleaseHaarClassifierCascade(&m_FDcascade);
	cvReleaseMemStorage(&m_FDstorage);
	cvReleaseHaarClassifierCascade(&m_EDcascade);
	cvReleaseMemStorage(&m_EDstorage);
}

bool CFaceAlign::Init()
{
	float sc = .5;

	m_faceSz.width = int(130*sc);
	m_faceSz.height = int(160*sc);

	normRow = 48*sc;
	normDis = 60*sc;

	return (InitHaarFace() && InitHaarEye());
}

bool CFaceAlign::InitHaarFace()
{
	m_FDcascade = (CvHaarClassifierCascade*)cvLoad(FDFN, 0, 0, 0);
	if (!m_FDcascade) {
		CString msg;
		msg.Format("Can't load %s.", _T(EDFN));
		::MessageBox1(msg);
		return false;
	}

	m_FDstorage = cvCreateMemStorage(0);
	if (!m_FDstorage) {
		::MessageBox1("Can't create memory storage for face detection.");
		return false;
	}
	return true;
}

bool CFaceAlign::InitHaarEye()
{
	m_EDcascade = (CvHaarClassifierCascade*)cvLoad(EDFN, 0, 0, 0);
	if (!m_EDcascade) {
		CString msg;
		msg.Format("Can't load %s.", _T(EDFN));
		::MessageBox1(msg);
		return false;
	}
	m_EDstorage = cvCreateMemStorage(0);
	if (!m_EDstorage) {
		::MessageBox1("Can't create memory storage for eye detection.");
		return false;
	}
	return true;
}

bool CFaceAlign::HaarFaceDetect2( IplImage *input )
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
	return true;
}

bool CFaceAlign::HaarEyeDetect( IplImage *input, CvPoint2D32f *eye ) // 检测一只眼的中心位置
{
	CvSeq *pEyes = cvHaarDetectObjects(
		input, m_EDcascade, m_EDstorage,
		1.3, 3,
		0 |
		CV_HAAR_DO_CANNY_PRUNING	|
		//CV_HAAR_FIND_BIGGEST_OBJECT	|
		//CV_HAAR_DO_ROUGH_SEARCH		|
		0,
		cvSize(20, 20));

	if (pEyes && pEyes->total > 0) // 将几个结果平均
	{
		eye->x = eye->y = 0;
		for (int i = 0; i < (pEyes ? pEyes->total : 0); i++)
		{
			CvRect *rc1 = (CvRect *)cvGetSeqElem(pEyes, i);
			eye->x += cvRound(rc1->x + rc1->width / 2);
			eye->y += cvRound(rc1->y + rc1->height / 2);
		}
		eye->x /= pEyes->total;
		eye->y /= pEyes->total;
		return true;
	}
	return false;
}

bool CFaceAlign::GetFace( IplImage *input, CvMat *faceImg, bool useBuf )
{
	if (! HaarFaceDetect2(input))
		return false;

	// 分别检测左眼和右眼
	CvRect rcLeft = cvRect(m_rcCurFace.x, m_rcCurFace.y, 
		m_rcCurFace.width/2, m_rcCurFace.height/2);
	cvSetImageROI(input, rcLeft);
	if (HaarEyeDetect(input, &leftEye))
	{
		leftEye.x += m_rcCurFace.x;
		leftEye.y += m_rcCurFace.y;
	}	
	cvResetImageROI(input);

	CvRect rcRight = cvRect(m_rcCurFace.x + m_rcCurFace.width/2, m_rcCurFace.y, 
		m_rcCurFace.width/2, m_rcCurFace.height/2);
	cvSetImageROI(input, rcRight);
	if (HaarEyeDetect(input, &rightEye))
	{
		rightEye.x += m_rcCurFace.x + m_rcCurFace.width/2;
		rightEye.y += m_rcCurFace.y;
	}
	cvResetImageROI(input);
	
	bool ret;
	if (rightEye.x - leftEye.x < m_rcCurFace.width/5)
		ret = false;
	else ret = EyeCoord2FaceCrop(input, faceImg, useBuf);
	return ret;
}

bool CFaceAlign::EyeCoord2FaceCrop( IplImage * input, CvMat * faceImg, bool useBuf )
{
	static CvPoint2D32f lefta[m_nFiltLevel], righta[m_nFiltLevel];
	static int			idx;
	CvPoint2D32f		l1 = cvPoint2D32f(0,0), r1 = cvPoint2D32f(0,0);
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
	CvMat *tmpDst = cvCreateMat(input->height, input->width, CV_8UC1);
	cv2DRotationMatrix(l1, m_angle, normDis/m_dis, map); // 相似变换
	//DispCvArr(map, "map");
	cvWarpAffine(input, tmpDst, map);

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
