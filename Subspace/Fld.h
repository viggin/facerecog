#pragma once

#include "../tools/tools.h"

// define the precision of subspace base vectors, depth = 32 or 64
#define CV_COEF_FC1 CV_32FC1 // 64 seems to be better


class CFld
{
public:
	CFld(void);
	~CFld(void);

public:

	// See the comments of method "Lda"
	int TrainFld(CvMat *inputs, int *trainIds);


	// Called before or after TrainFld.
	// If before, set the m_postLdaDim. If dim>m_classNum-1 when training, m_postLdaDim = m_classNum-1.
	// If after, set the m_postLdaDim and change the W_fldT and W_prjT. dim should be <= W_prjT->rows.
	void SetFldPrjDim(int dim);


	// Each column of inputs and results is a feature vector.
	void ProjectFld(CvMat *inputs, CvMat *results);


	// Will be called before re-training.
	// Automatically called in destructor
	void Release();


	/*
	Do PCA for inputs, the results is saved in mu_total and W_pcaT.
	inputs:			feature matrix, each column is a feature vector.
	postPcaDimCoef:	If it's a integer, it is the dimension of the PCA subspace;
		If it's a decimal between 0~1, it is the ratio of the reserved eigenvalues
	*/
	void Pca(CvMat *inputs, double postPcaDimCoef);


	/*
	inputs:		M-by-N, similar with Pca, but the average of each row should be 0. inputs will be changed.
	trainIds:	ID tags of train samples. should be of the same length with N.
	postLdaDimCoef:	if = 0, it will be automatically chosen as (classnumber - 1)
		( See P. N. Belhumeur£¬Eigenfaces vs. fisherfaces: recognition using 
		class specific linear projection)
		If so, if (classnumber - 1) < 3, in case the dim is too small, it will be min(N, 3)
	return:		the number of classes used for training(How many diff people).

	The eigenvector and eigenvalue calculated by CLAPACK is a little diff from MATLAB.
	Calls CalcSwSb and CalcLdaSpace, save result to W_fldT.
	*/
	void Lda(CvMat *inputs, int *trainIds, int postLdaDimCoef = 0);


private:


	void CalcSwSb(CvMat *inputs, int *trainIds, CvMat *Sw, CvMat *Sb);


	void CalcLdaSpace(CvMat *Sw64, CvMat *Sb64, int postLdaDimCoef);


public:

	CvMat	*mu_total;
	CvMat	*W_pcaT, *W_fldT, *W_prjT;
	CvMat	*fldEigenVals; // CV_64FC1, 1 row m_postLdaDim cols.
	int		m_postPcaDim, m_postLdaDim;
	double	m_pcaRatio;
	int		m_trainNum, m_inputDim;
	int		m_classNum; // class number of training samples

};
