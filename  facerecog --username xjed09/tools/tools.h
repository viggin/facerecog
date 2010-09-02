/* 
	一些比较通用的没有用到MFC的函数，供算法模块调用。
	实现在cvTools.cpp和FreqTools.cpp
*/

#pragma once

#include "../opencv/PublicHeaders.h"

// 判断IplImage是否是灰度图，或CvMat是否只有一个channel
#define CV_IS_GRAY(arr)	(	(CV_IS_IMAGE_HDR(arr) && ((IplImage*)(arr))->nChannels == 1)	||	\
							(CV_IS_MAT_HDR(arr) && CV_MAT_CN(((CvMat*)(arr))->type) == 1)	)

#define VAR_NAME(x)		(#x)


#ifdef __cplusplus
extern "C"{
#endif


// Wrapper of MessageBox
DLLEXP int MessageBox1(LPCTSTR lpText, UINT uType = MB_OK | MB_ICONINFORMATION, 
				LPCTSTR lpCaption = "message", HWND hWnd = NULL);


// 在img中画方框、圈等
DLLEXP void MarkOut(IplImage *img, CvRect *rc, CvRect *parentRc, double scale, char shape, 
			 CvScalar color = CV_RGB(255, 0, 0), int thickness = 1);


// 类似MATLAB的fftshift，将频谱图像的四个象限调换，以使低频挪到图像中央
DLLEXP void fftShift(CvArr *src_arr, CvArr *dst_arr);


/*
	返回滤波器Kernel
	filterType: 滤波器类型,0:ideal,1:gaussian,2:butterworth
	bHighpass: true: highpass;(not supported) false: lowpass
	d0: 截止频率(与滤波器大小无关)
	order: 阶数(butterworth)
	h: 输出的高通滤波器(归一化)
*/
DLLEXP void GenFilterKernel(CvMat *h, int filterType, /*bool bHighpass,*/ double d0, int order = 1);


// 将3通道或1通道的8位图像转换为1通道浮点图像
DLLEXP void Arr2gray(CvArr *src0, CvMat *dest);


// 2维fft，src must be CvMat with CV_32FC1, does fewer checks
// dest should have 2 channels, could be larger than src.
void DLLEXP fft2_M32FC1(CvMat *src, CvMat *dest);


// 计算2通道复数矩阵的幅度
DLLEXP void Magnitude(CvArr *input, CvMat *mag);


// img是8位图像或浮点2通道复数频谱矩阵，对频谱幅度做log变换后显示
DLLEXP void ShowMagnitude(CvArr *img, bool has_done_dft);


// 频域滤波，kr是原滤波核的半径(窗口宽度=kr*2+1)，kernelF是频域的滤波核，长宽应是
// cvGetOptimalDFTSize( max(pic->rows, pic->cols) + kr*2 )，多出来的 kr*2 是为了去除循环卷积影响
// 这个函数主要是针对同一个滤波核，多次滤波的情况，用FFT加速。res可以是1或2通道。
void filter_freq_strict(CvMat *pic, CvMat *kernelF, int kr, CvMat *res);


// Debug模式中，用TRACE在输出窗口显示CvArr
DLLEXP void DispCvArr(CvArr *a, char *varName = VAR_NAME(varName), bool showTranspose = false, char *format = NULL); // for 1D/2D 1 channel mat/img


DLLEXP void DispCvArrMultiChannel(CvArr *a, char *varName = VAR_NAME(varName)); // for 1D/2D N channel mat/img


/*
	concatenate all the arrays in pppa
	e.g. 
	CvMat *m00,*m01,...
	CvMat ***pppa = new CvMat **[2];
	pppa[0] = {m00, m01};
	pppa[1] = {m10, m11, m12};
	=> dst = [m00,m01; m10,m11,m12];
	if every thing is all right.
	若指定了colNum,则表明各行列数相同，colNums可为NULL；否则应使colNum=0
*/
DLLEXP void ConcatArrs( CvArr ***pppa, CvMat *dst, int rowNum, int colNum, int *colNums = NULL);


// 检查pppa中诸矩阵是否能够concatenate,若能则返回总size，否则assert中断
DLLEXP CvSize CheckConcat(CvArr ***pppa, int rowNum, int colNum, int *colNums);


// 文件操作
DLLEXP void WriteStringLine(ofstream &os, CString str);
DLLEXP void ReadStringLine(ifstream &is, CString &str);
DLLEXP void WriteIntText(ofstream &os, int n, CString name = "");
DLLEXP void ReadIntText(ifstream &is, int &n, bool hasName = true);

// 注意mat不应是一个矩阵通过getsubrect方式得到的子矩阵，即mat的step应该等于其 列数*通道数*元素字节数
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

DLLEXP void tic(); // 计时开始

DLLEXP double toc(); // 返回从上次tic到现在的秒数

#ifdef __cplusplus
}
#endif
