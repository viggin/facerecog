/*
	convolving an image with Gabor filters using FFT
*/

#define DLLSRC

#include "Gabor.h"

#define PI CV_PI

// Gabor related coefs and variables
PARAM double	kmax, f, sigma, th;
PARAM int		scaleNum, angleNum, kernelRadius;
PARAM int		*scales; // Gabor kernel group: scales of each angle

static CvSize	g_faceSz;
static int		kernelWidth, fsz;
static CvMat	***G; // the Gabor kernel in freq domain
static CvMat	*tPicR, *tPicI, *tPicF0, *tPicF;

CvMat ***gface; // the convolved Gabor face magnitude

static void InitGaborKernel();


CvSize InitGabor( CvSize imgSize )
{
	kmax = PI/2;
	f = sqrt(double(2));
	sigma = 2*PI;
	scaleNum = 5;
	angleNum = 8;
	th = 5e-3;

	// kernel radius is automatically calculated, the criterion is the max magnitude on the 
	// border should be less than TH
	kernelRadius = (int)ceil( sqrt( -log( th*sigma*sigma/kmax/kmax ) *2*sigma*sigma/kmax/kmax ) );
	scales = new int[scaleNum];
	for (int i = 0; i < scaleNum; i++)
		scales[i] = i;

	g_faceSz = imgSize;
	InitGaborKernel();
	tPicR = cvCreateMat(fsz, fsz, CV_32FC1);
	tPicI = cvCreateMat(fsz, fsz, CV_32FC1);
	tPicF0 = cvCreateMat(fsz, fsz, CV_32FC2);
	tPicF = cvCreateMat(fsz, fsz, CV_32FC2);
	
	gface = new CvMat **[scaleNum];
	for (int i = 0; i < scaleNum; i++)
	{
		gface[i] = new CvMat *[angleNum];
		for (int j = 0; j < angleNum; j++)
			gface[i][j] = cvCreateMat(g_faceSz.height, g_faceSz.width, CV_32FC1);
	}

	return cvSize(angleNum,scaleNum);
}

static void InitGaborKernel()
{
	fsz = cvGetOptimalDFTSize( max(g_faceSz.height, g_faceSz.width) + kernelRadius*2 );
	kernelWidth = kernelRadius*2 + 1;

	G = new CvMat **[scaleNum];
	CvMat *re = cvCreateMat(kernelWidth, kernelWidth, CV_32FC1),
		*im = cvCreateMat(kernelWidth, kernelWidth, CV_32FC1),
		*g = cvCreateMat(kernelWidth, kernelWidth, CV_32FC2);

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
			G[i][j] = cvCreateMat(fsz, fsz, CV_32FC2);
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

CvMat*** GaborConv( CvArr *faceImg32 )
{
	//ShowGaborFace(faceImg32);
	CvMat sub;
	cvCopyMakeBorder(faceImg32, tPicR, cvPoint(kernelRadius, kernelRadius), IPL_BORDER_REPLICATE);
	cvZero(tPicI);
	cvMerge(tPicR, tPicI, NULL, NULL, tPicF0);
	cvDFT(tPicF0, tPicF0, CV_DXT_FORWARD, g_faceSz.height + kernelRadius*2);

	for (int i = 0; i < scaleNum; i++)
	{
		for (int j = 0; j < angleNum; j++)
		{
			cvMulSpectrums(tPicF0, G[i][j], tPicF, NULL);
			cvDFT(tPicF, tPicF, CV_DXT_INVERSE_SCALE, g_faceSz.height + kernelRadius*2); // filter result
			Magnitude(tPicF,tPicR);
			cvGetSubRect(tPicR, &sub, cvRect(kernelRadius*2,kernelRadius*2, g_faceSz.width, g_faceSz.height));
			cvCopy(&sub, gface[i][j]);
		}
	}

	return gface;
}

void ShowGaborFace( CvArr *faceImg32 )
{
	CvMat ***res = GaborConv(faceImg32);
	CvMat *gf = cvCreateMat(scaleNum*g_faceSz.height, angleNum*g_faceSz.width, CV_32FC1);
	CvMat subdst;
	for (int i = 0; i < scaleNum; i++)
	{
		for (int j = 0; j < angleNum; j++)
		{
			cvGetSubRect(gf, &subdst, 
				cvRect(j*g_faceSz.width, i*g_faceSz.height, g_faceSz.width, g_faceSz.height));
			cvCopy(&(res[i][j]), &subdst);
		}
	}

	cvShowImage("img", faceImg32);
	cvShowImage("gabor", gf);
	cvWaitKey();
	cvReleaseMat(&gf);
}

void ReleaseGabor()
{
	delete [scaleNum]scales;
	for (int i = 0; i < scaleNum; i++) // release the kernel
	{
		for (int j = 0; j < angleNum; j++)
		{
			cvReleaseMat(&G[i][j]);
			cvReleaseMat(&gface[i][j]);
		}
		delete [angleNum]G[i]; // same with delete []G[i]
		delete [angleNum]gface[i]; // same with delete []G[i]
	}
	delete [scaleNum]G;
	delete [scaleNum]gface;

	cvReleaseMat(&tPicR);
	cvReleaseMat(&tPicI);
	cvReleaseMat(&tPicF);
	cvReleaseMat(&tPicF0);
}
