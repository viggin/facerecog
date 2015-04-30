/*
	Fisher LDA subspace with normalized cosine metric nearest neighbor classifier(the NN classifier should be implemented seperately)
	use this method together with FaceFeature_GaborMag
*/

#define DLLSRC

#include "Subspace.h"
#include "../tools/AlgorithmSelect.h"

#ifdef COMPILE_SS_FLD


static CFld g_fld;

int CalcSubspace( CvMat *inputs, int *trainIds )
{
	return g_fld.TrainFld(inputs,trainIds);;
}

void Project( CvMat *inputs, CvMat *results )
{
	g_fld.ProjectFld(inputs, results);
}

int GetModelSize(){return g_fld.m_postLdaDim;}

int GetFtDim(){return g_fld.m_inputDim;}

// If the dim is larger than 1, use cosine metric;
// Else, use L2 metric.
double CalcVectorDist( CvMat *target, CvMat *query )
{
	// use normalized cosine metric
	// other alternative metrics: L1, L2, Mahalanobis ...
	if (target->rows > 1)
		return (1-(cvDotProduct(target, query) / cvNorm(target) / cvNorm(query)))/2;
	else
		return cvNorm(target, query, CV_L2);
}

// export/import to a binary-and-ascii file: precision, feature dim, subspace dim, mu_total ,W_prjT
bool WriteDataToFile( ofstream &os )
{
	WriteIntText(os, g_fld.W_prjT->step / g_fld.W_prjT->cols, "dataBytes:");
	WriteIntText(os, g_fld.m_inputDim, "inputDim:");
	WriteIntText(os, g_fld.m_postLdaDim, "postLdaDim:");
	WriteCvMatBin(os, g_fld.mu_total, "mu_total:");
	WriteCvMatBin(os, g_fld.W_prjT, "W_prjT:");
	return true;
}

bool ReadDataFromFile( ifstream &is )
{
	g_fld.Release();

	int		dataBytes;
	ReadIntText(is, dataBytes);
	if ((dataBytes == 4 && CV_COEF_FC1 == CV_64FC1) ||
		(dataBytes == 8 && CV_COEF_FC1 == CV_32FC1))
	{
		::MessageBox1("float/double not match!");
		return false;
	}

	ReadIntText(is, g_fld.m_inputDim);
	ReadIntText(is, g_fld.m_postLdaDim);
	g_fld.mu_total = cvCreateMat(g_fld.m_inputDim, 1, CV_COEF_FC1);
	ReadCvMatBin(is, g_fld.mu_total);
	g_fld.W_prjT = cvCreateMat(g_fld.m_postLdaDim, g_fld.m_inputDim, CV_COEF_FC1);
	ReadCvMatBin(is, g_fld.W_prjT);

	return true;
}

void ReleaseSubspace()
{
	//g_fld.Release();
}

#endif
