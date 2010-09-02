/*
	不能用#pragma once，因为这个头文件会至少进行两次编译，
	当被FaceAlign_ASM.h等头文件包含时，由于定义了ALIGN_HEADER，
	编译的是#else之前的部分；被FaceMngr.h包含时，编译的是后一部分。
	如果用了#pragma once，则会出现递归包含。
	其余各模块：Feature、Mngr等的定义结构都类似。
*/

#ifdef ALIGN_HEADER // 当被各个算法分别的头文件包含时

#define COMPILE_ALIGN_ASM // 编译哪个算法

 // 所有Align算法都需要的一些定义

#define INPUT_PATH	""

#else // 当被FaceMngr项目包含时

// 各个算法分别的头文件
#include "FaceAlign_ASM.h"
#include "FaceAlign_coord.h"
#include "FaceAlign_Haar.h"

#endif