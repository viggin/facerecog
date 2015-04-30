/*
	Provides a series of methods useful for face recognition. They are intergrated to a class.
*/

#pragma once

#include "FaceMngr_def.h"


/* "CFaceMngr::m_lstModel": class "std::vector<_Ty>" needs dll interface for client of class "CFaceMngr" to use */
#pragma warning(disable: 4251)

class __declspec(dllexport) CFaceMngr // why can't I use DLLEXP here? because of the extern "C"?
{
public:
	CFaceMngr(void);
	~CFaceMngr(void);

	/* 
		loads and calls init func for FaceAlign,LightPrep,FaceFeature
		dllList:	the names of the 4 algorithm dlls, like 
			FaceAlign_ASM.dll, LightPrep_Tan.dll, FaceFeature_GSF.dll, Subspace_BlkFLD.dll
	*/
	bool	InitMngr(vector<LPTSTR> dllList);


/// ENROLL

	/*
		do alignment and light preprocessing, return true if succeeded to find a face.
		related flags are FM_DO_NOT_NORM, FM_DO_FACE_ALIGN, FM_DO_LIGHT_PREP, FM_ALIGN_USE_BUF, FM_DO_NORM
		see the definition of these flags for details
		pic:		input 24 bit RGB or 8 bit gray picture
		faceImg8:	output 8 bit face image, the size could be obtained by CFaceMngr::m_faceSz after calling CFaceMngr::InitMngr
		eye coordinates:	could be zero, see function GetFace in FaceAlign.h
	*/
	bool Pic2NormFace(CvArr *pic, CvMat *faceImg8, DWORD flag = FM_DO_NORM, 
		CvPoint2D32f &leftEye = cvPoint2D32f(0,0), CvPoint2D32f &rightEye = cvPoint2D32f(0,0));


	/*
		calculate the model vector for normalized face faceImg8, return true if succeeded. Need to train first.
	*/
	bool	NormFace2Model(CvArr *faceImg8, CvMat *model);


	/*
		Save model vector and info to the gallery.
		path:	the path of the picture from which model vector was extracted.
		classId:the identity number of the person. Could be FM_UNKNOWN_CLASS_ID. Useful when training.
		model:	the model vector (calculated by NormFace2Model)
	*/
	void	SaveToModel(LPCTSTR path, int classId, CvMat *model);


	/*
		A combination of methods: Pic2NormFace, NormFace2Model and SaveToModel.
		Save the face in the picture (if any) to the gallery CFaceMngr::m_lstModel
		strPath:	the path of the picture
		classId:	see SaveToModel
		flag:		see the flags in Pic2NormFace. You can also set FM_SAVE_NORM_FACE, if so, the normalized face will be saved
			in strPath\FACE_REL_PATH

		if the function failed in automatically locate the face (Pic2NormFace returns false), it will let the user select the 
			eye position with mouse. A instance of CPickEyeDlg will be created. See PickEyeDlg.h for more info.
	*/
	bool	SavePicToModel(LPCTSTR strPath, int classId, DWORD flag);
	

	/* clear the gallery */
	void	ClearList();


	/* return the size of the gallery */
	int		GetModelCount();


/// TRAINING

	/*
		Train a classifier (like a subspace project matrix).
		rootPath:	the root path of all the pictures used for training. could be empty
		paths:		a vector containing the information of each picture used for training
		flag:		could be a combination of FM_SAVE_NORM_FACE, FM_SHOW_DETAIL, FM_TRAIN_SAVE2MODEL and the flags used by Pic2NormFace
			see the definitions of these flags for more info.

		e.g.:	rootPath = D:\\face\\£¬paths = {{"John_1.bmp",1}, {"John_2.bmp",1}, {"Mike_1.bmp",5},...}
		Now, only the biggest face in the picture will be used for training. If no face is automatically detected, the picture will be discarded.

	*/
	bool	Train(LPCTSTR rootPath, vector<SFInfo> &paths, DWORD flag);


	/* return true if training has been done */
	bool	HasTrained();
private:

	/* generate the matrix for training "inputs", and trainIds, according to m_lstModel */
	void	FormTrainMat(CvMat *inputs, int *trainIds);


	/* save the training samples to the gallery after training */
	void	TrainResSave2Model();


/// RECOGNITION

public:

	/*
		Recognize the input "model" after enrolling and training, find the most similar sample from the gallery, and
			save the information to "info". Nearest Neighbor method is used.
	*/
	bool	ModelRecognize(CvMat *model, SMatch *info);


	/*
		A combination of Pic2NormFace, NormFace2Model and ModelRecognize.
		flag:	see the flags in Pic2NormFace
	*/
	bool	PicRecognize(CvArr *pic, DWORD flag, SMatch *info);


	/*
		Recognize the input "model" after enrolling and training, rank the samples from the gallery according to their similarity
			with "model", the order is saved in matchOrder(row vector).
		That is, m_lstModel[matchOrder[i]] is the i'th most similar sample
	*/
	bool	ModelRecognizeMulti(CvMat *model, CvMat *matchOrder);


	/*
		A combination of Pic2NormFace, NormFace2Model and ModelRecognizeMulti.
		flag:	see the flags in Pic2NormFace
	*/
	bool PicRecognizeMulti(CvArr *pic, DWORD flag, CvMat *matchOrder, 
		CvPoint2D32f &leftEye = cvPoint2D32f(0,0), CvPoint2D32f &rightEye = cvPoint2D32f(0,0));


/// EXPORT/IMPORT

	bool	WriteMatToFile( ofstream &os ); // save the coefficients of the classifier to file, return true if succeeded
	bool	WriteModelToFile( ofstream &os ); // write the current gallery (including the info and the model vector) to file
	bool	ReadMatFromFile( ifstream &is );
	int		ReadModelFromFile( ifstream &is ); // return the model imported

/// MEMBER VARIABLES

	vector<SModel>	m_lstModel; // the gallery

	CvMat		*tfaceImg8, *tfaceImg32, *tfeature, *tmodel; // temp variables

	SFuncBundle	f;	// functions in the algorithm dlls
	HINSTANCE	ha,hl,hf,hs;	// dll modules: FaceAlign,LightPrep,FaceFeature,Subspace
	vector<LPTSTR> m_dllList;	// the name of dlls

	//CString m_rtPath;
	CvSize	m_faceSz;
	int		m_featureSz, m_modelSz; // the dimention(length) of the feature vector and model vector
	int		m_trainNum, m_trainclsNum; // the number and class number of training samples

	bool	m_bTrained;	// if training has been done
	int		m_stamp; // after each training, a stamp will be generated according to the time.
		// it will be saved to the classifier and the gallery file, in order to avoid mis-using of mismatch classifier and gallery
	bool	m_bAllLoaded;
};
