#ifdef FT_HEADER

#define COMPILE_FT_GABORF

#define PI		3.1415926535897932384626433832795
#define CV_FT_FC1	CV_32FC1 // depth = 32 or 64,32 is faster than 64, while the correct rate seems a little higher
#define CV_FT_FC2	CV_32FC2

typedef float	ft_float; // ft_float = float or double

#else

#include "FaceFeature_Gabor.h"
#include "FaceFeature_GaborF.h"
#include "FaceFeature_pixel.h"

#endif