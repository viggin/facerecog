#define DLLSRC

#include "LightPrep.h"


CLightPrep::CLightPrep()
{
	h = lutEq2Dst = m_lutSrc2Eq = m_mask = m_invMask = tfaceImg32 = NULL;
	m_histdst = m_histsrc = NULL;
}

CLightPrep::~CLightPrep()
{
	Release();
}

void CLightPrep::Release()
{
	cvReleaseMat(&h);
	cvReleaseMat(&lutEq2Dst);
	cvReleaseHist(&m_histdst);
	cvReleaseHist(&m_histsrc);
	cvReleaseMat(&m_lutSrc2Eq);
	cvReleaseMat(&m_mask);
	cvReleaseMat(&m_invMask);
	cvReleaseMat(&tfaceImg32);
}

bool CLightPrep::Init( CvSize imgSz, bool useMask )
{
	tfaceImg32 = cvCreateMat(imgSz.height, imgSz.width, CV_32FC1);
	InitFilterKernel(imgSz);
	return (InitMask(imgSz, useMask) && InitHistNorm(imgSz));
}

void CLightPrep::InitFilterKernel( CvSize imgSz )
{
	double	d0 = 1.414*1;
	h_radius = (imgSz.width + imgSz.height) /2/10;
	double	f_high = 1, f_low = .4;
	h = cvCreateMat(h_radius*2+1, h_radius*2+1, CV_32FC1);
	GenFilterKernel(h, 1, d0); // gauss kernel
	cvScale(h, h, -(f_high - f_low), 0); // derived from a downloaded MATLAB homographic filter function
	cvmSet(h, h_radius, h_radius, cvmGet(h, h_radius, h_radius)+f_high);
	//DispCvArr(h, "h");
}

bool CLightPrep::InitMask( CvSize imgSz, bool useMask )
{
	if (! useMask)
	{
		m_mask = NULL;
		return true;
	}

	IplImage *maskOri = cvLoadImage(MASK_FN, CV_LOAD_IMAGE_GRAYSCALE);
	if(!maskOri) {
		CString msg;
		msg.Format("Can't load mask image %s.", MASK_FN);
		::MessageBox1(msg);
		return false;
	}

	m_mask = cvCreateMat(imgSz.height, imgSz.width, CV_8UC1);
	m_invMask = cvCreateMat(imgSz.height, imgSz.width, CV_8UC1);
	cvResize(maskOri, m_mask);
	cvCmpS(m_mask, 0, m_invMask, CV_CMP_EQ); // È¡·´
	//DispCvArr(m_mask,"m_mask");

	cvReleaseImage(&maskOri);
	return true;
}

bool CLightPrep::InitHistNorm( CvSize imgSz )
{
	int histSz = 256;
	//static float m_bins[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	//	1,0,0,1,1,1,0,3,1,0,3,3,1,1,1,1,1,2,1,1,0,2,3,0,0,5,0,4,6,4,8,4,1,6,3,5,2,3,9,7,6,11,6,
	//	9,9,8,12,17,12,15,21,13,15,23,24,23,26,26,32,34,32,35,32,32,21,32,35,35,42,48,41,38,47,
	//	32,48,38,36,35,57,60,55,46,50,45,56,41,57,60,49,65,46,66,59,76,58,64,59,79,74,78,92,96,
	//	80,84,76,82,86,78,79,85,66,69,74,57,55,58,57,53,68,48,42,51,47,41,36,35,40,36,25,35,26,
	//	43,32,30,33,36,19,27,31,17,19,29,25,23,20,14,18,20,22,19,20,16,21,17,11,12,7,11,6,6,1,
	//	3,1,5,0,1,1,0,0,0,0,0,0,1,0,0,0,0,1,0,1,0,0,0,1,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,
	//	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	////m_histdst = cvCreateHist(1, &sz, CV_HIST_ARRAY);
	//m_histdst = new CvHistogram;
	//cvMakeHistHeaderForArray(1, &sz, m_histdst, m_bins);
	IplImage *histModel = cvLoadImage(HISTMD_FN, CV_LOAD_IMAGE_GRAYSCALE);
	if(!histModel) {
		CString msg;
		msg.Format("Can't load histogram model image %s.", HISTMD_FN);
		::MessageBox1(msg);
		return false;
	}

	m_histdst = cvCreateHist(1, &histSz, CV_HIST_ARRAY);
	cvCalcArrHist((CvArr **)&histModel, m_histdst, 0, 
		(m_mask && m_mask->cols == histModel->width && m_mask->rows == histModel->height) ? m_mask : NULL);
	cvNormalizeHist(m_histdst, 1);
	cvReleaseImage(&histModel);

	CvMat *lutSumDst = cvCreateMat(1, histSz, CV_8UC1);
	lutEq2Dst = cvCreateMat(1, histSz, CV_8UC1);
	float	*histOfDst = (float *)cvPtr1D(m_histdst->bins, 0);
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
		lutEq2Dst->data.ptr[i] = k;
	}
	cvReleaseMat(&lutSumDst);

	m_scale = 255.0/(imgSz.width * imgSz.height);
	m_histsrc = cvCreateHist(1, &histSz, CV_HIST_ARRAY); // src's hist
	m_lutSrc2Eq = cvCreateMat(1, 256, CV_8UC1);

	return true;
}

void CLightPrep::RunLightPrep( CvMat *faceImg8 )
{
	cvConvertScale(faceImg8, tfaceImg32, 1.0/255);
	HomographicFilter(tfaceImg32);
	cvConvertScale(tfaceImg32, faceImg8, 255);
	HistNorm(faceImg8);
}

void CLightPrep::HomographicFilter(CvMat *img)
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

	//cvCopyMakeBorder(img, padded, cvPoint(h_radius, h_radius) , IPL_BORDER_REPLICATE);
	cvFilter2D(img, img, h);

	cvExp(img, img);
	cvNormalize(img, img, 0,1, CV_MINMAX);
}

void CLightPrep::HistNorm(CvArr *src)
{
	float	*histOfSrc;
	float	sum = 0;

	cvCalcArrHist((CvArr **)&src, m_histsrc, 0, m_mask);
	histOfSrc = (float *)cvPtr1D(m_histsrc->bins, 0);
	
	for(int i = 0; i < 256; i++)
	{
		sum += histOfSrc[i];
		m_lutSrc2Eq->data.ptr[i] = (uchar)cvRound(sum*m_scale);
	}
	m_lutSrc2Eq->data.ptr[0] = 0;

	cvLUT(src, src, m_lutSrc2Eq);
	cvLUT(src, src, lutEq2Dst);
}

void CLightPrep::MaskFace( CvArr *src )
{
	cvSet(src, cvScalar(0), m_invMask);
}
