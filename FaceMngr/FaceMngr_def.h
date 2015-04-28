/*
	related inclusions and definitions for CFaceMngr
*/

#pragma once

#include "../tools/tools.h"


// flags used in Pic2NormFace, Enroll, Train, PicRecognize...
#define FM_DO_NOT_NORM		0	// do no face alignment nor light preprocessing
#define FM_DO_FACE_ALIGN	1
#define FM_DO_LIGHT_PREP	2
#define FM_ALIGN_USE_BUF	4	// smooth the alignment position among frames
#define FM_DO_NORM		(FM_DO_FACE_ALIGN | FM_DO_LIGHT_PREP)

#define FM_SAVE_NORM_FACE	8	// save normalized faces to files when enrolling or training
#define FM_TRAIN_SAVE2MODEL	16	// save training samples to face database
#define FM_SAVE_REL_PATH	32	// save relative path when training
#define FM_SHOW_DETAIL		64	// print some info to command line when training

// unexpected class ID
#define FM_UNKNOWN_CLASS_ID	-1 
#define FM_RECOG_NOT_DONE	-3

#define DEF_PIC_PATH	"facePics\\" // default picture path. In the program, a path should have a slash in the right but not the left
#define FACE_REL_PATH	"faces\\" // default norm face path

#ifndef SFACEPOSE
struct SFacePose
{
	CvRect	rc; // Rect of the face in the picture
	float	eyeDis; // distance between eyes
	float	eyeAngle; // rotation of the face
};
#endif


// function pointers, in order to dynamically load dlls
typedef bool		(*tPfnInitAlign)(CvSize*);
typedef bool		(*tPfnGetFace)(IplImage*,CvMat*,bool, CvPoint2D32f&,CvPoint2D32f&);
typedef SFacePose	(*tPfnGetFacePose)(void);
typedef void		(*tPfnRelease)();

typedef bool		(*tPfnInit)(CvSize);
typedef void		(*tPfnRunLightPrep)(CvMat*);

typedef int			(*tPfnInitFeature)(CvSize);
typedef void		(*tPfnGetFeature)(CvArr*,CvMat*);

typedef int			(*tPfnCalcSubspace)(CvMat*,int*);
typedef void		(*tPfnProject)(CvMat*,CvMat*);
typedef int			(*tPfnGetInt)();
typedef double		(*tPfnCalcVectorDist)(CvMat*,CvMat*);
typedef bool		(*tPfnWrite)(ofstream&);
typedef bool		(*tPfnRead)(ifstream&);

struct SModel // used in the face database
{
	int		classId;
	CString	picPath; // path of the origin picture
	CvMat	*model;
};

struct SMatch // used when matching a face
{
	int		classId;
	CString	picPath;
	double	dist;    // matching distance
};

struct SFInfo // Struct of File Info£¬used to store file info when training
{
	int		classId;
	CString picPath;
	SFInfo(int id,CString path):classId(id),picPath(path){}
};

// a struct to store functions exported from the algorithm dlls
struct SFuncBundle
{
	tPfnInitAlign	InitAlign;
	tPfnGetFace		GetFace;
	tPfnGetFacePose	GetFacePose;
	tPfnRelease		ReleaseAlign;

	tPfnInit		InitLight;
	tPfnRunLightPrep RunLightPrep;
	tPfnRelease		ReleaseLight;

	tPfnInitFeature	InitFeature;
	tPfnGetFeature	GetFeature;
	tPfnRelease		ReleaseFeature;

	tPfnCalcSubspace CalcSubspace;
	tPfnProject		Project;
	tPfnGetInt		GetModelSize;
	tPfnGetInt		GetFtDim;
	tPfnCalcVectorDist CalcVectorDist;
	tPfnWrite		WriteDataToFile;
	tPfnRead		ReadDataFromFile;
	tPfnRelease		ReleaseSubspace;
};

typedef vector<SModel>::iterator	smd_iter;
typedef vector<SFInfo>::iterator	sfi_iter;
typedef vector<SMatch>::iterator	smt_iter;