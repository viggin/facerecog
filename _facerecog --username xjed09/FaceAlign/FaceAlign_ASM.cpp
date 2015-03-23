/*
	first uses VJ object detection algorithm in OpenCV for rough face detection,
	then uses Yao Wei's Active Shape Model to detect eye coordinates,
	finally call EyeCoord2FaceCrop for face alignment. For Yao Wei's work, ref to:

	YAO Wei. Research on Facial Expression Recognition and Synthesis.
	<EM> Master Thesis, Department of Computer Science and Technology, 
	Nanjing University</EM>, Feb 2009. \url http://code.google.com/p/asmlibrary
*/

#define DLLSRC

#include "FaceAlign.h"
#include "../tools/AlgorithmSelect.h"

#ifdef COMPILE_ALIGN_ASM

// files needed
#define FDFN	"haarcascade_frontalface_alt2.xml"
#define ASMFN	"AsmModel.amf"

#include "asmfitting.h"

#pragma comment(lib, "asmlibrary.lib")

#ifdef _DEBUG
#pragma comment(lib,"opencv_objdetect220d.lib")
#else
#pragma comment(lib,"opencv_objdetect220.lib")
#endif

static CvSize	g_faceSz;		// the size of the aligned face
static int		g_normRow, g_normDis;	// the eye's position of the aligned face
static int		g_nFiltLevel;			// to remove the noise in the detected eye position, use average position of g_nFiltLevel frames

static CvPoint2D32f		*g_lefta, *g_righta;
static float			g_angle, g_dis; // angle and distance between 2 eyes
static CvRect			g_faceRc;

static asmfitting		g_AsmFit;
static asm_shape		g_detshape, g_shape;

CvHaarClassifierCascade	*g_FDcascade;
CvMemStorage			*g_FDstorage;

// declaration of functions which are not the interface of the dll(DLLEXP)
bool InitAsm();
bool AsmDetectFeaturePoints(IplImage *pic8);


bool InitAlign( CvSize *faceSz)
{
	PARAM *faceSz = cvSize(68,84); // set to return the size of the aligned face
	PARAM g_normRow = 27;
	PARAM g_normDis = 34;

	g_faceSz = *faceSz;
	g_nFiltLevel = 3;

	g_lefta = new CvPoint2D32f[g_nFiltLevel];
	g_righta = new CvPoint2D32f[g_nFiltLevel];
	return InitAsm();
}

bool InitAsm()
{
	if(! g_AsmFit.Read(ASMFN)) {
		CString msg;
		msg.Format("Can't load ASM model file %s.", ASMFN);
		//::MessageBox(NULL, msg, "message", MB_OK | MB_ICONWARNING);
		MessageBox1(msg);
		return false;
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

DLLEXP bool GetFace( IplImage *pic8, CvMat *faceImg8, bool useBuf, 
					CvPoint2D32f &leftEye /*= cvPoint2D32f(0,0)*/, CvPoint2D32f &rightEye /*= cvPoint2D32f(0,0)*/ )
{
	if (leftEye.x <= 0 || leftEye.y <= 0 ||
		rightEye.x <= 0 || rightEye.y <= 0)
	{
		if (! AsmDetectFeaturePoints(pic8)) return false;
		leftEye.x = g_shape[31].x;
		leftEye.y = g_shape[31].y;
		rightEye.x = g_shape[36].x;
		rightEye.y = g_shape[36].y;
	}
	
	return EyeCoord2FaceCrop(pic8, faceImg8, leftEye, rightEye, useBuf);
}

SFacePose GetFacePose()
{
	SFacePose fp;
	fp.eyeAngle = g_angle;
	fp.eyeDis = g_dis;
	fp.rc = g_faceRc;
	return fp;
}

DLLEXP void ReleaseAlign()
{
	delete []g_lefta;
	delete []g_righta;
	cvReleaseHaarClassifierCascade(&g_FDcascade);
	cvReleaseMemStorage(&g_FDstorage);
}

bool AsmDetectFeaturePoints( IplImage *pic8 )
{
	int nFaces;
	CvSeq *pFaces;
	bool bResize = false;
	IplImage *picCopy = pic8;
	if (pic8->width > 700 || pic8->height > 700) // if the picture is too big
	{
		pic8 = cvCreateImage(cvGetSize(pic8)/2, 8, 1);
		cvPyrDown(picCopy, pic8);
		bResize = true;
	}

	pFaces = cvHaarDetectObjects(
		pic8, g_FDcascade, g_FDstorage,
		1.1,
		3,
		0 |
		CV_HAAR_DO_CANNY_PRUNING	|
		CV_HAAR_FIND_BIGGEST_OBJECT	|
		//CV_HAAR_DO_ROUGH_SEARCH		|
		0,
		cvSize(20, 20));
	nFaces = pFaces->total;

	if (nFaces == 0) // use a looser standard to detect again
	{
		pFaces = cvHaarDetectObjects(
			pic8, g_FDcascade, g_FDstorage,
			1.1,
			1, 
			0 |
			CV_HAAR_DO_CANNY_PRUNING	|
			CV_HAAR_FIND_BIGGEST_OBJECT	|
			//CV_HAAR_DO_ROUGH_SEARCH		|
			0,
			cvSize(20, 20));
		nFaces = pFaces->total;
	}
	if (nFaces == 0) 
	{
		if (bResize) cvReleaseImage(&pic8);
		return false;
	}

	g_detshape.Resize(2);
	g_faceRc = *(CvRect*)cvGetSeqElem(pFaces, 0);

	if (bResize)
	{
		g_faceRc.x *= 2;
		g_faceRc.y *= 2;
		g_faceRc.width *= 2;
		g_faceRc.height *= 2;
	}
	g_detshape[0].x = float(g_faceRc.x);
	g_detshape[0].y = float(g_faceRc.y);
	g_detshape[1].x = float(g_faceRc.x+g_faceRc.width);
	g_detshape[1].y = float(g_faceRc.y+g_faceRc.height);
	InitShapeFromDetBox(g_shape, g_detshape,
		g_AsmFit.GetMappingDetShape(), g_AsmFit.GetMeanFaceWidth());

	g_AsmFit.Fitting(g_shape, picCopy); // fit ASM model
	//DrawPoints(pic8, g_shape);

	if (bResize) cvReleaseImage(&pic8);
	return true;
}


bool EyeCoord2FaceCrop( IplImage * pic8, CvMat * faceImg8,  CvPoint2D32f leftEye, 
					   CvPoint2D32f rightEye, bool useBuf )
{
	static int idx = 0;
	static bool bInited = false;
	CvPoint2D32f l1 = cvPoint2D32f(0,0), r1 = cvPoint2D32f(0,0);
	if (useBuf) // when detect face in a video stream, one may hope the position changes more smoothly. So we smooth the eye coordinates between each call
	{
		g_lefta[idx] = leftEye;
		g_righta[idx++] = rightEye;
		idx %= g_nFiltLevel;

		if (!bInited)
		{
			for (int i = 1; i < g_nFiltLevel; i++)
			{
				g_lefta[i] = leftEye;
				g_righta[i] = rightEye;
			}
		}

		for (int i = 0; i < g_nFiltLevel; i++) // smooth the coordinates
		{
			l1.x += g_lefta[i].x/g_nFiltLevel;
			l1.y += g_lefta[i].y/g_nFiltLevel;
			r1.x += g_righta[i].x/g_nFiltLevel;
			r1.y += g_righta[i].y/g_nFiltLevel;
		}
	}
	else
	{
		l1 = leftEye;
		r1 = rightEye;
	}

	float xDis = r1.x - l1.x,
		yDis = r1.y - l1.y;

	g_angle = cvFastArctan(yDis, xDis);
	g_dis = sqrt(xDis*xDis + yDis*yDis);


	CvMat *map = cvCreateMat(2, 3, CV_32FC1);
	CvMat *largePic8 = cvCreateMat(pic8->height*2, pic8->width*2, CV_8UC1); // in case the cropped face goes out of the border
	CvMat *tmpDst = cvCreateMat(largePic8->height, largePic8->width, CV_8UC1);
	cvCopyMakeBorder(pic8, largePic8, cvPoint(pic8->width/2, pic8->height/2), IPL_BORDER_REPLICATE);

	l1.x += pic8->width/2;
	l1.y += pic8->height/2;
	cv2DRotationMatrix(l1, g_angle, g_normDis/g_dis, map); // similar transformation
	//DispCvArr(map, "map");
	cvWarpAffine(largePic8, tmpDst, map);
	//cvShowImage("a",tmpDst);
	//cvWaitKey();

	int		leftEyeXNew = cvRound((g_faceSz.width - g_normDis)/2);
	int		left = cvRound(l1.x - leftEyeXNew),
		top = cvRound(l1.y - g_normRow);
	CvMat	tmpHeader, *sub = 0;

	if (left >= 0 && top >= 0 &&
		left + g_faceSz.width <= tmpDst->width &&
		top + g_faceSz.height <= tmpDst->height)
	{	
		sub = cvGetSubRect(tmpDst, &tmpHeader, cvRect(left, top, g_faceSz.width, g_faceSz.height));
		cvCopy(sub, faceImg8);
		//cvShowImage("f",faceImg8);
		//cvWaitKey();
	}

	cvReleaseMat(&map);
	cvReleaseMat(&largePic8);
	cvReleaseMat(&tmpDst);
	return (sub != 0);
}

#endif
