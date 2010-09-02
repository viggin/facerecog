/****************************************************************************
*						ASMLibrary
*			http://code.google.com/p/asmlibrary
* Copyright (c) 2008 by Yao Wei, all rights reserved.
* Author:      	Yao Wei
* Contact:     	njustyw@gmail.com
****************************************************************************/

#ifndef _ASM_LIBRARY_H_
#define _ASM_LIBRARY_H_

#include <stdio.h>
#include "../opencv/cv.h"
#include "../opencv/highgui.h"

#define SearchStep		3

#ifdef WIN32
#define ASMLIB __declspec(dllexport)
#else
#define ASMLIB 
#endif

/** Class for 2d shape data. */
class ASMLIB asm_shape
{
public:
    /** Constructor */
	asm_shape();
    
	/** Copy Constructor */
	asm_shape(const asm_shape &v);
    
	/** Destructor */
    ~asm_shape();

	/**
     Access elements.
     @param i Index of points.
     @return   Point at the certain index.
	*/
	const CvPoint2D32f operator[](int i)const{ return m_vPoints[i];	}
	
	/**
     Access elements.
     @param i Index of points.
     @return   Point at the certain index.
	*/
	CvPoint2D32f& operator[](int i){ return m_vPoints[i];	}
	
	/**
     Get the number of points.
     @return   Number of points.
	*/
	inline const int NPoints()const{ return	m_nPoints; }

    /**
     Override of operator=.
    */
    asm_shape&			operator=(const asm_shape &s);
    
	/**
     Override of operator=.
    */
	asm_shape&			operator=(double value);
    
	/**
     Override of operator+.
    */
    const asm_shape		operator+(const asm_shape &s)const;
    
	/**
     Override of operator+=.
    */
    asm_shape&			operator+=(const asm_shape &s);
    
	/**
     Override of operator-.
    */
    const asm_shape     operator-(const asm_shape &s)const;
    
	/**
     Override of operator-=.
    */
    asm_shape&			operator-=(const asm_shape &s);
    
	/**
     Override of operator*.
    */
    const asm_shape     operator*(double value)const;
    
	/**
     Override of operator*=.
    */
    asm_shape&			operator*=(double value);
    
	/**
     Override of operator*.
    */
    double				operator*(const asm_shape &s)const;
    
	/**
     Override of operator/.
    */
    const asm_shape     operator/(double value)const;
    
	/**
     Override of operator/=.
    */
    asm_shape&			operator/=(double value);

	/**
     Release memory.
    */
    void    Clear();
    
	/**
     Allocate memory.
	 @param length Number of of shape points.
    */
    void    Resize(int length);
    
	/**
     Read points from file.
	 @param filename  Filename the shape located in.
     @return   true on pts format, false on asf format, exit otherwise.
    */
    bool	ReadAnnotations(const char* filename);
	
	/**
     Read points from asf format file.
	 @param filename  Filename the shape located in.
    */
    void    ReadFromASF(const char*filename);
	
	/**
     Read points from pts format file.
	 @param filename  Filename the shape located in.
    */
    void	ReadFromPTS(const char*filename);
	
	/**
     Write shape data into file stream.
	 @param f  stream to write to.
    */
	void	Write(FILE* f);
	
	/**
     Read shape data from file stream.
	 @param f  stream to read from.
    */
	void	Read(FILE* f);
	
	/**
     Calculate minimum x-direction value of shape.
    */
	const double  MinX()const;
    
	/**
     Calculate minimum y-direction value of shape.
    */
	const double  MinY()const;
    
	/**
     Calculate maximum x-direction value of shape.
    */
	const double  MaxX()const;
    
	/**
     Calculate maximum y-direction value of shape.
    */
	const double  MaxY()const;
	
	/**
     Calculate the left and right index for x-direction in the shape.
	 @param iLeft Index of points in x-direction which has the minimum x.
	 @param iRight Index of points in x-direction which has the maximum x.
    */
	void		  GetLeftRight(int& iLeft, int& iRight)const;
    
	/**
     Calculate width of shape.
	 @param iLeft Index of points in x-direction which has the minimum x.
	 @param iRight Index of points in x-direction which has the maximum x.
    */
	const double  GetWidth(int iLeft = -1, int iRight = -1)const;
	
	/**
     Calculate height of shape.
    */
	const double  GetHeight()const { return MaxY()-MinY();	}
	
    /**
     Calculate center of gravity for shape.
	 @param x Value of center in x-direction.
	 @param y Value of center in y-direction.
    */
	void    COG(double &x, double &y)const;
    
	/**
     Translate the shape to make its center locate at (0, 0).
	*/
	void    Centralize();
    
	/**
	 Translate the shape.
	 @param x Value of translation factor in x-direction.
	 @param y Value of translation factor in y-direction.
    */
	void    Translate(double x, double y);
    
	/**
     Scale shape by an uniform factor.
	 @param s Scaling factor.
	*/
	void    Scale(double s);
    
	/**
     Rotate shape by anti clock-wise.
	 @param theta Angle to be rotated.
	*/
	void    Rotate(double theta);
	
	/**
     Scale shape in x and y direction respectively.
	 @param sx Scaling factor in x-direction.
	 @param sy Scaling factor in y-direction.
	*/
	void    ScaleXY(double sx, double sy);
	
	/**
     Normalize shape so that its center is (0, 0) and norm is 1.
	 @return the 2-norm of original shape.
	*/
	double	Normalize();
	
	
	enum{ LU, SVD, Direct };

	
	/**
	 Calculate the similarity transform T(a,b,tx,ty) between one shape and another reference shape. 
	 Similarity transform: T(a,b,tx,ty) = [a -b Tx; b  a Ty; 0  0  1]
	 @param ref Reference shape.
	 @param a  Return s*cos(theta) in form of similarity transform.
	 @param b  Return s*sin(theta) in form of similarity transform.
	 @param tx  Return Tx in form of similarity transform.
	 @param ty  Return Ty in form of similarity transform.
	 @param method  Method of similarity transform.
	*/
	void    AlignTransformation(const asm_shape &ref, 
			double &a, double &b, double &tx, double &ty, int method = SVD)const;
    
	/**
	 Align the shape to the reference shape. 
	 @param ref Reference shape.
	 @param method  Method of similarity transform.
	*/
	void    AlignTo(const asm_shape &ref, int method = SVD);
    
	/**
	 Transform Shape using similarity transform T(a,b,tx,ty). 
	*/
	void    TransformPose(double a, double b, double tx, double ty);

	/**
	 Calculate the angular bisector between two lines Pi-Pj and Pj-Pk. 
	 @param i Index of point vertex.
	 @param j Index of point vertex.
	 @param k Index of point vertex.
	 @return Angular bisector vector in form of (cosx, sinx). 
	*/
	CvPoint2D32f CalcBisector(int i, int j, int k)const;

	/**
	 Calculate the Euclidean norm. 
	 @return Euclidean norm.
	*/
	double  GetNorm2()const;

	/**
	 Calculate the normal vector at certain vertex around the shape contour. 
	 @param cosX Normal vector in X-direction.
	 @param sinX Normal vector in Y-direction.
	 @param i Index of point vertex.
	*/
	void	CalcProfStepSize(double &cosX, double &sinX, int i)const;

	/**
	 Convert from OpenCV's CvMat to class asm_shape
	 @param res CvMat that converted from.
	*/
	void    Mat2Point(const CvMat* res);
	
	/**
	 Convert from class asm_shape to OpenCV's CvMat.
	 @param res CvMat that converted to.
	*/
	void    Point2Mat(CvMat* res)const;

private:
	void    Transform(double c00, double c01, double c10, double c11);

private:
	CvPoint2D32f* m_vPoints;	/**< point data */
	int m_nPoints;				/**< number of points */
};

/** Class for 1d profile. */
class ASMLIB asm_profile
{
public:
	/** Null Constructor */
	asm_profile();

	/** Constructor
	 @param length Width of profile
	*/
	asm_profile(int length);
	
	/** Copy Constructor */
    asm_profile(const asm_profile &v);
	
	/* and Destructor */
	~asm_profile();

	/**
     Access profile elements.
     @param i Index of profile.
     @return  Value at the certain index.
	*/
	inline const double operator[](int i)const{  return m_vProf[i]; }
	
	/**
     Access profile elements.
     @param i Index of profile.
     @return  Value at the certain index.
	*/
	inline double& operator[](int i){  return m_vProf[i]; }
	
	/**
     Access raw ptr of profile data.
     @return  Raw ptr of profile data.
	*/
	const double* GetData()const { return m_vProf;	}
	
    /**
     Override of operator=.
    */
	asm_profile&		operator=(const asm_profile &p);
	
	/**
     Override of operator=.
    */
	asm_profile&		operator=(double value);
    
	/**
     Override of operator+.
    */
	const asm_profile   operator+(const asm_profile &p)const;
    
	/**
     Override of operator+=.
    */
	asm_profile&		operator+=(const asm_profile &p);
    
	/**
     Override of operator-.
    */
	const asm_profile   operator-(const asm_profile &p)const;
    
	/**
     Override of operator-=.
    */
	asm_profile&		operator-=(const asm_profile &p);
	
	/**
     Override of operator*.
    */
	const asm_profile   operator*(double value)const;
    
	/**
     Override of operator*=.
    */
	asm_profile&		operator*=(double value);
	
	/**
     Override of operator/.
    */
	const asm_profile   operator/(double value)const;
    
	/**
     Override of operator/=.
    */
	asm_profile&		operator/=(double value);

	/**
     Release memory.
    */
    void    Clear();

	/**
     Allocate memory.
	 @param length Width of profile.
    */
    void    Resize(int length);
	
	/**
     Write profile data into file stream.
	 @param f  stream to write to.
    */
	void	Write(FILE* f);
	
	/**
     Read profile data from file stream.
	 @param f  stream to read from.
    */
	void	Read(FILE* f);

	/**
     Get the width of profile.
	*/
	inline const int  NLength()const{ return m_nWidth; }
	
	/**
     Pre-Calculate profile of all possible locations at one certain point vertex.
	 Note: Use this before calling GetOneLandmarkProf().
	 @param image  Image resource.
	 @param Shape  Shape Point information.
	 @param iPoint Index of point vertex.
	 @param width  Width of profile.
	 @param usinginterpolate Will sample pixel by bilinear interpolate or not?
	 @param cosX Normal vector in X-direction.
	 @param sinX Normal vector in Y-direction.
    */
	static void PrepareProf(const IplImage* image, const asm_shape& Shape, 
		int iPoint, int width, bool usinginterpolate, 
		double* cosX = NULL, double* sinX = NULL);
	
	/**
     Get the profile for one certain point vertex at the 
	 @param image  Image resource.
	 @param Shape  Shape Point information.
	 @param iPoint Index of point vertex.
	 @param offset Offset bias from the point Shape(iPoint).
    */
	void GetOneLandmarkProf(const IplImage* image, const asm_shape &Shape,
		int iPoint, int offset = 0);

	/**
     Normalize the profile so that its 1-norm is 1. 
	*/
	void Normalize();

	/**
	 Convert from OpenCV's CvMat to class asm_profile.
	 @param res CvMat that converted from.
	*/
	void Mat2Prof(const CvMat* res);
	
	/**
	 Convert from class asm_profile to OpenCV's CvMat.
	 @param res CvMat that converted to.
	*/
	void Prof2Mat(CvMat* res)const;

private:
	double *m_vProf; /**< profile data */
	int m_nWidth;    /**< profile width */

};

/** Class for handling camera or avi */
class ASMLIB asm_cam_or_avi
{
public:
	
	/** Null Constructor */
	asm_cam_or_avi();
	
	/** Destructor */
	~asm_cam_or_avi();

	/**
	 Open an AVI file.
	 @param videofile Filename the video file located in.
	 @return false on failure, true otherwise.
	*/
	bool Open(const char* videofile);
	
	/**
	 Capture from live camera.
	*/
	void CaptureCamera();

	/**
	 Close camera or avi [i.e. Release memory].
	*/
	void Close();

	/**
	 Get one certain frame of video/camera.
	 @param frame_no [When frame_no = -1 read from camera, and when frame_no >= 0, read i-th frame.]
	 @return Internal IplImage ptr.
	*/
	IplImage* ReadFrame(int frame_no = -2);
	
	/**
	 Get frame count of video. (It won't work for live camera).
	*/
	inline const int FrameCount()const
	{return (int)cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_COUNT);}

	/**
	 Get type of class asm_cam_or_avi (0 for avi, and 1 for cam).
	*/
	inline const int Type()const{ return avi_or_cam;	}

private:
	IplImage* capimg;		/**< Original image captured from video */
	IplImage *image;		/**< Flipped Image captured from video (when nessary) */
	CvCapture* capture;		/**< Used for function cvQueryFrame */
	int avi_or_cam;			/**< 0:avi, 1:cam */

};

/** Left and Right index in x-direction of shape */
typedef struct scale_param
{
	int left;	/**< Index of points in x-direction which has the minimum x */
	int right;	/**< Index of points in x-direction which has the maximum x */
}scale_param;

/** Class for active shape model. */
class ASMLIB asm_model
{
public:
	/**
	 Constructor
	*/
	asm_model();
	
	/**
	 Destructor
	*/
	~asm_model();

	
	/**
	 Build active shape model.
	 @param imagelists Lists of image files.
	 @param n_images Number of image files.
	 @param shapedatas Lists of shape point data.
	 @param n_shapes Number of shape data.
	 @param interpolate Will sample pixel by bilinear interpolate or not?
	 @param halfwidth  Half-side width of profile.
	 @param percentage Fraction of shape variation to retain during PCA.
	 @param level_no Pyramid level.
	 @return false on failure, true otherwise.
	*/
	bool  Build(const char** imagelists,  int n_images,
		 const asm_shape* shapedatas, int n_shapes,
		 bool interpolate, int halfwidth,  double percentage, int level_no);

	/**
	 Image alignment/fitting with an initial shape.
	 @param Shape Data that carries initial shape and also restores result after fitting.
	 @param grayimage Gray image resource.
	 @param max_iter Number of iteration.
	 @param param Left and right index for x-direction in the shape (Always set null).
	 @return Cost of fitting.
	*/
	double Fit(asm_shape& Shape, const IplImage *grayimage, 
		int max_iter = 20, const scale_param* param = NULL);	
	
	/**
     Write model data to file stream.
	 @param f  stream to write to.
    */
	void WriteModel(FILE* f);
	
	/**
     Read model data from file stream.
	 @param f  stream to read from.
    */
	void ReadModel(FILE* f);

	/**
	 Get mean shape of model.
	*/
	const asm_shape& GetMeanShape()const { return m__meanshape;	}
	
	/**
	 Get threshold of fitting (Will be calculated during building).
	*/
	const double GetThreshold()const { return m_dThresholdCost;	}
	
	/**
	 Get modes of shape distribution model (Will be calculated in shape's PCA)
	*/
	const int GetModesOfModel()const { return m_nModes;}
	
	/**
	 Get the width of mean shape [Identical to m__meanshape.GetWidth()].
	*/
	const double GetReferenceWidthOfFace()const { return m_dReferenceFaceWidth; }

private:

	/**
     Get the optimal offset at one certain point vertex during the process of best profile matching.
	 @param DeltaX Normal vector in X-direction.
	 @param DeltaY Normal vector in Y-direction.
	 @param iPoint Index of point vertex.
	 @param Shape  shape data.
	 @param nMaxOffset Maximum profile sampling size.
	 @param iLev Certain pyramid level.
	 @param image Image resource
	 @return Offset bias from Shape(iPoint)
	*/
	int FindOnePointBestOffset(double& DeltaX, double& DeltaY,
		int iPoint, const asm_shape& Shape,
		int nMaxOffset, int iLev, const IplImage* image);

	/**
     Update shape by matching the image profile to the model profile.
	 @param Shape2  Updated shape .
	 @param Shape  Shape that will be matched.
	 @param nMaxOffset Maximum profile sampling size.
	 @param iLev Certain pyramid level.
	 @param image Image resource
	 @return How many point vertex [less than Shape.NPoints()] will be updated.
	*/
	int MatchWholeProfile(asm_shape& Shape2, const asm_shape& Shape, 
		int nMaxOffset, int iLev, const IplImage* image);

	/**
     Calculate shape parameters and pose parameters.
	 @param p  Shape parameters.
	 @param a  s*cos(theta) in form of similarity transform.
	 @param b  s*sin(theta) in form of similarity transform.
	 @param tx  Tx in form of similarity transform.
	 @param ty  Ty in form of similarity transform.
	 @param Shape  Shape data.
	 @param iter_no Number of iteration.
	*/
	void CalcParams(CvMat* p, double& a, double& b,	double& tx, double& ty, 
		const asm_shape& Shape, int iter_no = 2);
	
	/**
     Constrain the shape parameters.
	 @param p  Shape parameters.
	*/
	void Clamp(CvMat* p);

	/**
     Generate shape instance according to shape parameters p and pose parameters.
	 @param Shape  Shape data.
	 @param p  Shape parameters.
	 @param a  s*cos(theta) in form of similarity transform.
	 @param b  s*sin(theta) in form of similarity transform.
	 @param tx  Tx in form of similarity transform.
	 @param ty  Ty in form of similarity transform.
	 */
	void CalcGlobalShape(asm_shape& Shape, const CvMat* p, double a, double b, double tx, double ty);

	/**
     Pyramid fitting at one certain level.
	 @param Shape  Shape data.
	 @param image Image resource.
	 @param iLev Certain pyramid level.
	 @param iter_no Number of iteration.
	*/
	void PyramidFit(asm_shape& Shape, const IplImage* image, int iLev, int iter_no);

	/**
     Align all training shape data.
	 @param Shapes  All Shape datas.
	 @param n_shapes Number of shape data.
	*/
	void AlignAllShapes(asm_shape* Shapes, int n_shapes);
    
	/**
     Calculate mean shape of all shapes.
	 @param meanShape Mean shape of all shapes.
	 @param Shapes  All shape datas.
	 @param n_shapes Number of shape data.
	*/
	void  CalcMeanShape(asm_shape &meanShape, const asm_shape* Shapes, int n_shapes); 

	/**
     Do PCA on shape data.
	 @param Shapes  All shape datas.
	 @param n_shapes Number of shape data.
	 @param percentage Fraction of shape variation to retain during PCA.
	*/
	void DoPCA(asm_shape* Shapes, int n_shapes, double percentage);

	/**
     Calculate profiles for all landmarks, all samples and all pyramid levels.
	 @param profiles  All profile datas.
	 @param n_samples Number of samples.
	 @param n_points Number of shape point.
	 @param n_level Number of pyramid level.
	 @param imagelists Lists of image files.
	 @param Shapes  All shape datas.
	 @param interpolate Will sample pixel by bilinear interpolate or not?
	*/
	void CalcProfiles(asm_profile***profiles, int n_samples, int n_points, int n_level,
		const char* imagelists[],  const asm_shape* Shapes,
		bool interpolate);

	/**
     Calculate mean & covariance matrix.
	 @param profiles  All profile datas.
	 @param n_samples Number of samples.
	 @param n_points Number of shape point.
	 @param n_level Number of pyramid level.
	*/
	void CalcStatProfiles(asm_profile***profiles, int n_samples, 
		int n_points, int n_level);

private:

	CvMat*  m_M;   /**< mean vector of shape data */
    CvMat*  m_B;   /**< eigenvetors of shape data */
    CvMat*  m_V;   /**< eigenvalues of shape data */

	CvMat* m_SM;   /**< mean of shapes projected space */	
	CvMat* m_SSD;  /**< standard deviation of shapes projected space	*/	

	CvMat*** m_P; /**< mean of profile data*/
	asm_profile** m__meanprofile;	/**< mean of profile data*/
	
	CvMat*** m_G; /**< inverted covariance matrix of profile data*/
	
	asm_shape m__meanshape;	 /**< mean shape of aligned shapes */
	
	int m_nPoints;				/**< number of shape points */
	int m_nWidth;				/**< width of each landmark's profile */
	int m_nLevel;					/**< pyramid level of multi-resolution */
	int m_nModes;					/**< number of truncated eigenvalues */
	double m_dReferenceFaceWidth;	/**< Reference FaceWidth*/
	bool m_fInterpolate;			/**< whether to using image interpolate or not*/
	double m_dThresholdCost;		/**< Threshold to determine whether fitting succeed or not*/

private:
	CvMat*		m_CBackproject; /**< Cached variables for speed up */
	CvMat*		m_CBs;			/**< Cached variables for speed up */
	double*		m_dist;			/**< Cached variables for speed up */
	asm_profile m_profile;		/**< Cached variables for speed up */
	asm_shape	m_searchShape;	/**< Cached variables for speed up */
	asm_shape	m_tempShape;	/**< Cached variables for speed up */
};

/** Class for Viola and Jones's AdaBoost Haar-like Face Detector */
class ASMLIB asm_vjfacedetect  
{
public:
	/**
	 Constructor
	*/
	asm_vjfacedetect();
	
	/**
	 Destructor
	*/
	~asm_vjfacedetect();

	/**
	 Detect all human face from image.
	 @param Shapes Returned face detected box which stores the Top-Left and Bottom-Right points, so its NPoints() = 2 here).
	 @param n_shapes Numbers of faces to return.
	 @param image Image resource.
	 @return false on no face exists in image, true otherwise.
	*/
	bool DetectFace(asm_shape** Shapes, int& n_shapes, const IplImage* image);
	
	/**
	 Release the shape resource allocated by DetectFace().
	*/
	static void FreeResultSource(asm_shape** Shapes);

	/**
	 Detect only one face from image, and this human face is located as close as to the center of image.
	 @param Shape Returned face detected box which stores the Top-Left and Bottom-Right points, so its NPoints() = 2 here).
	 @param image Image resource.
	 @return false on no face exists in image, true otherwise.
	*/
	bool DetectCentralFace(asm_shape &Shape, const IplImage* image);

	/**
	 Load adaboost cascade file for detect face.
	 @param cascade_name Filename the cascade detector located in.
	*/
	void LoadCascade(const char* cascade_name = "haarcascade_frontalface_alt2.xml");

private:
	CvMemStorage* __storage; 			/**< Memory storage */
	CvHaarClassifierCascade* __cascade;	/**< Haar cascade classifier */
};

#ifdef __cplusplus
extern "C"{
#endif

/**
 @param X x-coordinate of source object.
 @param Offset Length from the source object to the target object.
 @param cosX Value of cosine angle between the horizontal line and source-target line.
 @return x-coordinate of target object.
*/
ASMLIB inline double GetX(double X, int Offset, double cosX)
{	return X + (Offset * cosX); }

/**
 @param Y y-coordinate of source object.
 @param Offset Length from the source object to the target object.
 @param sinX Value of sine angle between the horizontal line and source-target line.
 @return y-coordinate of target object.
*/
ASMLIB inline double GetY(double Y, int Offset, double sinX)
{	return Y + (Offset * sinX); }

/**
 Write CvMat data to file stream.
 @param f  stream to write to.
 @param mat  CvMat that will be wrote.
*/
ASMLIB void WriteCvMat(FILE* f, const CvMat* mat);

/**
 Read CvMat data from file stream.
 @param f  stream to read from.
 @param mat  CvMat that will be read.
*/
ASMLIB void ReadCvMat(FILE* f, CvMat* mat);	

/**
 Image pixel at the location (x,y) using bilinear interpolate.
  @param image Image resource.
  @param x Grid value in x-direction.
  @param y Grid value in y-direction.
  @param width  Width of image.
  @param height Height of image.
  @return Pixel value.
*/
ASMLIB double GetBilinearPixel(const IplImage *image, double x, double y, 
			  int width, int height);

/**
 Image pixel at the location (x,y) using no interpolation.
  @param image Image resource.
  @param x Grid value in x-direction.
  @param y Grid value in y-direction.
  @param width  Width of image.
  @param height Height of image.
  @return Pixel value.
*/
ASMLIB int GetOriPixel(const IplImage *image, double x, double y, 
			  int width, int height);

/**
 Calculate Mahalanobis distance.
 @param covmat Covariance matrix.
 @param p Vector used to calculate distance.
 @return Mahalanobis distance.
*/
ASMLIB double CalcMahalanobisDist(const CvMat* covmat, const double* p);

/**
 Initialize shape from the detected box.
 @param Shape Returned initial shape.
 @param DetShapeBox VJ detected box calling by asm_vjfacedetect::Detect().
 @param refShape Reference average mean shape.
 @param refwidth Width of reference average mean shape.
*/
ASMLIB void InitShapeFromDetBox(asm_shape &Shape, const asm_shape& DetShapeBox, 
		const asm_shape &refShape, double refwidth);

/**
 Draw the fitting shape points onto the image.
 @param image Image resource.
 @param Shape Shape data.
*/
ASMLIB void DrawPoints(IplImage* image, const asm_shape& Shape);
	
/**
 Draw the fitting shape edge  onto the image.
 @param image Image resource.
 @param Shape Shape data.
 @param edge_start Starting index of edges.
 @param edge_end Ending index of edges.
 @param n_edges Number of edges.
*/
ASMLIB void DrawEdges(IplImage* image, const asm_shape& Shape,
		int* edge_start, int* edge_end, int n_edges);

/**
 Read the whole shape datas from the file lists.
 @param Shapes All shape datas.
 @param n_shapes Number of shape data.
 @param shapelists Lists of shape point files.
 @param imagelists Lists of image files.
 @return false on failure, true otherwise.
*/
ASMLIB bool ReadAllShapes(asm_shape* Shapes, int n_shapes,
			const char** shapelists, const char** imagelists);

#ifdef __cplusplus
}
#endif

#endif  // _ASM_LIBRARY_H_

