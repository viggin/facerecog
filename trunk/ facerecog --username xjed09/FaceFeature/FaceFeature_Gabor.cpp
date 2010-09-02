#define DLLSRC

#include "FaceFeature_Gabor.h"

#ifdef COMPILE_FT_GABOR


CFaceFeature::CFaceFeature(void)
{
	kmax = PI/2;
	f = sqrt(double(2));
	sigma = 2*PI;
	scaleNum = 5;
	angleNum = 8;
	th = 5e-3;

	// 自动计算核尺寸，以使边界上小波幅度的最大值小于th
	kernelRadius = (int)ceil( sqrt( -log( th*sigma*sigma/kmax/kmax ) *2*sigma*sigma/kmax/kmax ) );
	scales = new int[scaleNum];
	for (int i = 0; i < scaleNum; i++) // 释放小波组
		scales[i] = i;

	sampleIntv = 4;
}

CFaceFeature::~CFaceFeature(void)
{
	if (m_bInited) Release();
}

void CFaceFeature::Release()
{
	delete [scaleNum]scales;
	for (int i = 0; i < scaleNum; i++) // 释放小波组
	{
		for (int j = 0; j < angleNum; j++)
			cvReleaseMat(&G[i][j]);
		delete [angleNum]G[i]; // 这种写法与 delete []G[i]相同
	}
	delete [scaleNum]G;

	cvReleaseMat(&padded);
	cvReleaseMat(&tI);
	cvReleaseMat(&tC);
	cvReleaseMat(&tD);

	delete []startPts;
	delete []endPts;
}

int CFaceFeature::Init( CvSize imgSize, CvMat *mask )
{
	InitGaborKernel();
	tI = cvCreateMat(kernelWidth, kernelWidth, CV_FT_FC1),
	tC = cvCreateMat(kernelWidth, kernelWidth, CV_FT_FC2),
	tD = cvCreateMat(kernelWidth, kernelWidth, CV_FT_FC2);
	return InitSamplePoints(mask, imgSize);
}

void CFaceFeature::InitGaborKernel()
{
	kernelWidth = kernelRadius*2 + 1;

	G = new CvMat **[scaleNum];
	CvMat *R = cvCreateMat(this->kernelWidth, this->kernelWidth, CV_FT_FC1),
		*I = cvCreateMat(this->kernelWidth, this->kernelWidth, CV_FT_FC1);
	double DC = exp(-sigma*sigma/2);

	for (int i = 0; i < scaleNum; i++)
	{
		G[i] = new CvMat *[angleNum];
		double k = kmax/pow(f, this->scales[i]),
			tmpV = k*k /sigma/sigma;

		for (int j = 0; j < angleNum; j++)
		{
			G[i][j] = cvCreateMat(this->kernelWidth, this->kernelWidth, CV_FT_FC2);
			double phi = PI/angleNum*j;
			double tmpV0, tmpV1;

			for (int y = -kernelRadius; y <= kernelRadius; y++)
			{
				for (int x = -kernelRadius; x <= kernelRadius; x++)
				{
					tmpV0 = tmpV * exp(-tmpV * (x*x + y*y) / 2.0);
					tmpV1 = k*cos(phi)*x + k*sin(phi)*y;
					cvmSet( R, y+kernelRadius, x+kernelRadius, tmpV0 * cos(tmpV1 - DC) );
					cvmSet( I, y+kernelRadius, x+kernelRadius, tmpV0 * sin(tmpV1) );

					/* G{scale_idx,angle_idx} = k^2/sigma^2 * exp(-k^2*(X.^2+Y.^2)/2/sigma^2)...
						.*(exp(1i*(k*cos(phi)*X+k*sin(phi)*Y) - DC)); */
				}
			}

			cvMerge(R, I, NULL, NULL, G[i][j]); // 复值小波
		}
	}
	cvReleaseMat(&R);
	cvReleaseMat(&I);
}

int CFaceFeature::InitSamplePoints(CvMat *mask, CvSize imgSize)
{
	ssz = cvSize( (imgSize.width - 1)/sampleIntv + 1, 
		(imgSize.height - 1)/sampleIntv + 1 );
	topleft = cvPoint(((imgSize.width - 1) % sampleIntv)/2,
		((imgSize.height - 1) % sampleIntv)/2);
	padSize = cvSize( __max(0, kernelRadius - topleft.x), // 卷积时需要延拓
		__max(0, kernelRadius - topleft.y) );
	padded = cvCreateMat(imgSize.height + padSize.height * 2, imgSize.width + padSize.width * 2, CV_FT_FC1);

	CvPoint pt;
	startPts = new int[ssz.height]; // 每行的起始采样点的x坐标
	endPts = new int[ssz.height]; // 每行最后一个采样点的x坐标
	int ptNum = 0;
	for (int i = 0; i < ssz.height; i++)
	{
		pt.y = topleft.y + i*sampleIntv;
		startPts[i] = topleft.x;
		endPts[i] = 0;
		for (int j = 0; j < ssz.width; j++)
		{
			pt.x = topleft.x + j*sampleIntv;
			if (mask && cvGetReal2D(mask, pt.y, pt.x) == 0)
			{
				if (endPts[i] == 0) // 相当于一个flag
					startPts[i] = pt.x + sampleIntv;
			}
			else
			{
				endPts[i] = pt.x;
				ptNum++;
			}
		}
	}

	return ptNum * scaleNum * angleNum;
}

void CFaceFeature::GetFeature( CvArr *faceImg32, CvMat *ft32 )
{
	/*
	ssz = floor((sz-1)/sampleRate);
	[idxX idxY] = meshgrid(0:ssz(2),0:ssz(1));
	topleft = floor(rem(sz-1,sampleRate)/2)+1;
	idxX = idxX*sampleRate+topleft(2);
	idxY = idxY*sampleRate+topleft(1);
	samplePos = sub2ind(sz,idxY,idxX);
	*/
	
	cvCopyMakeBorder(faceImg32, padded, cvPoint(padSize.width, padSize.height) , IPL_BORDER_REPLICATE);

	CvRect	rc = cvRect(0,0, kernelWidth,kernelWidth);
	CvMat	*sub, tmpHeader;
	int		resIdx = 0;

	cvSetZero(tI);

	for (int y = 0; y < ssz.height; y++)
	{
		rc.y = topleft.y + padSize.height + y*sampleIntv - kernelRadius;
		for (int x = startPts[y]; x <= endPts[y]; x += sampleIntv)
		{
			rc.x = x + padSize.width - kernelRadius;
			sub = cvGetSubRect(padded, &tmpHeader, rc);
			cvMerge(sub, tI, NULL, NULL, tC);
			for (int i = 0; i < scaleNum; i++) // 每次对一个点做所有卷积，以节省一些操作
			{
				for (int j = 0; j < angleNum; j++)
				{
					cvMulSpectrums(tC, G[i][j], tD, NULL);
					CvScalar sum = cvSum(tD);
					cvmSet(ft32, resIdx++, 0, sqrt(sum.val[0]*sum.val[0] + sum.val[1]*sum.val[1]));
				}
			}
		}
	}
}

void CFaceFeature::ShowGaborKernel()
{
	int *an = new int[scaleNum];
	
	CvSize	sz = CheckConcat((CvArr***)G, scaleNum, angleNum, NULL);
	CvMat	*m = cvCreateMat(sz.height, sz.width, CV_FT_FC2),
		*r = cvCreateMat(sz.height, sz.width, CV_FT_FC1);
	ConcatArrs((CvArr***)G, m, scaleNum, angleNum, NULL);
	cvSplit(m, r, NULL,NULL,NULL);
	cvNormalize(r, r, 1,0, CV_MINMAX);
	cvShowImage("Gabor kernel real part", r);
	cvWaitKey();
	cvReleaseMat(&m);
	cvReleaseMat(&r);
	cvDestroyWindow("Gabor kernel real part");
}

void CFaceFeature::GaborConv( CvArr *src, CvMat *dst, int scale, int angle )
{/*
	padSize.height = 9;
	padSize.width = 9;


	CvRect	rc = cvRect(0,0, kernelWidth,kernelWidth);
	CvMat	*sub, tmpHeader;
	int		resIdx = 0;
	CvSize	sz = cvGetSize(src);
	CvMat	*I = cvCreateMat(kernelWidth, kernelWidth, CV_FC1),
		*C = cvCreateMat(kernelWidth, kernelWidth, CV_FC2),
		*D = cvCreateMat(kernelWidth, kernelWidth, CV_FC2);
	cvSetZero(I);
	padded = cvCreateMat(sz.height + padSize.height * 2, sz.width + padSize.width * 2, CV_FC1);
	cvCopyMakeBorder(src, padded, cvPoint(padSize.width, padSize.height) , IPL_BORDER_REPLICATE);


	for (int y = 0; y < sz.height; y++)
	{
		rc.y = y + padSize.height - kernelRadius;
		for (int x = 0; x < sz.width; x++)
		{
			rc.x = x + padSize.width - kernelRadius;
			sub = cvGetSubRect(padded, &tmpHeader, rc);
			cvMerge(sub, I, NULL, NULL, C);
			cvMulSpectrums(C, G[scale][angle], D, NULL);
			CvScalar sum = cvSum(D);
			cvmSet(dst, y, x, sqrt(sum.val[0]*sum.val[0] + sum.val[1]*sum.val[1]));
		}
	}

	cvReleaseMat(&I);
	cvReleaseMat(&C);
	cvReleaseMat(&D);*/
}

#endif