{
// 尝试将Sw和Sb分别计算，前者用很多同一类的样本，后者用很多不同类的样本，正确率降10%
	int classNum = 40, classPopu = 10, Swtc = 8, Sbtn = 2;
	int v1[] = {1,3,5,7,8,9,10,2,4,6}, v2[40];
	for (int i = 0; i < classNum; i++)
		v2[i] = i+1;
	for (int i = 0; i < Swtc; i++)
	{
		for (int j = 1; j <= classPopu; j++)
		{
			picPath.Format("s%d\\%d.png", v2[i], j);
			trainList.push_back(SFInfo(v2[i], picPath));
		}
	}

	for (int i = 1; i <= classNum; i++)
	{
		for (int j = 0; j < Sbtn; j++)
		{
			picPath.Format("s%d\\%d.png", i, v1[j]);
			trainList.push_back(SFInfo(i, picPath));
		}
	}

	for (int i = Swtc; i < classNum; i++)
	{
		for (int j = Sbtn*2; j < classPopu; j++)
		{
			picPath.Format("s%d\\%d.png", v2[i], v1[j]);
			testList.push_back(SFInfo(v2[i], picPath));
		}
	}

void CSubspace::FisherLda1( CvMat *inputs, int *trainIds, int postLdaDim /*= 0*/ )
{
	int	oriDim = inputs->rows, halfcol = inputs->cols/2;
	CvMat *Sw = cvCreateMat(oriDim, oriDim, CV_COEF_FC1),
		*Sb = cvCreateMat(oriDim, oriDim, CV_COEF_FC1); 
	CvMat *Sw1 = cvCloneMat(Sw), *Sb1 = cvCloneMat(Sb);
	CvMat *halfInput = cvCreateMat(oriDim, halfcol, CV_COEF_FC1),
		sub;

	cvGetSubRect(inputs, &sub, cvRect(0,0, halfcol, oriDim));
	CalcSwSb(&sub, trainIds, Sw, Sb1);

	cvGetSubRect(inputs, &sub, cvRect(halfcol,0, halfcol, oriDim));
	CalcSwSb(&sub, trainIds+halfcol, Sw1, Sb);

	CalcLdaSpace(Sw, Sb, postLdaDim);

	cvReleaseMat(&Sw);
	cvReleaseMat(&Sb);
	cvReleaseMat(&Sw1);
	cvReleaseMat(&Sb1);
	cvReleaseMat(&halfInput);
}
}