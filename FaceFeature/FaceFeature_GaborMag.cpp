/*
	First set a mask on the face, then evenly select smpPtNum sample points within the mask,
	then compute scaleNum*angleNum Gabor filter results on each point using DFT,
	finally use the magnitude of these scaleNum*angleNum*smpPtNum features as a feature vector.
*/

#define DLLSRC

#include "FaceFeature.h"
#include "../tools/AlgorithmSelect.h"

#ifdef COMPILE_FT_GaborMag

#define PI	CV_PI

// files needed
#define MASK_FN1	"mask.bmp"


// Gabor related coefs and variables
static double	kmax, f, sigma, th;
static int		scaleNum, angleNum, kernelRadius;
static int		*scales; // Gabor kernel group: scales of each angle

static CvSize	g_faceSz;
static int		kernelWidth, fsz;
static CvMat	***G;
static CvMat	*tPicR, *tPicI, *tPicF0, *tPicF;


// down-sample related coefs and variables
static int		sampleIntv;

static int		smpPtNum;
static int		*smpPts; // the index of sample points, convinient for direct addressing
static CvMat	*g_mask;

void InitGaborKernel();

// similar with InitMask in LightPrep.cpp
bool InitMask1();

// Down-sample is even, sample points are only selected in the mask.
int InitSamplePoints();

void ShowGaborFace1(CvArr *faceImg32);


int InitFeature( CvSize imgSize )
{
	kmax = PI/2;
	f = sqrt(double(2));
	sigma = 2*PI;
	PARAM scaleNum = 5;
	PARAM angleNum = 8;
	th = 5e-3;

	// kernel radius is automatically calculated, the criterion is the max magnitude on the 
	// border should be less than TH
	kernelRadius = (int)ceil( sqrt( -log( th*sigma*sigma/kmax/kmax ) *2*sigma*sigma/kmax/kmax ) );
	scales = new int[scaleNum];
	for (int i = 0; i < scaleNum; i++)
		scales[i] = i;

	PARAM sampleIntv = 4; // the distance(row or col) between two adjacent sample points is sampleIntv

	g_faceSz = imgSize;
	InitGaborKernel();
	tPicR = cvCreateMat(fsz, fsz, CV_FT_FC1);
	tPicI = cvCreateMat(fsz, fsz, CV_FT_FC1);
	tPicF0 = cvCreateMat(fsz, fsz, CV_FT_FC2);
	tPicF = cvCreateMat(fsz, fsz, CV_FT_FC2);
	if (! InitMask1())
	{
		smpPts = NULL;
		return 0;
	}
	return InitSamplePoints();
}

void InitGaborKernel()
{
	fsz = cvGetOptimalDFTSize( max(g_faceSz.height, g_faceSz.width) + kernelRadius*2 );
	kernelWidth = kernelRadius*2 + 1;

	G = new CvMat **[scaleNum];
	CvMat *re = cvCreateMat(kernelWidth, kernelWidth, CV_FT_FC1),
		*im = cvCreateMat(kernelWidth, kernelWidth, CV_FT_FC1),
		*g = cvCreateMat(kernelWidth, kernelWidth, CV_FT_FC2);

	for (int i = 0; i < scaleNum; i++)
	{
		G[i] = new CvMat *[angleNum];
		double k = kmax/pow(f, scales[i]),
			tmpV = k*k /sigma/sigma;

		for (int j = 0; j < angleNum; j++)
		{
			double phi = PI/angleNum*j;
			double tmpV0, tmpV1;

			for (int y = -kernelRadius; y <= kernelRadius; y++)
			{
				for (int x = -kernelRadius; x <= kernelRadius; x++)
				{
					tmpV0 = tmpV * exp(-tmpV * (x*x + y*y) / 2.0);
					tmpV1 = k*cos(phi)*x + k*sin(phi)*y;
					cvmSet( re, y+kernelRadius, x+kernelRadius, tmpV0 * cos(tmpV1) );
					cvmSet( im, y+kernelRadius, x+kernelRadius, tmpV0 * sin(tmpV1) );

					/* G{scale_idx,angle_idx} = k^2/sigma^2 * exp(-k^2*(X.^2+Y.^2)/2/sigma^2)...
						.*(exp(1i*(k*cos(phi)*X+k*sin(phi)*Y) - DC)); */
				}
			}

			cvMerge(re, im, NULL, NULL, g); // complex-value kernel
			G[i][j] = cvCreateMat(fsz, fsz, CV_FT_FC2);
			cvCopyMakeBorder(g, G[i][j], cvPoint(0,0), IPL_BORDER_CONSTANT);
			cvDFT(G[i][j], G[i][j], CV_DXT_FORWARD, kernelWidth); // kernel in freq domail
			cvSet2D(G[i][j], 0,0, cvScalar(0)); // reduce DC
			//ShowMagnitude(G[i][j],true);
		}
	}
	cvReleaseMat(&re);
	cvReleaseMat(&im);
	cvReleaseMat(&g);
}

int InitSamplePoints()
{
	/* equivalent MATLAB code
	ssz = floor((sz-1)/sampleRate);
	[idxX idxY] = meshgrid(0:ssz(2),0:ssz(1));
	topleft = floor(rem(sz-1,sampleRate)/2)+1;
	idxX = idxX*sampleRate+topleft(2);
	idxY = idxY*sampleRate+topleft(1);
	samplePos = sub2ind(sz,idxY,idxX);
	*/
	CvSize ssz = cvSize( (g_faceSz.width - 1)/sampleIntv + 1, 
		(g_faceSz.height - 1)/sampleIntv + 1 );
	CvPoint topleft = cvPoint(((g_faceSz.width - 1) % sampleIntv)/2,
		((g_faceSz.height - 1) % sampleIntv)/2);

	CvPoint pt;
	int *startPts = new int[ssz.height]; // X coord of the starting sample point of each row
	int *endPts = new int[ssz.height]; // X coord of the last sample point of each row
	smpPtNum = 0;
	for (int i = 0; i < ssz.height; i++)
	{
		pt.y = topleft.y + i*sampleIntv;
		startPts[i] = topleft.x;
		endPts[i] = 0;
		for (int j = 0; j < ssz.width; j++)
		{
			pt.x = topleft.x + j*sampleIntv;
			if (g_mask && cvGetReal2D(g_mask, pt.y, pt.x) == 0)
			{
				if (endPts[i] == 0) // as a flag
					startPts[i] = pt.x + sampleIntv;
			}
			else
			{
				endPts[i] = pt.x;
				smpPtNum++;
			}
		}
	}

	//cvZero(tPicR);
	smpPts = new int[smpPtNum];
	int idx = 0;
	for (int y = 0; y < ssz.height; y++)
	{
		int realY = topleft.y + kernelRadius * 2 + y*sampleIntv; // kernelRadius * 2: avoid the circular effect of fft
		for (int x = startPts[y]; x <= endPts[y]; x += sampleIntv)
		{
			int realX = x + kernelRadius*2;
			smpPts[idx++] = realY*fsz + realX;
			//*((ft_float *)(tPicR->data.ptr)+smpPts[idx-1]) = 1;
		}
	}

	//cvShowImage("p",tPicR);
	//cvWaitKey();
	delete []startPts;
	delete []endPts;
	return smpPtNum * scaleNum * angleNum;
}

bool InitMask1()
{
	IplImage *maskOri = cvLoadImage(MASK_FN1, CV_LOAD_IMAGE_GRAYSCALE);
	if(!maskOri) {
		CString msg;
		msg.Format("Can't load mask image %s.", MASK_FN1);
		::MessageBox1(msg);
		g_mask = NULL;
		return false;
	}

	g_mask = cvCreateMat(g_faceSz.height, g_faceSz.width, CV_8UC1);
	cvResize(maskOri, g_mask);
	cvReleaseImage(&maskOri);
	return true;
}

void GetFeature( CvArr *faceImg32, CvMat *ft32 )
{
	//ShowGaborFace(faceImg32);
	cvCopyMakeBorder(faceImg32, tPicR, cvPoint(kernelRadius, kernelRadius), IPL_BORDER_REPLICATE);
	cvZero(tPicI);
	cvMerge(tPicR, tPicI, NULL, NULL, tPicF0);
	cvDFT(tPicF0, tPicF0, CV_DXT_FORWARD, g_faceSz.height + kernelRadius*2);

	ft_type *pSrc, *pDst = (ft_type *)(ft32->data.ptr);
	ft_type *pBase = (ft_type *)(tPicF->data.ptr);
	ft_type f1, f2;
	int dstStep = ft32->step / sizeof(ft_type);
	for (int i = 0; i < scaleNum; i++)
	{
		for (int j = 0; j < angleNum; j++)
		{
			cvMulSpectrums(tPicF0, G[i][j], tPicF, NULL);
			cvDFT(tPicF, tPicF, CV_DXT_INVERSE_SCALE, g_faceSz.height + kernelRadius*2); // filter result
			for (int p = 0; p < smpPtNum; p++)
			{
				pSrc = pBase + smpPts[p]*2;
				f1 = *pSrc;
				f2 = *(pSrc+1);
				*pDst = sqrt(f1*f1 + f2*f2); // norm
				pDst += dstStep;
			}
		}
	}
}

void ShowGaborFace1( CvArr *faceImg32 )
{
	cvCopyMakeBorder(faceImg32, tPicR, cvPoint(kernelRadius, kernelRadius), IPL_BORDER_REPLICATE);
	cvZero(tPicI);
	cvMerge(tPicR, tPicI, NULL, NULL, tPicF0);
	cvDFT(tPicF0, tPicF0, CV_DXT_FORWARD, g_faceSz.height + kernelRadius*2);

	CvMat *gf = cvCreateMat(scaleNum*g_faceSz.height, angleNum*g_faceSz.width, CV_FT_FC1);
	CvMat subsrc, subdst;
	for (int i = 0; i < scaleNum; i++)
	{
		for (int j = 0; j < angleNum; j++)
		{
			cvMulSpectrums(tPicF0, G[i][j], tPicF, NULL);
			cvDFT(tPicF, tPicF, CV_DXT_INVERSE_SCALE, g_faceSz.height + kernelRadius*2);
			Magnitude(tPicF, tPicR);
			cvGetSubRect(tPicR, &subsrc, cvRect(kernelWidth-1,kernelWidth-1, g_faceSz.width, g_faceSz.height));
			cvGetSubRect(gf, &subdst, 
				cvRect(j*g_faceSz.width, i*g_faceSz.height, g_faceSz.width, g_faceSz.height));
			cvCopy(&subsrc, &subdst);
		}
	}

	cvShowImage("img", faceImg32);
	cvShowImage("gabor", gf);
	cvWaitKey();
	cvReleaseMat(&gf);
}

void ReleaseFeature()
{
	delete [scaleNum]scales;
	for (int i = 0; i < scaleNum; i++) // release the kernel
	{
		for (int j = 0; j < angleNum; j++)
			cvReleaseMat(&G[i][j]);
		delete [angleNum]G[i]; // same with delete []G[i]
	}
	delete [scaleNum]G;

	cvReleaseMat(&tPicR);
	cvReleaseMat(&tPicI);
	cvReleaseMat(&tPicF);
	cvReleaseMat(&g_mask);

	delete []smpPts;
}

#endif