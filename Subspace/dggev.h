/*  DGGEV computes for a pair of N-by-N real nonsymmetric matrices (A,B)
	the generalized eigenvalues, and the right generalized eigenvectors:
									Ax = lBx
	This function is simple wrapper of the CLAPACK 3.1.1 _dggev function. 
	See:
	http://www.netlib.org/clapack/
	Here are some examples for wrapping c functions for c++ use:
	http://www-heller.harvard.edu/people/shaw/programs/lapack.html

	Parameters:
	A,B:	Concatenate the origin matrix COLUMN BY COLUMN.
	dim:	A and B have the size dim*dim.
	Evecs:	The eigenvectors concatenated one by one. They are NOT sorted
			according to the eigenvalues. If the j-th eigenvalue is real, 
			then v(j) = VR(:,j), the j-th column of VR. If the j-th and
			(j+1)-th eigenvalues form a complex conjugate pair, then
			v(j) = VR(:,j)+i*VR(:,j+1) and v(j+1) = VR(:,j)-i*VR(:,j+1).
			Each eigenvector is scaled so the largest component has
			abs(real part)+abs(imag. part)=1.
	alphaR, alphaI, beta:
			The eigenvalues are formed by (alphaR + alphaI*i)/beta.
			If alphaI(j) is zero, then the j-th eigenvalue is real; 
			if positive, then the j-th and (j+1)-st eigenvalues are 
			a complex conjugate pair, with alpha(j+1) negative.
			Note: the quotients alphaR(j)/beta(j) and alphaI(j)/beta(j)
			may easily over- or underflow, and beta(j) may even be zero.
			Thus, the user should avoid naively computing the ratio
			alpha/beta.
	Returned value:
			= 0:  successful exit
			< 0:  if INFO = -i, the i-th argument had an illegal value.
			= 1,...,N:
				The QZ iteration failed.  No eigenvectors have been
				calculated, but ALPHAR(j), ALPHAI(j), and BETA(j)
				should be correct for j=INFO+1,...,N.
			> N:  =N+1: other than QZ iteration failed in DHGEQZ.
				=N+2: error return from DTGEVC.

	Yan Ke,10/05/20
*/
#ifndef DEBUG
#pragma comment(linker, "/NODEFAULTLIB:msvcrtd.lib")
#endif
//#pragma comment(linker, "/NODEFAULTLIB:libcmt.lib")

extern "C" int __declspec(dllimport) GeneralEig(double *A, double *B, int dim, double *Evecs, 
				double *alphaR, double *alphaI, double *beta);