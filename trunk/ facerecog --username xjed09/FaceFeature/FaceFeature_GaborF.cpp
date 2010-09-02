#define DLLSRC

#include "FaceFeature_GaborF.h"

#ifdef COMPILE_FT_GABORF


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

	sampleIntv = 6;
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

	cvReleaseMat(&tPicR);
	cvReleaseMat(&tPicI);
	cvReleaseMat(&tPicF);

	delete []smpPts;
}

int CFaceFeature::Init( CvSize imgSize, CvMat *mask )
{
	m_faceSz = imgSize;
	InitGaborKernel();
	tPicR = cvCreateMat(fsz, fsz, CV_FT_FC1),
	tPicI = cvCreateMat(fsz, fsz, CV_FT_FC1),
	tPicF0 = cvCreateMat(fsz, fsz, CV_FT_FC2);
	tPicF = cvCreateMat(fsz, fsz, CV_FT_FC2);
	return InitSamplePoints(mask);
}

void CFaceFeature::InitGaborKernel()
{
	fsz = cvGetOptimalDFTSize( max(m_faceSz.height, m_faceSz.width) + kernelRadius*2 );
	kernelWidth = kernelRadius*2 + 1;

	G = new CvMat **[scaleNum];
	CvMat *re = cvCreateMat(kernelWidth, kernelWidth, CV_FT_FC1),
		*im = cvCreateMat(kernelWidth, kernelWidth, CV_FT_FC1),
		*g = cvCreateMat(kernelWidth, kernelWidth, CV_FT_FC2);

	for (int i = 0; i < scaleNum; i++)
	{
		G[i] = new CvMat *[angleNum];
		double k = kmax/pow(f, this->scales[i]),
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

			cvMerge(re, im, NULL, NULL, g); // 复值小波
			G[i][j] = cvCreateMat(fsz, fsz, CV_FT_FC2);
			cvCopyMakeBorder(g, G[i][j], cvPoint(0,0), IPL_BORDER_CONSTANT);
			cvDFT(G[i][j], G[i][j], CV_DXT_FORWARD, kernelWidth); // 得到频域小波
			cvSet2D(G[i][j], 0,0, cvScalar(0)); // 去直流
			//ShowMagnitude(G[i][j],true);
		}
	}
	cvReleaseMat(&re);
	cvReleaseMat(&im);
	cvReleaseMat(&g);
}

int CFaceFeature::InitSamplePoints(CvMat *mask)
{
	/*
	ssz = floor((sz-1)/sampleRate);
	[idxX idxY] = meshgrid(0:ssz(2),0:ssz(1));
	topleft = floor(rem(sz-1,sampleRate)/2)+1;
	idxX = idxX*sampleRate+topleft(2);
	idxY = idxY*sampleRate+topleft(1);
	samplePos = sub2ind(sz,idxY,idxX);
	*/
	CvSize ssz = cvSize( (m_faceSz.width - 1)/sampleIntv + 1, 
		(m_faceSz.height - 1)/sampleIntv + 1 );
	CvPoint topleft = cvPoint(((m_faceSz.width - 1) % sampleIntv)/2,
		((m_faceSz.height - 1) % sampleIntv)/2);

	CvPoint pt;
	int *startPts = new int[ssz.height]; // 每行的起始采样点的x坐标
	int *endPts = new int[ssz.height]; // 每行最后一个采样点的x坐标
	smpPtNum = 0;
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
				smpPtNum++;
			}
		}
	}

	//cvZero(tPicR);
	smpPts = new int[smpPtNum];
	int idx = 0;
	for (int y = 0; y < ssz.height; y++)
	{
		int realY = topleft.y + kernelRadius * 2 + y*sampleIntv;
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

void CFaceFeature::GetFeature( CvArr *faceImg32, CvMat *ft32 )
{
	//ShowGaborFace(faceImg32);
	cvCopyMakeBorder(faceImg32, tPicR, cvPoint(kernelRadius, kernelRadius), IPL_BORDER_REPLICATE);
	cvZero(tPicI);
	cvMerge(tPicR, tPicI, NULL, NULL, tPicF0);
	cvDFT(tPicF0, tPicF0, CV_DXT_FORWARD, m_faceSz.height + kernelRadius*2);

	ft_float *pSrc, *pDst = (ft_float *)(ft32->data.ptr);
	ft_float *pBase = (ft_float *)(tPicF->data.ptr);
	ft_float f1, f2;
	int dstStep = ft32->step / sizeof(ft_float);
	for (int i = 0; i < scaleNum; i++)
	{
		for (int j = 0; j < angleNum; j++)
		{
			cvMulSpectrums(tPicF0, G[i][j], tPicF, NULL);
			cvDFT(tPicF, tPicF, CV_DXT_INVERSE_SCALE, m_faceSz.height + kernelRadius*2); // 卷积结果
			for (int p = 0; p < smpPtNum; p++)
			{
				pSrc = pBase + smpPts[p]*2;
				f1 = *pSrc;
				f2 = *(pSrc+1);
				*pDst = sqrt(f1*f1 + f2*f2); // 求模
				pDst += dstStep;
			}
		}
	}
}

void CFaceFeature::ShowGaborFace( CvArr *faceImg32 )
{
	cvCopyMakeBorder(faceImg32, tPicR, cvPoint(kernelRadius, kernelRadius), IPL_BORDER_REPLICATE);
	cvZero(tPicI);
	cvMerge(tPicR, tPicI, NULL, NULL, tPicF0);
	cvDFT(tPicF0, tPicF0, CV_DXT_FORWARD, m_faceSz.height + kernelRadius*2);

	CvMat *gf = cvCreateMat(scaleNum*m_faceSz.height, angleNum*m_faceSz.width, CV_FT_FC1);
	CvMat subsrc, subdst;
	for (int i = 0; i < scaleNum; i++)
	{
		for (int j = 0; j < angleNum; j++)
		{
			cvMulSpectrums(tPicF0, G[i][j], tPicF, NULL);
			cvDFT(tPicF, tPicF, CV_DXT_INVERSE_SCALE, m_faceSz.height + kernelRadius*2); // 卷积结果
			Magnitude(tPicF, tPicR);
			cvGetSubRect(tPicR, &subsrc, cvRect(kernelWidth-1,kernelWidth-1, m_faceSz.width, m_faceSz.height));
			cvGetSubRect(gf, &subdst, 
				cvRect(j*m_faceSz.width, i*m_faceSz.height, m_faceSz.width, m_faceSz.height));
			cvCopy(&subsrc, &subdst);
		}
	}

	cvShowImage("img", faceImg32);
	cvShowImage("gabor", gf);
	cvWaitKey();
	cvReleaseMat(&gf);
}

#endif