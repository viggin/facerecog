/*帧频
double		told = (double)cvGetTickCount(), tnew;
tnew = (double)cvGetTickCount();
cout<<(double)cvGetTickFrequency()*1e6/(tnew-told)<<" fps"<<endl;
cout<<(int)cvGetCaptureProperty(capture, CV_CAP_PROP_FPS)<<" cam fps"<<endl; // unusable
told = tnew;
*/

/*测试IPP应用(未成功)
const char* opencv_libraries = 0; 
const char* addon_modules = 0;
printf("%d\n", cvUseOptimized(0));
cvGetModuleInfo( 0, &opencv_libraries,&addon_modules ); 
printf( "OpenCV: %s\r\nAdd-on Modules: %s\r\n.", opencv_libraries, addon_modules); 
system("pause");
*/

/*将cv输出窗口嵌入到应用程序中
cvNamedWindow("Tracking", 0);
HWND hWnd = (HWND) cvGetWindowHandle("Tracking");
HWND hParent = ::GetParent(hWnd);
::SetParent(hWnd, GetDlgItem(IDC_STATIC_OUTPUT)->m_hWnd); // 嵌入到一个STATIC控件
::ShowWindow(hParent, SW_HIDE);
*/

/*绕过ASMlibrary 人脸检测(似乎不好)
bool CFace::AsmDetectFeaturePoints( IplImage *input/*, CvPoint &eyeCorners[4]*//* )
{
	if (!m_bInited && !Init()) return false;

	//int nFaces;
	asm_shape shape;
	asm_shape detshape;

	//bool hasFace = face_detect.DetectFace(&detshapes, nFaces, input);
	//if(hasFace)
	//{
	//	double	maxWidth = 0;
	//	int		maxIdx = 0;
	//	for(int i = 0; i < nFaces; i++)
	//	{
	//		if (detshapes[i].GetWidth() > maxWidth)
	//		{
	//			maxWidth = detshapes[i].GetWidth();
	//			maxIdx = i;
	//		}
	//	}

	//	InitShapeFromDetBox(shape, detshapes[maxIdx],
	//		fit_asm.GetMappingDetShape(), fit_asm.GetMeanFaceWidth());
	IplImage	*faceImg = 0;
	CvSeq		*pFaces = 0;
	HaarDetectFaces(input, 1, faceImg, pFaces);

	if(faceImg)
	{
		CvRect faceRc = *(CvRect *)cvGetSeqElem(pFaces, 0);
		detshape.Resize(2);
		detshape[0] = cvPoint2D32f(0, 0);
		detshape[1] = cvPoint2D32f(faceRc.width, faceRc.height);
		InitShapeFromDetBox(shape, detshape,
			fit_asm.GetMappingDetShape(), fit_asm.GetMeanFaceWidth());
		fit_asm.Fitting(shape, faceImg);

		CvFont font;
		cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, .3, .3);
		for(int i = 0; i < shape.NPoints(); i++)
		{
			char idx[3];
			sprintf_s(idx, 3, "%d", i);
			shape[i].x += faceRc.x;
			shape[i].y += faceRc.y;
			cvPutText(input, idx, cvPointFrom32f(shape[i]), &font, CV_RGB(255, 0, 0));
			cvEllipse(input, cvPointFrom32f(shape[i]), cvSize(2, 2), 0, 0, 0, CV_RGB(0, 255, 0), 2);
		}

	}

	return true;
}*/

/*更通用的fft2
//void fft2(CvArr *src, CvMat *&dest, CvSize *dft_sz/* = NULL*/)
//{
//	CvSize src_sz = cvGetSize(src);
//	if (!dft_sz) 
//	{
//		dft_sz = new CvSize;
//		dft_sz->height = cvGetOptimalDFTSize(src_sz.height);
//		dft_sz->width = cvGetOptimalDFTSize(src_sz.width);
//	}
//	else assert(dft_sz->height > 0 && dft_sz->width > 0);
//
//	CvMat *src_gray32 = cvCreateMat(src_sz.height, src_sz.width, CV_32F);
//	if (CV_IS_IMAGE(src) && ((IplImage *)src)->nChannels == 3) // 3通道矩阵？
//	{
//		IplImage *src_gray = cvCreateImage(src_sz, ((IplImage *)src)->depth,1);
//		cvCvtColor(src, src_gray, CV_RGB2GRAY);
//		cvConvert(src_gray, src_gray32);
//		cvReleaseImage(&src_gray);
//	}
//	else cvConvert(src, src_gray32);
//
//	CvMat tmp;
//	dest = cvCreateMat(dft_sz->height, dft_sz->width, CV_32F);
//	cvGetSubRect(dest, &tmp, cvRect(0,0, src_sz.width,src_sz.height));
//	cvCopy(src_gray32, &tmp);
//	if (dft_sz->width - src_sz.width > 0)
//	{
//		cvGetSubRect( dest, &tmp, cvRect(src_sz.width,0, dft_sz->width - src_sz.width, src_sz.height));
//		cvSetZero( &tmp );
//	}
//	
//	// no need to pad bottom part of dftgA with zeros because of
//	// use nonzerogrows parameter in cvDFT() call below
//	cvDFT(dest, dest, CV_DXT_FORWARD, src_sz.height);
//
//	cvReleaseMat(&src_gray32);
//}
*/

/*显示ASM编号
CvFont font;
		cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, .3, .3);
		for(int i = 0; i < shape.NPoints(); i++)
		{
			char idx[3];
			sprintf_s(idx, 3, "%d", i);
			cvPutText(input, idx, cvPointFrom32f(shape[i]), &font, CV_RGB(255, 0, 0));
			cvEllipse(input, cvPointFrom32f(shape[i]), cvSize(2, 2), 0, 0, 0, CV_RGB(0, 255, 0), 2);
		}
*/

/*
void CfrtestDlg::DetectAndDraw( IplImage *input )
{
	IplImage	*faceImg = 0;
	double		scale = 1.3;
	CvSeq		*pFaces = 0, *pLeftEyes = 0, *pRightEyes = 0;

	align.HaarDetectFaces(input, faceImg);

	for (int i = 0; i < (pFaces ? pFaces->total : 0); i++)
	{
		CvRect	*faceRc = (CvRect*)cvGetSeqElem(pFaces, i);
		markOut(input, faceRc, NULL, scale, 'r', CV_RGB(0, 255, 0));

		align.HaarDetectEyes(faceImg, pLeftEyes, pRightEyes);
		for (int j = 0; j < (pLeftEyes ? pLeftEyes->total : 0); j++)
		{
			CvRect *rc1 = (CvRect*)cvGetSeqElem(pLeftEyes, j);
			markOut(input, 
				&cvRect(rc1->x + rc1->width, cvRound(rc1->y + rc1->height/2), 3, 3), 
				faceRc, scale, 'e', CV_RGB(128,0,255), 3);
		}
		for (int j = 0; j < (pRightEyes ? pRightEyes->total : 0); j++)
		{
			CvRect *rc1 = (CvRect*)cvGetSeqElem(pRightEyes, j);
			markOut(input, 
				&cvRect(cvRound(rc1->x + faceRc->width/2), cvRound(rc1->y + rc1->height/2), 3, 3), 
				faceRc, scale, 'e', CV_RGB(255,0,128), 3);
		}
	}

	cvReleaseImage(&faceImg); // 此处没有遵守谁申请，谁释放的原则，因为faceImg的大小在DetectFace之前是未知的，为了省事，
	// 就在DetectFace函数中create了。
}
*/

/* 用asm_shape类中的函数align.效果一般
	const asm_model *model = m_AsmFit.GetModel();
	asm_shape ms = model->GetMeanShape();
	cvShowImage("before", input);
	double a,b,tx,ty;
	m_shape.AlignTransformation(ms, a,b,tx,ty);
	m_shape.AlignTo(ms);

	/*float trans[] = {a,-b,tx,b,a,ty};
	CvMat map = cvMat(2,3,CV_32FC1,trans);
	DispCvArr(&map,"map");*/
	double angle = cvFastArctan(-b, a),
		scale = sqrt(a*a + b*b);
	CvPoint2D32f leftEye1 = m_shape[31],	rightEye1 = m_shape[36];

	CvMat *map = cvCreateMat(2, 3, CV_32FC1);
	IplImage *after = cvCloneImage(input);
	IplImage *faceCopy = cvCreateImage(cvSize(130,150),IPL_DEPTH_8U,1);
	cv2DRotationMatrix(leftEye, angle, scale ,map);
	cvWarpAffine(input, after, map);

	CvMat sub;
	cvGetSubRect(after, &sub, cvRect(leftEye.x-30, leftEye.y-45, 130,150));
	cvCopy(&sub, faceCopy);
	cvShowImage("after", faceCopy);
	////cvWaitKey();
*/

/* dft in freq domain(lightprep init)
	/*dft_width = cvGetOptimalDFTSize(imgSz.width); // the kernel is small, spatial filtering is a little faster
	dft_height = cvGetOptimalDFTSize(imgSz.height); // and freq filtering needs to solve circle conv problem
	H = cvCreateMat(dft_height, dft_width, CV_32FC2);
	fft2_M32FC1(h, H);*/

	//ShowMagnitude(H, true);

	//CvMat	*tmpR, *tmpI;
	//tmpR = cvCreateMat(dft_height, dft_width, CV_32FC1);
	//Magnitude(H, tmpR);
	//cvNormalize(tmpR, tmpR, 0,1, CV_MINMAX); // use a freq domain highpass filter. not good. why?
	//cvSubRS(tmpR, cvScalar(1), tmpR);
	//tmpI = cvCloneMat(tmpR);
	//cvSetZero(tmpI); // ??
	//cvMerge(tmpR, tmpI, NULL, NULL, H);

	//cvReleaseMat(&tmpR);
	//cvReleaseMat(&tmpI);

	//padded = cvCreateMat(imgSz.height + h_radius*2, imgSz.width +h_radius*2, CV_32FC1); // cvFilter2D seem to auto rep pad
*/

/*更通用的histnorm
void CLightPrep::cvHistNorm(CvArr *src)
{
	CvHistogram *hist = 0;
	CvMat		*lut = 0, *lutS = 0, *lutZ = 0;
	CvHistogram *histeq = NULL;

	cvCopyHist(m_histdst, &histeq);
	cvNormalizeHist(histeq, 1.0);

	int		i, hist_sz = 256;
	CvSize	img_sz;
	float	scale;
	float	*h, *hS;
	int		sum = 0;
	int		type;

	type = cvGetElemType(src);

	hist = cvCreateHist(1, &hist_sz, CV_HIST_ARRAY);
	lut = cvCreateMat(1, 256, CV_8UC1);
	lutS = cvCreateMat(1, hist_sz, CV_8UC1);
	lutZ = cvCreateMat(1, hist_sz, CV_8UC1);
	cvCalcArrHist((CvArr **)&src, hist);
	img_sz = cvGetSize(src);
	scale = 255.f/(img_sz.width * img_sz.height);
	h = (float *)cvPtr1D(hist->bins, 0);
	hS = (float *)cvPtr1D(histeq->bins, 0);
	for(i = 0; i < 256; i++)
	{
		sum += cvRound(hS[i]*255);
		lutS->data.ptr[i] = (uchar)cvRound(sum);
	}
	sum = 0;
	for(i = 0; i < hist_sz; i++)
	{
		sum += cvRound(h[i]);
		lut->data.ptr[i] = (uchar)cvRound(sum*scale);
	}
	for (i = 0; i < 256; i++)
	{
		int k = 0;
		while(lutS->data.ptr[k] < i) k++;
		lutZ->data.ptr[i] = k;
	}
	lut->data.ptr[0] = 0;
	cvLUT(src, src, lut);
	cvLUT(src, src, lutZ);

	cvReleaseHist(&hist);
	cvReleaseMat(&lut);
	cvReleaseMat(&lutS);
	cvReleaseMat(&lutZ);
	cvReleaseHist(&histeq);
}
*/

/*显示所有Gabor Conv结果
	CvMat ***G = new CvMat **[5],***g = new CvMat **[5];
	for (int i = 0; i < 5; i++)
	{
		G[i] = new CvMat *[8];
		g[i] = new CvMat *[8];
		for (int j = 0; j < 8; j++)
		{
			G[i][j] = cvCreateMat(align.m_faceSz.height, align.m_faceSz.width, CV_FC1);
			g[i][j] = cvCreateMat(align.m_faceSz.height, align.m_faceSz.width, CV_FC1);
		}
	}
	CvMat *GG = cvCreateMat(align.m_faceSz.height*5, align.m_faceSz.width*8, CV_FC1);
//。。。
			for (int i = 0; i < 5; i++)
			{
				for (int j = 0; j < 8; j++)
				{
					feature.GaborConv(faceImg32,G[i][j],i,j);
					//DispCvArr(G[i][j],"f");
					//Magnitude(G[i][j],g[i][j]);
				}
			}
			ConcatArrs((CvArr***)G, GG,5,8);
			cvNormalize(GG,GG,0,1,CV_MINMAX);

			cvShowImage("GG",GG);
*/
