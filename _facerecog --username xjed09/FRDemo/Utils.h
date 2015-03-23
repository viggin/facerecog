/* 
	一些操作窗口的函数
*/

#pragma once
#include "stdafx.h"
#include "../FaceMngr/FaceMngr.h"


// 创建一个名为strWndName的OpenCV Window，将其嵌入到pWnd，并调整尺寸
void EmbedCvWindow(HWND pWnd, CString strWndName, int w, int h);


/*
	选择目录
	hWnd: could be NULL
	strTitle: 窗口标题
	[IN][OUT] CString &strDir: 默认绝对路径和选择得到的目录
*/
bool SelDirectory ( HWND hWnd, LPCTSTR strTitle, CString &strDir );


/*
	从folder中找出所有第一个字符不是!的文件名，存入list，供 FaceMngr::Train 调用。
	根据文件名解析出name，name相同的为一个class，classId同样存入list，训练时使用。
*/
void GenFileList(CString folder, vector<SFInfo> &list);
void GenFileListFromFile( CString fn, vector<SFInfo> &list );


/*
	根据文件名提取人名，规则是：文件名中最左边的下划线左边的字符；
	如果没有下划线，则是文件名中最右边的点左边字符；还没有，则是全部文件名。
	即图片的命名规则是：{name}_{something}.ext，比如：张三_0.jpg，张三_smile.bmp……
*/
CString FindName(CString fn);


//inline void operator /=(CvSize &a, double b)
//{
//	a.height = int(a.height / b);
//	a.width = int(a.width / b);
//}
//
//inline void operator *=(CvSize &a, double b)
//{
//	a.height = int(a.height * b);
//	a.width = int(a.width * b);
//}
//
//inline CvSize operator *(CvSize a, double b)
//{
//	return cvSize(int(a.width * b), int(a.height * b));
//}
//
//inline CvSize operator /(CvSize a, double b)
//{
//	return cvSize(int(a.width / b), int(a.height / b));
//}
//
//void tic(); // start timer
//
//double toc(); // return the elapsed seconds from the last TIC
