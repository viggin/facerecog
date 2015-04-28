/*
	 The strategy of Yan's method is: homographic filtering, then do histogram normalization.
*/

#define DLLSRC

#include "LightPrep.h"
#include "../tools/AlgorithmSelect.h"

#ifdef COMPILE_LIGHT_YAN


#define MASK_FN		"mask.bmp"
#define HISTMD_FN	"histModel.bmp" // the target image for doing histogram normalization

static CvMat	*g_tfaceImg32;
static CvMat	*g_h; // the Gaussian highpass kernel for HomographicFilter

static CvMat	*g_mask, *g_invMask;

// for HistNorm
static CvHistogram	*g_histdst, *g_histsrc;
static CvMat		*g_lutEq2Dst, *g_lutSrc2Eq;
static double		g_scale;

void InitFilterKernel(CvSize imgSz);

bool InitMask(CvSize imgSz, bool useMask);

bool InitHistNorm(CvSize imgSz);

// img must be CV_F32C1
void HomographicFilter(CvMat *img);

// derived from http://www.opencv.org.cn/forum/viewtopic.php?f=1&t=7055
// only for 8UC1
void HistNorm(CvArr *src);

// temporarily not in use
void MaskFace(CvArr *src);


bool InitLight( CvSize faceSz )
{
	bool useMask = true;
	g_tfaceImg32 = cvCreateMat(faceSz.height, faceSz.width, CV_32FC1);
	InitFilterKernel(faceSz);
	return (InitMask(faceSz, useMask) && InitHistNorm(faceSz));
}

void InitFilterKernel( CvSize faceSz )
{
	double	d0 = 1.414*1;
	int hRadius = (faceSz.width + faceSz.height) /2/10;
	double	f_high = 1, f_low = .4;
	g_h = cvCreateMat(hRadius*2+1, hRadius*2+1, CV_32FC1);
	GenFilterKernel(g_h, 1, d0); // gauss kernel
	cvScale(g_h, g_h, -(f_high - f_low), 0); // derived from a downloaded MATLAB homographic filter function
	cvmSet(g_h, hRadius, hRadius, cvmGet(g_h, hRadius, hRadius)+f_high);
	//DispCvArr(g_h, "g_h");
}

bool InitMask( CvSize faceSz, bool useMask )
{
	if (! useMask)
	{
		g_mask = NULL;
		return true;
	}

	IplImage *maskOri = cvLoadImage(MASK_FN, CV_LOAD_IMAGE_GRAYSCALE);
	if(!maskOri) {
		CString msg;
		msg.Format("Can't load mask image %s.", MASK_FN);
		::MessageBox1(msg);
		return false;
	}

	g_mask = cvCreateMat(faceSz.height, faceSz.width, CV_8UC1);
	g_invMask = cvCreateMat(faceSz.height, faceSz.width, CV_8UC1);
	cvResize(maskOri, g_mask);
	cvCmpS(g_mask, 0, g_invMask, CV_CMP_EQ); // È¡·´
	//DispCvArr(g_mask,"g_mask");

	cvReleaseImage(&maskOri);
	return true;
}

bool InitHistNorm( CvSize faceSz )
{
	int histSz = 256;
	//static float g_bins[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	//	1,0,0,1,1,1,0,3,1,0,3,3,1,1,1,1,1,2,1,1,0,2,3,0,0,5,0,4,6,4,8,4,1,6,3,5,2,3,9,7,6,11,6,
	//	9,9,8,12,17,12,15,21,13,15,23,24,23,26,26,32,34,32,35,32,32,21,32,35,35,42,48,41,38,47,
	//	32,48,38,36,35,57,60,55,46,50,45,56,41,57,60,49,65,46,66,59,76,58,64,59,79,74,78,92,96,
	//	80,84,76,82,86,78,79,85,66,69,74,57,55,58,57,53,68,48,42,51,47,41,36,35,40,36,25,35,26,
	//	43,32,30,33,36,19,27,31,17,19,29,25,23,20,14,18,20,22,19,20,16,21,17,11,12,7,11,6,6,1,
	//	3,1,5,0,1,1,0,0,0,0,0,0,1,0,0,0,0,1,0,1,0,0,0,1,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,
	//	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	////g_histdst = cvCreateHist(1, &sz, CV_HIST_ARRAY);
	//g_histdst = new CvHistogram;
	//cvMakeHistHeaderForArray(1, &sz, g_histdst, g_bins);
	IplImage *histModel = cvLoadImage(HISTMD_FN, CV_LOAD_IMAGE_GRAYSCALE);
	if(!histModel) {
		CString msg;
		msg.Format("Can't load histogram model image %s.", HISTMD_FN);
		::MessageBox1(msg);
		return false;
	}

	g_histdst = cvCreateHist(1, &histSz, CV_HIST_ARRAY);
	cvCalcArrHist((CvArr **)&histModel, g_histdst, 0, 
		(g_mask && g_mask->cols == histModel->width && g_mask->rows == histModel->height) ? g_mask : NULL);
	cvNormalizeHist(g_histdst, 1);
	cvReleaseImage(&histModel);

	CvMat *lutSumDst = cvCreateMat(1, histSz, CV_8UC1);
	g_lutEq2Dst = cvCreateMat(1, histSz, CV_8UC1);
	float	*histOfDst = (float *)cvPtr1D(g_histdst->bins, 0);
	float	sum = 0;
	for(int i = 0; i < 256; i++)
	{
		sum += histOfDst[i]*255;
		lutSumDst->data.ptr[i] = (uchar)cvRound(sum);
	}
	for (int i = 0; i < 256; i++)
	{
		int k = 0;
		while(lutSumDst->data.ptr[k] < i) k++;
		g_lutEq2Dst->data.ptr[i] = k;
	}
	cvReleaseMat(&lutSumDst);

	g_scale = 255.0/(faceSz.width * faceSz.height);
	g_histsrc = cvCreateHist(1, &histSz, CV_HIST_ARRAY); // src's hist
	g_lutSrc2Eq = cvCreateMat(1, 256, CV_8UC1);

	return true;
}

void RunLightPrep( CvMat *faceImg8 )
{
	cvConvertScale(faceImg8, g_tfaceImg32, 1.0/255);
	HomographicFilter(g_tfaceImg32);
	cvConvertScale(g_tfaceImg32, faceImg8, 255);
	HistNorm(faceImg8);
}

void HomographicFilter(CvMat *img)
{
	//CvMat	*dft_src = 0;
	
	cvAddS(img, cvScalar(.01), img);
	cvLog(img, img);

	/*dft_src = cvCreateMat(dft_height, dft_width, CV_32FC2);
	fft2_M32FC1(img, dft_src);	
 	cvMulSpectrums(dft_src, H, dft_src, NULL);
	cvDFT(dft_src, dft_src, CV_DXT_INVERSE_SCALE, img->height);*/

	/*CvMat tmp;
	cvGetSubRect(dft_src, &tmp, cvRect(0,0, img->width, img->height));
	cvSplit(&tmp, img, NULL, NULL, NULL);
	cvReleaseMat(&dft_src);*/

	//cvCopyMakeBorder(img, padded, cvPoint(g_hRadius, g_hRadius) , IPL_BORDER_REPLICATE);
	cvFilter2D(img, img, g_h);
	cvExp(img, img);
	cvNormalize(img, img, 0,1, CV_MINMAX);
}

void HistNorm(CvArr *src)
{
	float	*histOfSrc;
	float	sum = 0;

	cvCalcArrHist((CvArr **)&src, g_histsrc, 0, g_mask);
	histOfSrc = (float *)cvPtr1D(g_histsrc->bins, 0);
	
	for(int i = 0; i < 256; i++)
	{
		sum += histOfSrc[i];
		g_lutSrc2Eq->data.ptr[i] = (uchar)cvRound(sum*g_scale);
	}
	g_lutSrc2Eq->data.ptr[0] = 0;

	cvLUT(src, src, g_lutSrc2Eq);
	cvLUT(src, src, g_lutEq2Dst);
}

void MaskFace( CvArr *src )
{
	cvSet(src, cvScalar(0), g_invMask);
}

void ReleaseLight()
{
	cvReleaseMat(&g_h);
	cvReleaseMat(&g_lutEq2Dst);
	cvReleaseHist(&g_histdst);
	cvReleaseHist(&g_histsrc);
	cvReleaseMat(&g_lutSrc2Eq);
	cvReleaseMat(&g_mask);
	cvReleaseMat(&g_invMask);
	cvReleaseMat(&g_tfaceImg32);
}

#endif