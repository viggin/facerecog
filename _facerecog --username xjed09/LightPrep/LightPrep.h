/*
	Do the light preprocessing for face image.
	Usage: InitLight->RunLightPrep->RunLightPrep->...->ReleaseLight
	For Yan's method, 2 image files are needed. See the cpp file.
*/

#pragma once

#include "../tools/tools.h"

#pragma comment(lib, "../bin/tools.lib")

/*
	faceSz is the same with the faceSz returned by InitAlign
*/
DLLEXP bool InitLight(CvSize faceSz);


/*
	input/output are both faceImg8, which is of type CV_8UC1
*/
DLLEXP void RunLightPrep(CvMat *faceImg8);


/*
	should be called before unloading the dll
*/
DLLEXP void ReleaseLight();

