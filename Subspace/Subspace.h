/*
	Train a classifier (like a subsapce), then classify the feature vectors.
	Now we are have Fisherface and blockwise-Fisherface method,	and no subspace method
	For these methods, we should first project a feature vector into a "model" vector on the subspace,
		then use nearest neighbor classifier to classfy it.
	Usage: CalcSubspace / LoadDataFromFile -> Project -> CalcVectorDist
*/

#pragma once

#include "../tools/tools.h"
#include "Fld.h"

#pragma comment(lib, "../bin/tools.lib")


/*
	Use the inputs and trainIds to train a subspace classifier.
	inputs:		a matrix of column feature vectors
	trainIds:	an integer array with inputs->cols elements, if trainId[i]==trainId[j], then the i'th and j'th column of 
		inputs should be feature vectors from the same persons face
	Return the size of the model vector 
	For FLD method, this function calculates a FLD subspace and returns the dimension of the subspace;
	For BlkFLD, it calculates several FLD subspaces and returns the sum of the dimensions of the subspaces;
	For DirectMatch, it does nothing and returns inputs->rows. trainIds could be NULL
*/
DLLEXP int CalcSubspace(CvMat *inputs, int *trainIds);


/*
	project the feature vectors into model vectors. For DirectMatch method, inputs and results are the same.
	inputs:		can be either a vector or a matrix of column vectors
	results:	has GetModelSize() rows and inputs->cols columns
*/
DLLEXP void Project(CvMat *inputs, CvMat *results);


DLLEXP int GetModelSize();


DLLEXP int GetFtDim(); // dimension of the origin feature space


/*
	target and query should both be vector with GetSubspaceDim() rows and 1 column.
	For FLD method, this function uses cosine distance metric
	For BlkFLD, it uses cosine distance metric on each block and then compute the weighted sum of the distances.
	For DirectMatch, it uses histogram intersection distance metric on each block and then compute the 
	weighted sum of the distances.
	No matter which method is used, the return is normalized to 0~1; 
		the SMALLER the distance is, the more similar the 2 vectors are.
*/
DLLEXP double CalcVectorDist(CvMat *target, CvMat *query);


/*
	export to a binary-and-ascii file, containing
		precision, feature dimension, model size, project matrix and average vector, etc.
*/
DLLEXP bool WriteDataToFile(ofstream &os);


/*
	read subspace information from "is" to the static variables of the dll
*/
DLLEXP bool ReadDataFromFile(ifstream &is);


/*
	should be called before unloading the dll
*/
DLLEXP void ReleaseSubspace();
