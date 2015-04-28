/*
uses histogram intersection distance metric on each block and then compute the weighted sum of the distances.
compatible with FaceFeature_GSF or FaceFeature_LGBP
*/

#define DLLSRC

#include "Subspace.h"
#include "../FaceFeature/FaceFeature.h" // for the macro CV_FT_FC1
#include "../tools/AlgorithmSelect.h"


#ifdef COMPILE_SS_DirectMatch

static int	g_blkCnt; // number of blocks
static int	g_totalPrjDim;
static int	g_totalInputDim, g_blkInputDim;
static double	*g_blkWeights;
static bool	g_bNoWeights;
static CvMat	*g_tm; // temp matrix

int CalcSubspace( CvMat *inputs, int *trainIds )
{
	ReleaseSubspace();

	PARAM g_blkCnt = 40; // must be identical with the feature extraction module !!!
	g_totalInputDim = inputs->rows;
	g_blkInputDim = g_totalInputDim / g_blkCnt;

	double weight[40]= {    
		0.5552,    0.4512,    0.4844,    0.3919,
		0.5412,    0.6145,    0.6537,    0.5232,
		0.4742,    0.6213,    0.6209,    0.4789,
		0.3825,    0.5416,    0.5881,    0.4213,
		0.2759,    0.4358,    0.4695,    0.3126,
		0.2213,    0.3923,    0.4098,    0.2576,
		0.2299,    0.3484,    0.3710,    0.2384,
		0.2286,    0.3066,    0.3228,    0.2226,
		0.2200,    0.2635,    0.3002,    0.2009,
		0.1753,    0.2648,    0.2704,    0.1633};
		g_blkWeights = new double[g_blkCnt];
		PARAM g_bNoWeights = true;
		if (g_bNoWeights)
		{
			for (int i=0; i<g_blkCnt;i++) 
				g_blkWeights[i] = 1;
			g_tm = cvCreateMat(g_totalInputDim, 1, CV_FT_FC1);
		}
		else
		{
			for (int i=0; i<g_blkCnt;i++)
				g_blkWeights[i] = weight[i];
			g_tm = cvCreateMat(g_blkInputDim, 1, CV_FT_FC1);
		}

		g_totalPrjDim = g_totalInputDim;

		return g_totalPrjDim;
}

void Project( CvMat *inputs, CvMat *results )
{
	cvCopy(inputs, results);
}

int GetModelSize(){return g_totalPrjDim;}

int GetFtDim(){return g_totalInputDim;}

double CalcVectorDist( CvMat *target, CvMat *query )
{
	// use histogram intersection on each block
	// then fuse them in score level, with the weight of subspace dim of each block.
	double score = 0;
	if (! g_bNoWeights)
	{
		CvMat subT, subQ;
		int prjCnt = 0;
		for (int i = 0; i < g_blkCnt; i++)
		{
			cvGetRows(target, &subT, prjCnt, prjCnt+g_blkInputDim);
			cvGetRows(query, &subQ, prjCnt, prjCnt+g_blkInputDim);
			prjCnt += g_blkInputDim;
			cvMin(&subT,&subQ, g_tm);
			score += (cvSum(g_tm).val[0] * g_blkWeights[i]);
		}
	}
	else
	{
		cvMin(target,query, g_tm);
		score = +cvSum(g_tm).val[0];
	}

	double normSum = cvSum(target).val[0];
	//double norm1Sum = cvSum(query).val[0];
	score /= normSum;
	return 1-score;
}

bool WriteDataToFile( ofstream &os )
{
	WriteIntText(os, g_blkCnt, "blockCount:");
	WriteIntText(os, g_totalInputDim, "totalInputFeatureDim:");
	return true;
}

bool ReadDataFromFile( ifstream &is )
{
	ReleaseSubspace();

	ReadIntText(is, g_blkCnt);
	CString tmp;
	ReadStringLine(is,tmp);
	ReadIntText(is, g_totalInputDim);
	g_totalPrjDim = g_totalInputDim;
	g_blkInputDim = g_totalInputDim / g_blkCnt;


	double weight[40]= {    
		0.5552,    0.4512,    0.4844,    0.3919,
		0.5412,    0.6145,    0.6537,    0.5232,
		0.4742,    0.6213,    0.6209,    0.4789,
		0.3825,    0.5416,    0.5881,    0.4213,
		0.2759,    0.4358,    0.4695,    0.3126,
		0.2213,    0.3923,    0.4098,    0.2576,
		0.2299,    0.3484,    0.3710,    0.2384,
		0.2286,    0.3066,    0.3228,    0.2226,
		0.2200,    0.2635,    0.3002,    0.2009,
		0.1753,    0.2648,    0.2704,    0.1633};

	PARAM g_bNoWeights = true;
	g_blkWeights = new double[g_blkCnt];
	if (g_bNoWeights)
	{
		for (int i=0; i<g_blkCnt;i++) 
			g_blkWeights[i] = 1;
		g_tm = cvCreateMat(g_totalInputDim, 1, CV_FT_FC1);
	}
	else
	{
		for (int i=0; i<g_blkCnt;i++)
			g_blkWeights[i] = weight[i];
		g_tm = cvCreateMat(g_blkInputDim, 1, CV_FT_FC1);
	}

	return true;
}

void ReleaseSubspace()
{
	if (g_blkWeights) delete []g_blkWeights;
	g_blkCnt = g_totalPrjDim = g_totalInputDim = g_blkInputDim = 0;
	cvReleaseMat(&g_tm);
	g_tm = NULL;
	g_blkWeights = NULL;
}

#endif
