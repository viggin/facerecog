/* 
	Some utilities, like OpenCV image processing and file handling
	implemented in cvTools.cpp and FreqTools.cpp
*/

#pragma once

#include "PublicHeaders.h"

// Judge if an IplImage is grayscale, or a CvMat only has 1 channel
#define CV_IS_GRAY(arr)	(	(CV_IS_IMAGE_HDR(arr) && ((IplImage*)(arr))->nChannels == 1)	||	\
							(CV_IS_MAT_HDR(arr) && CV_MAT_CN(((CvMat*)(arr))->type) == 1)	)


// Wrapper of MessageBox
DLLEXP int MessageBox1(LPCTSTR lpText, UINT uType = MB_OK | MB_ICONINFORMATION, 
				LPCTSTR lpCaption = "message", HWND hWnd = NULL);


// draw a box,circle... on img
DLLEXP void MarkOut(IplImage *img, CvRect *rc, CvRect *parentRc, double scale, TCHAR shape, 
			 CvScalar color = CV_RGB(255, 0, 0), int thickness = 1);


// similar to fftshift in MATLAB£¬exchange 4 quadrants in the spectrum, 
// so as to move the low freq to the center
DLLEXP void fftShift(CvArr *src_arr, CvArr *dst_arr);


/*
	return a filter kernel
	filterType: 0:ideal,1:gaussian,2:butterworth
	bHighpass: true: highpass;(not supported) false: lowpass
	d0: cut-off freq, independent to kernel size
	order: for butterworth
	h: the output kernel(normalized)
*/
DLLEXP void GenFilterKernel(CvMat *h, int filterType, /*bool bHighpass,*/ double d0, int order = 1);


// transform 8bit 3/1-channel array to floating-point 1-channel
DLLEXP void Arr2gray(CvArr *src0, CvMat *dest);


// 2D fft£¬src must be CvMat with CV_32FC1, does fewer checks
// dest should have 2 channels, could be larger than src.
DLLEXP void fft2_M32FC1(CvMat *src, CvMat *dest);


// calc the magnitude of a 2-channel complex array 
DLLEXP void Magnitude(CvArr *input, CvMat *mag);


// img is a 8bit or floating-point 2-channel array, show the magnitude of the spectrum
DLLEXP void ShowMagnitude(CvArr *img, bool has_done_dft);


// Filtering in freq domain.
// kr: radius of the origin kernel(window width is kr*2+1)
// kernelF: kernel in freq domain, width should be 
// cvGetOptimalDFTSize( max(pic->rows, pic->cols) + kr*2 ), the extra kr*2 is for eliminating
// the effect of cyclic convolution.
// res: 1/2-channel
void filter_freq_strict(CvMat *pic, CvMat *kernelF, int kr, CvMat *res);


// Use TRACE to print CvArr in debug mode
// for 1D/2D 1 channel mat/img
DLLEXP void DispCvArr(CvArr *a, TCHAR *varName = "CvArr", bool showTranspose = false, TCHAR *format = NULL);


// for 1D/2D N channel mat/img
DLLEXP void DispCvArrMultiChannel(CvArr *a, TCHAR *varName = "CvArr"); 


/*
	concatenate all the arrays in pppa
	e.g. 
	CvMat *m00,*m01,...
	CvMat ***pppa = new CvMat **[2];
	pppa[0] = {m00, m01};
	pppa[1] = {m10, m11, m12};
	=> dst = [m00,m01; m10,m11,m12];
	if every thing is all right.
	If colNum is set, the number of columns of each row is equal, colNum can be NULL.
	Or colNum should be 0.
*/
DLLEXP void ConcatArrs( CvArr ***pppa, CvMat *dst, int rowNum, int colNum, int *colNums = NULL);


// check if the matrices in pppa can be concatenated.
// if so, total size is returned. Or,the program will be stopped.
DLLEXP CvSize CheckConcat(CvArr ***pppa, int rowNum, int colNum, int *colNums);


// File handling
DLLEXP void WriteStringLine(ofstream &os, CString str);
DLLEXP void ReadStringLine(ifstream &is, CString &str);
DLLEXP void WriteIntText(ofstream &os, int n, CString name = "");
DLLEXP void ReadIntText(ifstream &is, int &n, bool hasName = true);

// note: MAT shouldn't be a sub-matrix returned by cvGetSubRect.
DLLEXP void WriteCvMatBin(ofstream &os, CvMat *mat, CString name = "");
DLLEXP void ReadCvMatBin(ifstream &is, CvMat *mat, bool hasName = true);

inline void operator /=(CvSize &a, double b)
{
	a.height = int(a.height / b);
	a.width = int(a.width / b);
}

inline void operator *=(CvSize &a, double b)
{
	a.height = int(a.height * b);
	a.width = int(a.width * b);
}

inline CvSize operator *(CvSize a, double b)
{
	return cvSize(int(a.width * b), int(a.height * b));
}

inline CvSize operator /(CvSize a, double b)
{
	return cvSize(int(a.width / b), int(a.height / b));
}

DLLEXP void tic(); // start timer

DLLEXP double toc(); // return the elapsed seconds from the last TIC