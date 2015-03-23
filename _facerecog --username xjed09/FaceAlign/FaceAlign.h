/*
	This project will generate a dll that does face alignment.

	Example:
		IplImage *img = cvLoadImage("test.tif",CV_LOAD_IMAGE_GRAYSCALE);
		CvSize faceSz;
		InitAlign(&faceSz);
		CvMat *faceImg = cvCreateMat(faceSz.height,faceSz.width,CV_8UC1);
		GetFace(img,faceImg,false);
		cvShowImage("face",faceImg);
		cvWaitKey();
		ReleaseAlign();

	You can also call GetFacePose to get some coefs of the face.
	If you already have the coordinate of the eyes, call EyeCoord2FaceCrop directly.
	See the cpp files to find the input files required by each algorithm.

*/

#pragma once

#include "../tools/tools.h"

#pragma comment(lib, "../bin/tools.lib")


#ifndef SFACEPOSE
struct SFacePose
{
	CvRect	rc; // Rect of the face in the picture
	float	eyeDis; // distance between eyes
	float	eyeAngle; // rotation of the face
};
#define SFACEPOSE
#endif


/*
	face size is an output. Return true if succeed.
	Change the code of this funtion to change face size or other alignment coefs
*/
DLLEXP bool InitAlign(CvSize *faceSz);


/*
	use the provided eye coordinates leftEye and rightEye to crop a face out of pic8, and save it into faceImg.
	if the coordinates are out of, or too near to the image border, function will return false.
	pic8:		CV_8UC1, used to do alignment
	faceImg:	CV_8UC1, the size is the same with faceSz returned by InitAlign.
	useBuf:		if true, eye position will be smoothed among frames
*/
bool EyeCoord2FaceCrop( IplImage * pic8, CvMat * faceImg, 
							  CvPoint2D32f leftEye, CvPoint2D32f rightEye, bool useBuf );


/*
	for ASM method, if leftEye and rightEye is provided, this function performs like EyeCoord2FaceCrop
		Otherwise, it will use VJ algorithm and ASM to crop a face out of pic8, and save it into faceImg.
	for VJ method, it will use VJ algorithm to crop a face out of pic8, and save it into faceImg. Eye coordinates will be ignored
	if the coordinates are out of, or too near to the image border, function will return false.
	pic8:		CV_8UC1, used to do alignment
	faceImg:	CV_8UC1, the size is the same with faceSz returned by InitAlign.
	useBuf:		if true, eye position will be smoothed among frames
*/
DLLEXP bool GetFace(IplImage *pic8, CvMat *faceImg8, bool useBuf, 
					CvPoint2D32f &leftEye = cvPoint2D32f(0,0), CvPoint2D32f &rightEye = cvPoint2D32f(0,0));


DLLEXP SFacePose GetFacePose();


/*
	should be called before unloading the dll
*/
DLLEXP void ReleaseAlign();
