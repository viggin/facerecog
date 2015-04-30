#pragma once


// 'fopen': This function or variable may be unsafe. in opencv2/flann/logger.h(66)
#pragma warning(disable: 4996)

// OpenCV 2.2
#include <opencv\cv.h>
#include <opencv\highgui.h>

#ifdef _DEBUG
	#pragma comment(lib,"opencv_core245d.lib")
	#pragma comment(lib,"opencv_highgui245d.lib")
	#pragma comment(lib,"opencv_imgproc245d.lib")
	#pragma comment(lib,"opencv_objdetect245d.lib")
#else
	#pragma comment(lib,"opencv_core245.lib")
	#pragma comment(lib,"opencv_highgui245.lib")
	#pragma comment(lib,"opencv_imgproc245.lib")
	#pragma comment(lib,"opencv_objdetect245.lib")
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
