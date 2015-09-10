#include "dggev.h"
#include <iostream>
using namespace std;

#pragma comment(lib,"D:\\Yan Ke\\_Programming\\c++\\CLAPACK-3.1.1-VisualStudio\\LIB\\Win32\\libf2c.lib")
#pragma comment(lib,"D:\\Yan Ke\\_Programming\\c++\\CLAPACK-3.1.1-VisualStudio\\LIB\\Win32\\clapack.lib")
#pragma comment(lib,"D:\\Yan Ke\\_Programming\\c++\\CLAPACK-3.1.1-VisualStudio\\LIB\\Win32\\BLAS.lib")
#pragma comment(lib,"D:\\Yan Ke\\_Programming\\c++\\CLAPACK-3.1.1-VisualStudio\\LIB\\Win32\\cblaswrap.lib")

extern "C" int dggev_(char *jobvl, char *jobvr, int *n, 
					  double *a, int *lda, double *b, int *ldb, double *alphar, 
					  double *alphai, double *beta, double *vl, int *ldvl, 
					  double *vr, int *ldvr, double *work, int *lwork, 
					  int *info);

int GeneralEig( double *A, double *B, int dim, double *Evecs, 
				double *alphaR, double *alphaI, double *beta)
{
	char	jobvl = 'N',jobvr = 'V';
	int		lda = dim, ldb = dim;
	int		lwork = 40*dim, info;
	double	*work = new double[lwork];
	double	*vl = NULL, *vr = Evecs;
	int		ldvl = dim,ldvr = dim;

	dggev_(&jobvl, &jobvr, &dim, A, &lda, B, &ldb, 
		alphaR, alphaI, beta, vl, &ldvl, 
		vr, &ldvr, work, &lwork, &info);

	return info;
}