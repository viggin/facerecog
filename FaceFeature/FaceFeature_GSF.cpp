/*
	Gabor Surface Feature, proposed by the author, see Ke Yan, Youbin Chen and David Zhang, 
	"Gabor Surface Feature for Face Recognition," to be published in First Asian Conference on 
	Pattern Recognition (ACPR'11), Beijing, Nov. 2011
*/

#define DLLSRC

#include "FaceFeature.h"
#include "Gabor.h"
#include "../tools/AlgorithmSelect.h"

#ifdef COMPILE_FT_GSF

#define PROD(sz) ((sz).height*(sz).width)


// Sz: the width and height of a mat;
// Num: the count of sth.
// Cnt: PROD(Num)
static CvSize	g_faceSz, gsfFaceSz;
static CvSize	blockNum, winNum;
//int ex_blockCnt; // for Subspace_BlkFLD.cpp to use, seems unreachable
static int		gsfBinNum;
static CvSize	gKerNum;

static CvMat	*gsfFace;
static CvMat	*tmpFt;
static CvMat	*tdx1, *tdy1, *tdx2, *tdy2, *tGsfFace;


// quantize a Gabor face gface(generated from 1 Gabor kernel) into a gsfFace
void GenGsfFace( CvMat *gface, CvMat * gsfFace );

// compute local histograms of gafFace to form a feature vector ft1
void GetGsfHist( CvMat * gsfFace, CvMat * ft1 );


int InitFeature( CvSize faceSz )
{
	gKerNum = InitGabor(faceSz);

	g_faceSz = faceSz;
	gsfFaceSz = cvSize(faceSz.width-4, faceSz.height-4);
	gsfFace = cvCreateMat(gsfFaceSz.height, gsfFaceSz.width, CV_8UC1);
	tGsfFace= cvCreateMat(gsfFaceSz.height, gsfFaceSz.width, CV_8UC1);

	PARAM blockNum = cvSize(4,10); // 10row-by-4col blocks each face for blockwise FLD
	//PARAM blockNum = cvSize(4,5); // 5row-by-4col blocks each face for blockwise FLD
	//ex_blockCnt = PROD(blockNum);
	PARAM winNum = cvSize(2,1); // 1row-by-2col windows each block for histogramming
	//PARAM winNum = cvSize(1,1); // 1row-by-1col windows each block for histogramming
	PARAM gsfBinNum = 16; // 4 measures: gabor magnitude(I), gradiant(dI/dx and dI/dy), 2nd grad(d2I/dx2+d2I/dy2), binarilize, so 2^4=16 bins each window

	// each row stores features for a block
	tmpFt = cvCreateMat(PROD(blockNum), PROD(gKerNum) * PROD(winNum) * gsfBinNum, CV_FT_FC1);

	tdx1 = cvCreateMat(gsfFaceSz.height, gsfFaceSz.width+2, CV_32FC1);
	tdy1 = cvCreateMat(gsfFaceSz.height+2, gsfFaceSz.width, CV_32FC1);
	tdx2 = cvCreateMat(gsfFaceSz.height, gsfFaceSz.width, CV_32FC1);
	tdy2 = cvCreateMat(gsfFaceSz.height, gsfFaceSz.width, CV_32FC1);

	return PROD(cvGetSize(tmpFt));
}

void GetFeature( CvArr *faceImg32, CvMat *ft )
{
	CvMat ***gface = GaborConv(faceImg32);
	CvMat sub;
	int ftNum1 = PROD(winNum)*gsfBinNum, // feature num per block per Gabor kernel
		blkCnt = PROD(blockNum);
	int idxc = 0;
	cvZero(tmpFt);

	for (int gr = 0; gr < gKerNum.height; gr++)
	{
		for (int gc = 0; gc < gKerNum.width; gc++)
		{
			GenGsfFace(gface[gr][gc], gsfFace);
			//cvScale(gsfFace, tGsfFace, 256/gsfBinNum);
			//cvNamedWindow("gsf face");
			//cvShowImage("gsf face", tGsfFace);
			//cvWaitKey();

			cvGetSubRect(tmpFt, &sub, cvRect(idxc*ftNum1, 0, ftNum1, blkCnt));
			GetGsfHist(gsfFace, &sub);
			idxc++;
		}
	}

	cvReshape(tmpFt, &sub, 1, PROD(cvGetSize(ft)));
	cvCopy(&sub, ft);

}

void GenGsfFace( CvMat *gface, CvMat * gsfFace ) 
{
	CvMat sub1, sub2;
	cvGetSubRect(gface, &sub1, cvRect(2,2, gsfFaceSz.width+2, gsfFaceSz.height));
	cvGetSubRect(gface, &sub2, cvRect(0,2, gsfFaceSz.width+2, gsfFaceSz.height));
	cvSub(&sub1,&sub2, tdx1);

	cvGetSubRect(gface, &sub1, cvRect(2,2, gsfFaceSz.width, gsfFaceSz.height+2));
	cvGetSubRect(gface, &sub2, cvRect(2,0, gsfFaceSz.width, gsfFaceSz.height+2));
	cvSub(&sub1,&sub2, tdy1);

	cvGetSubRect(tdx1, &sub1, cvRect(2,0, gsfFaceSz.width, gsfFaceSz.height));
	cvGetSubRect(tdx1, &sub2, cvRect(0,0, gsfFaceSz.width, gsfFaceSz.height));
	cvSub(&sub1,&sub2, tdx2);

	cvGetSubRect(tdy1, &sub1, cvRect(0,2, gsfFaceSz.width, gsfFaceSz.height));
	cvGetSubRect(tdy1, &sub2, cvRect(0,0, gsfFaceSz.width, gsfFaceSz.height));
	cvSub(&sub1,&sub2, tdy2);

	cvGetSubRect(gface, &sub1, cvRect(2,2, gsfFaceSz.width, gsfFaceSz.height));
	double m = cvAvg(gface).val[0];
	cvCmpS(&sub1, m, gsfFace, CV_CMP_GT);
	cvAndS(gsfFace, cvScalar(0x08), gsfFace);

	cvGetSubRect(tdx1, &sub1, cvRect(1,0, gsfFaceSz.width, gsfFaceSz.height));
	cvCmpS(&sub1, 0, tGsfFace, CV_CMP_GT);
	cvAndS(tGsfFace, cvScalar(0x04), tGsfFace);
	cvAdd(gsfFace, tGsfFace, gsfFace);

	cvGetSubRect(tdy1, &sub1, cvRect(0,1, gsfFaceSz.width, gsfFaceSz.height));
	cvCmpS(&sub1, 0, tGsfFace, CV_CMP_GT);
	cvAndS(tGsfFace, cvScalar(0x02), tGsfFace);
	cvAdd(gsfFace, tGsfFace, gsfFace);

	cvAdd(tdx2, tdy2, tdx2);
	cvCmpS(tdx2, 0, tGsfFace, CV_CMP_GT);
	cvAndS(tGsfFace, cvScalar(0x01), tGsfFace);
	cvAdd(gsfFace, tGsfFace, gsfFace);
}

void GetGsfHist( CvMat * gsfFace, CvMat * ft1 ) 
{
	CvSize blkSz = cvSize(gsfFaceSz.width / blockNum.width,
		gsfFaceSz.height / blockNum.height);
	CvSize winSz = cvSize(blkSz.width / winNum.width,
		blkSz.height / winNum.height);

	UINT8 val;
	UINT8 *pSrc = (UINT8*)(gsfFace->data.ptr);
	ft_type *pDst = (ft_type*)(ft1->data.ptr);
	int bidx = 0, widx;

	for (int br = 0; br < blockNum.height; br++) // is this efficient?
	{
		for (int bc = 0; bc < blockNum.width; bc++)
		{
			widx = 0;
			for (int wr = 0; wr < winNum.height; wr++)
			{
				for (int wc = 0; wc < winNum.width; wc++)
				{
					for (int r = 0; r < winSz.height; r++)
					{
						for (int c = 0; c < winSz.width; c++)
						{
							val = *(pSrc + 
								(br*blkSz.height + wr*winSz.height + r) * gsfFace->step +
								(bc*blkSz.width + wc*winSz.width + c));
							(*(pDst + bidx * ft1->step / sizeof(ft_type) + gsfBinNum*widx + val))++;
						}
					}
					widx++;
				}
			}
			bidx++;
		}
	}
}

void ReleaseFeature()
{
	ReleaseGabor();
	cvReleaseMat(&gsfFace);
	cvReleaseMat(&tGsfFace);
	cvReleaseMat(&tmpFt);
	cvReleaseMat(&tdx1);
	cvReleaseMat(&tdx2);
	cvReleaseMat(&tdy1);
	cvReleaseMat(&tdy2);
}

#endif