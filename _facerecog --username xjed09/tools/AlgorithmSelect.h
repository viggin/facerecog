/* select algorithms to compile, should be included in each algorithm .cpp file */

#pragma once

/* Face alignment algorithm */
//#define COMPILE_ALIGN_VJ // Viola-Jones' Adaboost face detect algorithm, built-in OpenCV function
#define COMPILE_ALIGN_ASM // use Yao Wei's ASM algorithm to do face align, more accurate than VJ algorithm


/* light preprocessing algorithm */
//#define COMPILE_LIGHT_YAN	// use Yan's(my) light preprocessing method
#define COMPILE_LIGHT_TAN	// use Tan's light preprocessing method, better than Yan's


/* face feature algorithm */
//#define COMPILE_FT_GaborMag // Gabor magnitude, evenly sampled within the mask, should not be accompanied with SS_DirectMatch
#define COMPILE_FT_GSF // Gabor Surface Feature developed by Yan, better be accompanied with SS_BlkFLD or SS_DirectMatch
//#define COMPILE_FT_LGBP // Local Gabor Binary Patterns, better be accompanied with with SS_BlkFLD or SS_DirectMatch


/* processing algorithms after feature extraction, like subspace methods (the final classifier are nearest neighbor classifier) */
//#define COMPILE_SS_FLD // Fisher LDA(Fisherface)
#define COMPILE_SS_BlkFLD // Blockwise-FLD, better be accompanied with GSF or LGBP
//#define COMPILE_SS_DirectMatch // no subspace, directly match the 2 feature vectors using histogram intersection


/*
	define the type/precision of features, floating-point for GaborMag and char for LBP histogram-like features
*/
#if defined(COMPILE_FT_GSF)
	#define CV_FT_FC1	CV_8UC1
	typedef UINT8	ft_type;
#elif defined(COMPILE_FT_LGBP)
	#define CV_FT_FC1	CV_8UC1
	typedef UINT8	ft_type;
#elif defined(COMPILE_FT_GaborMag)
	#define CV_FT_FC1	CV_32FC1
	#define CV_FT_FC2	CV_32FC2
	typedef float	ft_type;
#endif


/*
	define the type of model vectors(feature vectors after subspace projection)
*/
#if defined(COMPILE_SS_DirectMatch) && (defined(COMPILE_FT_GSF) || defined(COMPILE_FT_LGBP))
	#define CV_MODEL_FC1 CV_8UC1
#else
	#define CV_MODEL_FC1 CV_32FC1
#endif

// if the data type of feature/model is changed, FaceMngr has to be re-compiled