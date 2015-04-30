#define DLLSRC

#include "Fld.h"
#include "dggev.h"


#pragma comment(lib, "dggev.lib") // solving generalized eigen problem using CLAPACK


CFld::CFld(void)
{
	m_postPcaDim = 0;
	m_postLdaDim = 0;
	PARAM m_pcaRatio = .97;
	m_classNum = m_inputDim = m_trainNum = 0;
	fldEigenVals = mu_total = W_pcaT = W_fldT = W_prjT = NULL;
}

CFld::~CFld(void)
{
	Release();
}

int CFld::TrainFld( CvMat *inputs, int *trainIds )
{
	Release();
	m_inputDim = inputs->rows;
	m_trainNum = inputs->cols;

	Pca(inputs, m_pcaRatio);

	CvMat *pcaFace = cvCreateMat(m_postPcaDim, m_trainNum, CV_COEF_FC1);
	cvProjectPCA(inputs, mu_total, W_pcaT, pcaFace);

	Lda(pcaFace, trainIds);

	W_prjT = cvCreateMat(m_postLdaDim, m_inputDim, CV_COEF_FC1);
	cvmMul(W_fldT, W_pcaT, W_prjT);

	cvReleaseMat(&pcaFace);

	return m_postLdaDim;
}

void CFld::SetFldPrjDim( int dim )
{
	if(m_inputDim == 0) // not trained
	{
		if(dim > 0) m_postLdaDim = dim;
	}
	else
	{
		if (dim >= W_prjT->rows || dim < 0) return;
		else if (dim == 0)
		{
			Release();
			return;
		}

		m_postLdaDim = dim;
		CvMat *tmp = cvCreateMat(dim, m_postPcaDim, CV_COEF_FC1);
		CvMat sub;

		cvGetSubRect(W_fldT, &sub, cvRect(0,0, m_postPcaDim, dim));
		cvCopy(&sub,tmp);
		cvReleaseMat(&W_fldT);
		W_fldT = cvCloneMat(tmp);
		cvReleaseMat(&tmp);

		tmp = cvCreateMat(dim, m_inputDim, CV_COEF_FC1);
		cvGetSubRect(W_prjT, &sub, cvRect(0,0, m_inputDim, dim));
		cvCopy(&sub,tmp);
		cvReleaseMat(&W_prjT);
		W_prjT = cvCloneMat(tmp);
		cvReleaseMat(&tmp);
	}
}

void CFld::ProjectFld( CvMat *inputs, CvMat *results )
{
	if (m_inputDim == 0)
	{
		::MessageBox1("No project matrix!");
		return;
	}
	cvProjectPCA(inputs, mu_total, W_prjT, results);
}

void CFld::Release()
{
	m_classNum = m_inputDim = m_trainNum = 0;
	cvReleaseMat(&mu_total);
	cvReleaseMat(&W_pcaT);
	cvReleaseMat(&W_fldT);
	cvReleaseMat(&W_prjT);
	cvReleaseMat(&fldEigenVals);
	fldEigenVals = mu_total = W_pcaT = W_fldT = W_prjT = NULL;
}

void CFld::Pca( CvMat *inputs, double postPcaDimCoef )
{
	int		oriDim = inputs->rows, sampleNum = inputs->cols;
	CvMat	*eigenVecs = cvCreateMat(sampleNum, oriDim, CV_COEF_FC1),
		*eigenVals = cvCreateMat(sampleNum, 1, CV_COEF_FC1);
	mu_total = cvCreateMat(oriDim, 1, CV_COEF_FC1);

	cvCalcPCA(inputs, mu_total, eigenVals, eigenVecs, CV_PCA_DATA_AS_COL); 
	// the eigenvalue seems to range in descending order
	// eigenvectors are row vectors, and seems to have been normalized

	m_postPcaDim = int(postPcaDimCoef);
	if (m_postPcaDim == 0) // postPcaDimCoef is between 0~1, decide the dim according to ratio
	{
		double sum = cvSum(eigenVals).val[0], sum1 = 0;
		int i = 0;
		do 
		{
			sum1 += cvmGet(eigenVals, i++, 0);
		} while (sum1 < sum*postPcaDimCoef);
		m_postPcaDim = i;
		if (m_postPcaDim < 8) m_postPcaDim = min(sampleNum, 8); // postPcaDim shouldn't be too small
	}

	CvMat sub;
	W_pcaT = cvCreateMat(m_postPcaDim, oriDim, CV_COEF_FC1);
	cvGetSubRect(eigenVecs, &sub, cvRect(0,0, oriDim, m_postPcaDim));
	cvCopy(&sub, W_pcaT);

	cvReleaseMat(&eigenVals);
	cvReleaseMat(&eigenVecs);
}

void CFld::Lda( CvMat *inputs, int *trainIds, int postLdaDimCoef /*= 0*/ )
{
	int	oriDim = inputs->rows;
	CvMat *Sw = cvCreateMat(oriDim, oriDim, CV_COEF_FC1),
		*Sb = cvCreateMat(oriDim, oriDim, CV_COEF_FC1); 

	CalcSwSb(inputs, trainIds, Sw, Sb);

	if (CV_COEF_FC1 == CV_64FC1)
		CalcLdaSpace(Sw, Sb, postLdaDimCoef);
	else
	{
		CvMat	*Sw64 = cvCreateMat(oriDim, oriDim, CV_64FC1),
			*Sb64 = cvCreateMat(oriDim, oriDim, CV_64FC1);
		cvConvert(Sw, Sw64);
		cvConvert(Sb, Sb64);
		CalcLdaSpace(Sw64, Sb64, postLdaDimCoef);

		CvMat *W_fldT64 = cvCloneMat(W_fldT);
		cvReleaseMat(&W_fldT);
		W_fldT = cvCreateMat(m_postLdaDim, oriDim, CV_COEF_FC1);
		cvConvert(W_fldT64, W_fldT);

		cvReleaseMat(&Sw64);
		cvReleaseMat(&Sb64);
		cvReleaseMat(&W_fldT64);
	}

	cvReleaseMat(&Sw);
	cvReleaseMat(&Sb);
}

void CFld::CalcSwSb( CvMat *inputs, int *trainIds, CvMat *Sw, CvMat *Sb )
{
	int		oriDim = inputs->rows, sampleNum = inputs->cols;
	int		*id2idx = new int[sampleNum], *smpNumEachClass = new int[sampleNum];
	CvMat	**muPerClass = new CvMat *[sampleNum], sub;
	m_classNum = 0;

	// within-class average
	for (int i = 0; i < sampleNum; i++)
	{
		cvGetCol(inputs, &sub, i);

		int j;
		for (j = 0; j < m_classNum; j++)
		{
			if (id2idx[j] == trainIds[i]) 
			{
				break;
			}
		}
		if (j == m_classNum)
		{
			muPerClass[j] = cvCloneMat(&sub);
			id2idx[j] = trainIds[i];
			smpNumEachClass[j] = 1;
			m_classNum++;
		}
		else
		{
			cvAdd(muPerClass[j], &sub, muPerClass[j]);
			smpNumEachClass[j] ++;
		}
	}

	for (int i = 0; i < m_classNum; i++)
		cvScale(muPerClass[i], muPerClass[i], 1.0/(double(smpNumEachClass[i])));

	// reduce within-class average
	for (int i = 0; i < sampleNum; i++) 
	{
		cvGetCol(inputs, &sub, i);
		int j;
		for (j = 0; j < m_classNum; j++)
			if (id2idx[j] == trainIds[i]) break;
		cvSub(&sub, muPerClass[j], &sub);
	}

	// within-class scatter matrix
	cvMulTransposed(inputs, Sw, 0);

	// between-class scatter matrix
	cvSetZero(Sb);
	CvMat *Sb1 = cvCreateMat(oriDim, oriDim, CV_COEF_FC1);
	for (int i = 0; i < m_classNum; i++)
	{
		cvMulTransposed(muPerClass[i], Sb1, 0);
		cvAddWeighted(Sb, 1, Sb1, smpNumEachClass[i], 0, Sb);
	}

	cvReleaseMat(&Sb1);
	delete []id2idx;
	delete []smpNumEachClass;
	for (int i = 0; i < m_classNum; i++)
		cvReleaseMat(& muPerClass[i]);
	delete []muPerClass;
}

void CFld::CalcLdaSpace( CvMat *Sw64, CvMat *Sb64, int postLdaDimCoef )
{
	int oriDim = Sw64->rows;
	CvMat	*eigenVecs = cvCreateMat(oriDim, oriDim, CV_64FC1);
	double	*ar = new double[oriDim], *ai = new double[oriDim], 
		*be = new double[oriDim];
	double	*A, *B, *E;

	// GeneralEig
	A = Sb64->data.db;
	B = Sw64->data.db;
	E = eigenVecs->data.db;
	GeneralEig(A, B, oriDim, E, ar, ai, be); 


	// choose subspace
	CvMat arm = cvMat(1, oriDim, CV_64FC1, ar);
	CvMat bem = cvMat(1, oriDim, CV_64FC1, be);
	CvMat *idxm = cvCreateMat(1, oriDim, CV_32SC1);
	cvAbs(&arm, &arm); // are all the value positive? is abs needed?(positive definite?)
	cvAbs(&bem, &bem);
	//DispCvArr(&arm,"arm");
	//DispCvArr(&bem,"bem");
	cvAddS(&bem, cvScalar(1), &bem); // be is always near zero
	cvDiv(&arm, &bem, &arm);
	cvSort(&arm, &arm, idxm, CV_SORT_DESCENDING | CV_SORT_EVERY_ROW);

	if (postLdaDimCoef == 0)
	{
		if (oriDim < m_classNum) m_postLdaDim = oriDim - 1; // is this good ?
		else m_postLdaDim = m_classNum-1; // ar[m_classNum-1] will be very small.
		if (m_postLdaDim < 1) m_postLdaDim = 1;
		//if (m_postLdaDim < 3) m_postLdaDim = min(sampleNum, 3);
	}

	W_fldT = cvCreateMat(m_postLdaDim, oriDim, CV_64FC1);
	CvMat subSrc, subDst;
	for (int i = 0; i < m_postLdaDim; i++)
	{
		cvGetRow(eigenVecs, &subSrc, int(cvGetReal1D(idxm, i)));
		cvGetRow(W_fldT, &subDst, i);
		cvCopy(&subSrc, &subDst); // normalize?
	}

	CvMat sub;
	cvGetSubRect(&arm, &sub, cvRect(0,0, m_postLdaDim, 1));
	fldEigenVals = cvCloneMat(&sub);
	//DispCvArr(fldEigenVals,"fldev");
	
	cvReleaseMat(&eigenVecs);
	delete []ar;
	delete []ai;
	delete []be;
	cvReleaseMat(&idxm);
}
