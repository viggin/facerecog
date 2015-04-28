#pragma once


// 'fopen': This function or variable may be unsafe. in opencv2/flann/logger.h(66)
#pragma warning(disable: 4996)

// OpenCV 2.2
#include <opencv\cv.h>
#include <opencv\highgui.h>

#ifdef _DEBUG
	#pragma comment(lib,"opencv_core220d.lib")
	#pragma comment(lib,"opencv_highgui220d.lib")
	#pragma comment(lib,"opencv_imgproc220d.lib")
	#pragma comment(lib,"opencv_objdetect220d.lib")
#else
	#pragma comment(lib,"opencv_core220.lib")
	#pragma comment(lib,"opencv_highgui220.lib")
	#pragma comment(lib,"opencv_imgproc220.lib")
	#pragma comment(lib,"opencv_objdetect220.lib")
#endif


#ifdef DLLSRC
	#define DLLEXP extern "C" __declspec(dllexport)
#else
	#define DLLEXP extern "C" __declspec(dllimport)
#endif

// This notation in the code indicates that 
// the variable is a tunable parameter of the algorithm.
#define PARAM

#include <vector>
#include <fstream>
using namespace std;

#include <atlstr.h>
#ifdef _DEBUG
#include <atltrace.h>
#define TRACE ATLTRACE
#endif
