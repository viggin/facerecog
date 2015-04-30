/*
	Extract features from the face.
	Now we support Gabor magnitude(evenly sampled within the mask, GaborMag), Gabor Surface Feature(GSF),
	and Gabor+LBP feature.
	Example:
		int ftSz = InitFeature(faceSz);
		CvMat *ft32 = cvCreateMat(ftSz,1,CV_FT_FC1);
		GetFeature(faceImg32,ft32);
		ReleaseFeature();
*/

#pragma once

#include "../tools/tools.h"

#pragma comment(lib, "../bin/tools.lib")


//extern int ex_blockCnt; // for Subspace_BlkFLD.cpp to use, seems unreachable
/*
	faceSz is the same with the faceSz returned by InitAlign
	Feature length is returned for initializing feature vectors.
	If error occurs, 0 is returned.
*/
DLLEXP int InitFeature(CvSize faceSz);


/*
	ft32 is a column vector with type CV_FT_FC1
*/
DLLEXP void GetFeature(CvArr *faceImg32, CvMat *ft);


/*
	should be called before unloading the dll
*/
DLLEXP void ReleaseFeature();
