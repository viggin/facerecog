#define DLLSRC

#include "tools.h"


void fftShift(CvArr *src_arr, CvArr *dst_arr)
{
	CvMat *tmp = 0;
	CvMat q1stub, q2stub;
	CvMat q3stub, q4stub;
	CvMat d1stub, d2stub;
	CvMat d3stub, d4stub;
	CvMat *q1, *q2, *q3, *q4;
	CvMat *d1, *d2, *d3, *d4;

	CvSize size = cvGetSize(src_arr);
	CvSize dst_size = cvGetSize(dst_arr);
	int cx, cy;

	if(dst_size.width != size.width ||
		dst_size.height != size.height){
			cvError( CV_StsUnmatchedSizes, "fftShift", "Source and Destination arrays must have equal sizes", __FILE__, __LINE__ );
	}

	if(src_arr == dst_arr){
		tmp = cvCreateMat(size.height/2, size.width/2, cvGetElemType(src_arr));
	}

	cx = size.width/2;
	cy = size.height/2; // image center

	q1 = cvGetSubRect( src_arr, &q1stub, cvRect(0,0,cx, cy) );
	q2 = cvGetSubRect( src_arr, &q2stub, cvRect(cx,0,cx,cy) );
	q3 = cvGetSubRect( src_arr, &q3stub, cvRect(cx,cy,cx,cy) );
	q4 = cvGetSubRect( src_arr, &q4stub, cvRect(0,cy,cx,cy) );
	d1 = cvGetSubRect( dst_arr, &d1stub, cvRect(0,0,cx,cy) );
	d2 = cvGetSubRect( dst_arr, &d2stub, cvRect(cx,0,cx,cy) );
	d3 = cvGetSubRect( dst_arr, &d3stub, cvRect(cx,cy,cx,cy) );
	d4 = cvGetSubRect( dst_arr, &d4stub, cvRect(0,cy,cx,cy) );

	if(src_arr!=dst_arr){
		if( !CV_ARE_TYPES_EQ( q1, d1 )){
			cvError( CV_StsUnmatchedFormats, "fftShift", "Source and Destination arrays must have the same format", __FILE__, __LINE__ );
		}
		cvCopy(q3, d1, 0);
		cvCopy(q4, d2, 0);
		cvCopy(q1, d3, 0);
		cvCopy(q2, d4, 0);
	}
	else{
		cvCopy(q3, tmp, 0);
		cvCopy(q1, q3, 0);
		cvCopy(tmp, q1, 0);
		cvCopy(q4, tmp, 0);
		cvCopy(q2, q4, 0);
		cvCopy(tmp, q2, 0);
		cvReleaseMat(&tmp);
	}
}

void GenFilterKernel(CvMat *h, int filterType, /*bool bHighpass,*/ double d0, int order/* = 1*/)
{
	CvSize sz = cvGetSize(h);
	assert(d0 > 0/* && d0 < __min(sz.width/2, sz.height/2)*/);
	assert(order > 0);
	assert(filterType >= 0 && filterType <= 3);

	double	i,j;
	double	r,z;
	for (int y = 0; y < sz.height; y++)
	{
		for (int x = 0; x < sz.width; x++)
		{
			i = x - double(sz.width - 1)/2;
			j = y - double(sz.height - 1)/2;
			r = sqrt(i*i+j*j);
			switch(filterType)
			{
			case 0:
				z = (r < d0);
				break;
			case 1:
				z = exp(-r*r/(2*d0*d0));
				break;
			case 2:
				z = 1/(1+pow(r/d0, 2*filterType));
			}
			//if (bHighpass) z = 1-z; // incorrect for spatial domain
			cvmSet(h, y, x, z);
		}
	}
	CvScalar s = cvSum(h);
	cvScale(h, h, 1./s.val[0]);
}

void Arr2gray(CvArr *src0, CvMat *dest)
{
	CvMat *src = 0, tmp;
	if (CV_IS_IMAGE(src0)) src = cvGetMat(src0, &tmp);
	else src = (CvMat *)src0;

	if (CV_MAT_CN(src->type) == 3)
	{
		CvMat *src_gray = cvCreateMat(src->rows, src->cols, CV_MAKETYPE(CV_MAT_DEPTH(src->type), 1));
		cvCvtColor(src, src_gray, CV_BGR2GRAY);
		cvConvertScale(src_gray, dest, 1.0/255);
		cvReleaseMat(&src_gray);
	}
	else cvConvertScale(src, dest, 1.0/255);
}

void fft2_M32FC1(CvMat *src, CvMat *dest)
{
	CvMat tmp;
	CvMat *srcRe = cvCreateMat(dest->height, dest->width, CV_32FC1),
		*srcIm = cvCreateMat(dest->height, dest->width, CV_32FC1);
	cvGetSubRect(srcRe, &tmp, cvRect(0,0, src->width, src->height));
	cvCopy(src, &tmp);
	if (dest->width - src->width > 0)
	{
		cvGetSubRect( srcRe, &tmp, cvRect(src->width,0, dest->width - src->width, src->height));
		cvSetZero( &tmp );
	}
	cvSetZero(srcIm);
	cvMerge(srcRe, srcIm, NULL, NULL, dest);

	cvDFT(dest, dest, CV_DXT_FORWARD, src->height);
	//DispCvArrMultiChannel(dest, "H");

	cvReleaseMat(&srcRe);
	cvReleaseMat(&srcIm);
}

void Magnitude(CvArr *input, CvMat *mag)
{
	CvSize sz = cvGetSize(input);
	CvMat *re = cvCreateMat(sz.height, sz.width, CV_32FC1),
		*im = cvCreateMat(sz.height, sz.width, CV_32FC1);

	cvSplit(input, re, im, NULL, NULL);

	cvPow(re, re, 2.0);
	cvPow(im, im, 2.0);
	cvAdd(re, im, mag, NULL);
	cvPow(mag, mag, 0.5);

	cvReleaseMat(&re);
	cvReleaseMat(&im);
}

void ShowMagnitude(CvArr *img, bool has_done_dft)
{
	CvMat	*dft_src = 0, *mag = 0;
	CvSize	imgsz = cvGetSize(img);
	dft_src = cvCreateMat(imgsz.height, imgsz.width, CV_32FC2);
	mag = cvCreateMat(imgsz.height, imgsz.width, CV_32FC1);
	if (!has_done_dft) 
	{
		Arr2gray(img, mag);
		fft2_M32FC1(mag, dft_src);
		Magnitude(dft_src, mag);
	}
	else Magnitude(img, mag);

	//cvSplit(img, mag, NULL,NULL,NULL);
	fftShift(mag, mag);
	cvAddS(mag, cvScalarAll(1.0), mag, NULL);
	cvLog(mag, mag);

	cvNormalize(mag, mag, 0,1, CV_MINMAX);
	cvShowImage("magnitude", mag);
	cvWaitKey(0);

	cvReleaseMat(&dft_src);
	cvReleaseMat(&mag);
}

void filter_freq_strict(CvMat *pic, CvMat *kernelF, int kr, CvMat *res)
{
	int fsz = kernelF->cols;
	CvMat *srcRe = cvCreateMat(fsz, fsz, CV_32FC1),
		*srcIm = cvCreateMat(fsz, fsz, CV_32FC1),
		*src = cvCreateMat(fsz, fsz, CV_32FC2);
	cvCopyMakeBorder(pic, srcRe, cvPoint(kr,kr), IPL_BORDER_REPLICATE);
	cvZero(srcIm);
	cvMerge(srcRe, srcIm, NULL, NULL, src);
	cvDFT(src, src, CV_DXT_FORWARD, pic->rows + kr*2);

	cvMulSpectrums(src, kernelF, src, NULL);
	cvDFT(src, srcRe, CV_DXT_INVERSE_SCALE);
	CvMat sub;
	cvGetSubRect(srcRe, &sub, cvRect(kr*2,kr*2, pic->width, pic->height));
	cvCopy(&sub, res); // crop the valid part

	cvReleaseMat(&srcRe);
	cvReleaseMat(&srcIm);
	cvReleaseMat(&src);
}
