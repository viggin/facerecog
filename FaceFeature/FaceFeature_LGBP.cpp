/*
	Gabor + Local Binary Patterns,  see W. Zhang, et. al., ¡°Local Gabor binary pattern histogram sequence (LGBPHS): 
	A novel non-statistical model for face representation and recognition,¡± in Proc. 10th ICCV, 2005, pp. 786¨C791. 
	and S. Shan, W. Zhang, et. al., ¡°Ensemble of piecewise FDA based on spatial histograms of local (Gabor) binary 
	patterns for face recognition,¡± in Proc. of the 18th ICPR, vol. 4, pp. 606¨C609, Hong Kong, August 2006.
*/

#define DLLSRC

#include <opencv2/core/core.hpp>
using namespace cv;

#include "FaceFeature.h"
#include "Gabor.h"
#include "../tools/AlgorithmSelect.h"

#ifdef COMPILE_FT_LGBP

#define PROD(sz) ((sz).height*(sz).width)
#define SQR2_2	0.7071068	// sqrt(2)/2


// Sz: the width and height of a mat;
// Num: the count of sth.
// Cnt: PROD(Num)
static Size	g_faceSz, lgbpFaceSz;
static Size	blockNum, winNum;
//int ex_blockCnt; // for Subspace_BlkFLD.cpp to use, seems unreachable
static int	lgbpBinNum;
static Size	gKerNum;

static Mat	lgbpFace;
static Mat	tmpFt;
static Mat	tLgbpFace1, tLgbpFace2;


void GenLgbp81Face( Mat & gface, Mat & lgbpFace );

void GetLgbpHist( Mat & lgbpFace, Mat & ft1 );


int InitFeature( CvSize imgSize )
{
	gKerNum = InitGabor(imgSize);

	g_faceSz = imgSize;
	lgbpFaceSz = Size(imgSize.width-2, imgSize.height-2); // lgbpFaceSz is 2 pixels smaller than faceSz
	lgbpFace.create(lgbpFaceSz.height, lgbpFaceSz.width, CV_8UC1);
	tLgbpFace1.create(lgbpFaceSz.height, lgbpFaceSz.width, CV_8UC1);
	tLgbpFace2.create(lgbpFaceSz.height, lgbpFaceSz.width, CV_32FC1);

	/* when setting the block & win Num, note the faceSz */
	PARAM blockNum = Size(4,10); // 10row-by-4col blocks each face for blockwise FLD. 
	//ex_blockCnt = blockNum.area(); // Although LGBP may be applied with direct matching(without blockwise FLD), but blockNum can still be added for compatibility
	PARAM winNum = Size(4,1); // 1row-by-4col windows each block for histogramming
	PARAM lgbpBinNum = 8; // quantize 256 LBP values to lgbpBinNum levels

	// each row stores features for a block
	tmpFt.create(PROD(blockNum), PROD(gKerNum) * PROD(winNum) * lgbpBinNum, CV_FT_FC1); // 8UC1

	return tmpFt.size().area();
}

void GetFeature( CvArr *faceImg32, CvMat *ft32 )// ft32 could be 8UC1, but for compatibility, set to 32F
{
	CvMat ***gface = GaborConv(faceImg32);
	int ftNum1 = PROD(winNum)*lgbpBinNum, // feature num per block per Gabor kernel
		blkCnt = PROD(blockNum);
	int idxc = 0;
	tmpFt.setTo(Scalar(0));

	PARAM int quantizeLevels = 8;
	int quantizeIntv = 256/quantizeLevels;
	for (int gr = 0; gr < gKerNum.height; gr++)
	{
		for (int gc = 0; gc < gKerNum.width; gc++)
		{
			GenLgbp81Face(Mat(gface[gr][gc]), lgbpFace);
			//namedWindow("lgbp face");
			//imshow("lgbp face", lgbpFace);
			//DispCvArr(&CvMat(lgbpFace), "before", false, "%1.0f\t");

			lgbpFace = (lgbpFace-quantizeIntv/2) / quantizeIntv; // have to minus quantizeIntv/2 first, because OpenCV rounds the division result for uchar
			//lgbpFace *= (256/quantizeLevels);
			//namedWindow("lgbp face1");
			//imshow("lgbp face1", lgbpFace);
			//waitKey();
			//DispCvArr(&CvMat(lgbpFace), "after", false, "%1.0f\t");

			double val;
			minMaxLoc(lgbpFace, 0, &val);
			GetLgbpHist(lgbpFace, tmpFt.colRange(idxc*ftNum1, (idxc+1)*ftNum1)); // colRange: the last col is excluded
			idxc++;
		}
	}

	Mat ft = tmpFt.reshape(0, PROD(cvGetSize(ft32)));
	ft.copyTo(Mat(ft32));

}

void GenLgbp81Face( Mat & gface, Mat & lgbpFace ) 
{
	int H = g_faceSz.height, W = g_faceSz.width;
	Mat C = gface(Range(1,H-1), Range(1,W-1));
	Mat R = gface(Range(1,H-1), Range(2,W)),
		U = gface(Range(0,H-2), Range(1,W-1)),
		L = gface(Range(1,H-1), Range(0,W-2)),
		D = gface(Range(2,H), Range(1,W-1)),
		UR = gface(Range(0,H-2), Range(2,W)),
		UL = gface(Range(0,H-2), Range(0,W-2)),
		DL = gface(Range(2,H), Range(0,W-2)),
		DR = gface(Range(2,H), Range(2,W));

	bitwise_and(R - C>0, Scalar(0x01), lgbpFace);

	tLgbpFace2 = SQR2_2*SQR2_2*UR + SQR2_2*(1-SQR2_2)*(U+R) + (1-SQR2_2)*(1-SQR2_2)*C;
	bitwise_and(tLgbpFace2 - C>0, Scalar(0x02), tLgbpFace1);
	lgbpFace += tLgbpFace1;

	bitwise_and(U - C>0, Scalar(0x04), tLgbpFace1);
	lgbpFace += tLgbpFace1;

	tLgbpFace2 = SQR2_2*SQR2_2*UL + SQR2_2*(1-SQR2_2)*(U+L) + (1-SQR2_2)*(1-SQR2_2)*C;
	bitwise_and(tLgbpFace2 - C>0, Scalar(0x08), tLgbpFace1);
	lgbpFace += tLgbpFace1;

	bitwise_and(L - C>0, Scalar(0x10), tLgbpFace1);
	lgbpFace += tLgbpFace1;

	tLgbpFace2 = SQR2_2*SQR2_2*DL + SQR2_2*(1-SQR2_2)*(D+L) + (1-SQR2_2)*(1-SQR2_2)*C;
	bitwise_and(tLgbpFace2 - C>0, Scalar(0x20), tLgbpFace1);
	lgbpFace += tLgbpFace1;

	bitwise_and(D - C>0, Scalar(0x40), tLgbpFace1);
	lgbpFace += tLgbpFace1;
	
	
	tLgbpFace2 = SQR2_2*SQR2_2*DR + SQR2_2*(1-SQR2_2)*(D+R) + (1-SQR2_2)*(1-SQR2_2)*C;
	bitwise_and(tLgbpFace2 - C>0, Scalar(0x80), tLgbpFace1);
	lgbpFace += tLgbpFace1;
}

void GetLgbpHist( Mat & lgbpFace, Mat & ft1 ) 
{
	Size blkSz(lgbpFaceSz.width / blockNum.width,
		lgbpFaceSz.height / blockNum.height);
	Size winSz(blkSz.width / winNum.width,
		blkSz.height / winNum.height);

	UINT8 val;
	UINT8 *pSrc = lgbpFace.data;
	ft_type *pDst = (ft_type*)(ft1.data);
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
							int a = lgbpFace.step[0];
							val = *(pSrc + 
								(br*blkSz.height + wr*winSz.height + r) * lgbpFace.step[0] +
								(bc*blkSz.width + wc*winSz.width + c));
							(*(pDst + bidx * ft1.step[0] / sizeof(ft_type) + lgbpBinNum*widx + val))++;
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
}

#endif