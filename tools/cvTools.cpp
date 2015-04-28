#define DLLSRC

#include "tools.h"


int MessageBox1( LPCTSTR lpText, UINT uType /*= MB_OK | MB_ICONINFORMATION*/, 
				LPCTSTR lpCaption /*= "message"*/, HWND hWnd /*= NULL*/ )
{
	return ::MessageBox(hWnd, lpText, lpCaption, uType);
}


void MarkOut(IplImage *img, CvRect *rc, CvRect *parentRc, double scale, TCHAR shape, 
			 CvScalar color/* = CV_RGB(255, 0, 0)*/, int thickness/* = 1*/)
{
	// if parentRc is set, parentRc is relative to img, and rc is relative to parentRc
	int	px = (parentRc ? parentRc->x : 0),
		py = (parentRc ? parentRc->y : 0);

	if(shape == 'r') // draw a box
	{
		cvRectangle(img,
			cvPoint(cvRound((px + rc->x)*scale), cvRound((py + rc->y)*scale)),
			cvPoint(cvRound((px + rc->x + rc->width)*scale), cvRound((py + rc->y + rc->height)*scale)),
			color, thickness, 8, 0);
	}
	else if (shape == 'e') // draw an ellipse
	{
		CvPoint		center;
		CvSize		axes;
		center.x = cvRound((px+rc->x + rc->width*0.5)*scale);
		center.y = cvRound((py+rc->y + rc->height*0.5)*scale);
		axes.width = cvRound(rc->width*0.5*scale);
		axes.height = cvRound(rc->height*0.5*scale);

		//printf("w:%d h:%d\n", axes.width, axes.height);
		cvEllipse(img, center, axes, 0, 0, 360, color, thickness);
	}
	else if (shape == 'l') // draw a line
	{
		cvLine(img,
			cvPoint(cvRound((px + rc->x)*scale), cvRound((py + rc->y + rc->height/2)*scale)),
			cvPoint(cvRound((px + rc->x + rc->width)*scale), cvRound((py + rc->y + rc->height/2)*scale)),
			color, thickness);
	}

}

#ifdef _DEBUG

DLLEXP void DispCvArr( CvArr *a, TCHAR *varName /*= "CvArr"*/, 
					  bool showTranspose /*= false*/, TCHAR *format /*= NULL*/ )
{
	/*CvMat	tmpHeader, *m = cvGetMat(a, &tmpHeader);
	int		depth = CV_MAT_DEPTH(m->type);*/
	CString str, tmpstr;
	CvSize	sz = cvGetSize(a);
	if (showTranspose) str.Format("\n%s(T) : %d x %d", varName, sz.height, sz.width);
	else str.Format("\n%s : %d x %d", varName, sz.height, sz.width);
	TRACE(str);
	TCHAR *form = "%f\t";
	if (format) form = format;
	if (!showTranspose)
		for (int i = 0; i < sz.height; i++)
		{
			str = "\n";
			for (int j = 0; j < sz.width; j++)
			{
				double d = cvGetReal2D(a, i, j);
				tmpstr.Format(form, d);
				str += tmpstr;
			}
			TRACE(str);
		}
	else
		for (int i = 0; i < sz.width; i++)
		{
			str = "\n";
			for (int j = 0; j < sz.height; j++)
			{
				double d = cvGetReal2D(a, j, i);
				tmpstr.Format(form, d);
				str += tmpstr;
			}
			TRACE(str);
		}
	
	TRACE("\n");
}

DLLEXP void DispCvArrMultiChannel( CvArr *a, TCHAR *varName /*= "CvArr"*/ )
{
	CvMat tmpHeader, *m;
	m = cvGetMat(a, &tmpHeader);
	int cnNum = CV_MAT_CN(m->type), depth = CV_MAT_DEPTH(m->type);
	CvMat **p = new CvMat *[4];
	for (int i = 0; i < 4; i++)
		p[i] = ( i >= cnNum ? NULL : 
		cvCreateMat(m->rows, m->cols, CV_MAKETYPE(depth, 1)) );
	cvSplit(m, p[0], p[1], p[2], p[3]);
	for (int i = 0; i < cnNum; i++)
	{
		CString str;
		str.Format("%s : channel %d", varName, i);
		DispCvArr(p[i], str.GetBuffer());
	}

	for (int i = 0; i < cnNum; i++)
		cvReleaseMat(&p[i]);
	delete []p;
}

#endif // _DEBUG

void ConcatArrs( CvArr ***pppa, CvMat *dst, int rowNum, int colNum, int *colNums /*= NULL */)
{
	CvMat	tmpHeader, *sub = 0;
	int		m = 0, n;
	CvSize	sz;
	for (int i = 0; i < rowNum; i++)
	{
		n = 0;
		for (int j = 0; j < (colNum ? colNum : colNums[i]); j++)
		{
			sz = cvGetSize(pppa[i][j]);
			sub = cvGetSubRect(dst, &tmpHeader, cvRect(n,m, sz.width, sz.height));
			cvCopy(pppa[i][j], sub);
			n += sz.width;
		}
		m += sz.height;
	}
}

CvSize CheckConcat( CvArr ***pppa, int rowNum, int colNum, int *colNums )
{
	int **h = new int *[rowNum], *w = new int[rowNum], height = 0;
	for (int i = 0; i < rowNum; i++)
	{
		h[i] = new int[(colNum ? colNum : colNums[i])];
		w[i] = 0;
		for (int j = 0; j < (colNum ? colNum : colNums[i]); j++)
		{
			h[i][j] = cvGetSize(pppa[i][j]).height;
			w[i] += cvGetSize(pppa[i][j]).width;
		}
	}
	for (int i = 0; i < rowNum; i++)
	{
		assert(w[i] == w[0]);
		for (int j = 0; j < (colNum ? colNum : colNums[i]); j++)
			assert(h[i][j] == h[i][0]);
		height += h[i][0];
		delete []h[i];
	}
	delete []h;
	int width = w[0];
	delete []w;
	return cvSize(width, height);
}

void WriteStringLine(ofstream &os, CString str)
{
	os<<str<<endl;
}

void ReadStringLine(ifstream &is, CString &str)
{
	TCHAR t[100];
	is.getline(t, 100);
	str = t;
}

void WriteIntText(ofstream &os, int n, CString name /*= ""*/)
{
	if (!name.IsEmpty()) WriteStringLine(os, name);
	os<<n<<endl;
}

void ReadIntText(ifstream &is, int &n, bool hasName /*= true*/)
{
	if (hasName)
	{
		CString tmp;
		ReadStringLine(is, tmp);
	}
	int m;
	is>>m;
	n = m;
	//is>>n;
	TCHAR c;
	is.get(c);
}

void WriteCvMatBin(ofstream &os, CvMat *mat, CString name /*= ""*/)
{
	if (!name.IsEmpty()) WriteStringLine(os, name);
	os.write((TCHAR*)mat->data.ptr, mat->step * mat->rows);
	os.put('\n');
}

void ReadCvMatBin(ifstream &is, CvMat *mat, bool hasName /*= true*/)
{
	if (hasName)
	{
		CString tmp;
		ReadStringLine(is, tmp);
	}
	is.read((TCHAR*)mat->data.ptr, mat->step * mat->rows);
	TCHAR c;
	is.get(c);
}

static double g_tic;
void tic()
{
	g_tic = (double)cvGetTickCount();
}

double toc()
{
	return ((double)cvGetTickCount() - g_tic) / ((double)cvGetTickFrequency()*1e6);
}