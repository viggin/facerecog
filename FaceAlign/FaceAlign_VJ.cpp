/*
	uses VJ object detection algorithm in OpenCV for face alignment
*/

#define DLLSRC

#include "FaceAlign.h"
#include "../tools/AlgorithmSelect.h"

#ifdef COMPILE_ALIGN_VJ

// files needed
#define FDFN	"haarcascade_frontalface_alt2.xml"

static CvSize	g_faceSz;
static CvRect	g_faceRc;
static CvPoint2D32f	g_margin;
static CvSize2D32f	g_resize;
static int	g_nFiltLevel;
static CvRect *history;

CvHaarClassifierCascade	*g_FDcascade;
CvMemStorage			*g_FDstorage;


bool InitAlign( CvSize *faceSz )
{
	*faceSz = cvSize(68,84);
	g_faceSz = *faceSz;
	PARAM g_resize = cvSize2D32f(.81,1); // coefs to adjust the CvRect returned by OpenCV face detector
	PARAM double heightShift = .1;
	g_margin = cvPoint2D32f((1-g_resize.width)/2, heightShift);
	g_nFiltLevel = 3;
	history = new CvRect[g_nFiltLevel];
	for (int i = 0; i < g_nFiltLevel; i++)
	{
			history[i] = cvRect(0,0,0,0);
	}

	g_FDcascade = (CvHaarClassifierCascade*)cvLoad(FDFN, 0, 0, 0);
	if (!g_FDcascade) {
		CString msg;
		msg.Format("Can't load %s.", _T(FDFN));
		::MessageBox1(msg);
		return false;
	}

	g_FDstorage = cvCreateMemStorage(0);
	if (!g_FDstorage) {
		::MessageBox1("Can't create memory storage for face detection.");
		return false;
	}
	return true;
}

bool GetFace( IplImage *pic8, CvMat *faceImg8, bool useBuf,
			 CvPoint2D32f &leftEye /*= cvPoint2D32f(0,0)*/, CvPoint2D32f &rightEye /*= cvPoint2D32f(0,0)*/)
{
	CvSeq *pFaces = cvHaarDetectObjects(
		pic8, g_FDcascade, g_FDstorage,
		1.3,
		3,
		0 |
		CV_HAAR_DO_CANNY_PRUNING	|
		CV_HAAR_FIND_BIGGEST_OBJECT	|
		CV_HAAR_DO_ROUGH_SEARCH		|
		0,
		cvSize(20, 20));

	if (!pFaces || pFaces->total == 0)
		return false;
	g_faceRc = *(CvRect *)cvGetSeqElem(pFaces, 0);

	
	//static CvRect	*history = new CvRect[g_nFiltLevel];
	static int idx;
	if (useBuf)
	{
		history[idx++] = g_faceRc;
		g_faceRc = cvRect(0,0,0,0);
		idx %= g_nFiltLevel;
		for (int i = 0; i < g_nFiltLevel; i++)
		{
			g_faceRc.x += history[i].x/g_nFiltLevel;
			g_faceRc.y += history[i].y/g_nFiltLevel;
			g_faceRc.width += history[i].width/g_nFiltLevel;
			g_faceRc.height += history[i].height/g_nFiltLevel;
		}
	}

	g_faceRc.height = int(g_resize.height*g_faceRc.height);
	g_faceRc.width = int(g_resize.width*g_faceRc.width);
	g_faceRc.x += int(g_margin.x*g_faceRc.width);
	g_faceRc.y += int(g_margin.y*g_faceRc.height);
	

	CvMat sub;
	cvGetSubRect(pic8, &sub, g_faceRc);
	//cvNamedWindow("test");
	//cvShowImage("test",&sub);
	cvResize(&sub, faceImg8);
	return true;
}

SFacePose GetFacePose()
{
	SFacePose fp;
	fp.eyeAngle = 0;
	fp.eyeDis = float(g_faceRc.width)*3/7;
	fp.rc = g_faceRc;
	return fp;
}

void ReleaseAlign()
{
	delete []history;
	cvReleaseHaarClassifierCascade(&g_FDcascade);
	cvReleaseMemStorage(&g_FDstorage);
}

#endif
