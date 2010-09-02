/****************************************************************************
*						ASMLibrary
*			http://code.google.com/p/asmlibrary
* Copyright (c) 2008 by Yao Wei, all rights reserved.
* Author:      	Yao Wei
* Contact:     	njustyw@gmail.com
****************************************************************************/

#ifndef _ASM_FITTING_H_
#define _ASM_FITTING_H_

#include "asmlibrary.h"

/** Wrapped Class for face alignment/tracking using active shape model */
class ASMLIB asmfitting
{
public:
	/** Constructor */
	asmfitting();

	/** Destructor */
	~asmfitting();
	
	/**
     Process face alignment on image. (Only one face box)
	 @param Shape Data that carries initial shape and also restores result after fitting.
	 @param image Image resource.
	*/
	void Fitting(asm_shape& Shape, const IplImage* image);
	
	/**
     Process face alignment on image. (Multi-face box)
	 @param Shapes All shape datas.
	 @param n_shapes Number of human face.
	 @param image Image resource.
	*/
	void Fitting2(asm_shape* Shapes, int n_shapes, const IplImage* image);
	
	/**
     Process face tracking on video/camera.
	 @param Shape Data that carries initial shape and also restores result after fitting.
	 @param image Image resource.
	 @param frame_no Certain frame number of video/camera.
	 @param useopticalflow Whether to use optical flow or not?
	 @return false on failure, true otherwise.
	*/
	bool ASMSeqSearch(asm_shape& Shape, const IplImage* image, 
		int frame_no = 0, bool useopticalflow = false);
	
	/**<
     Get the Average Viola-Jone Box.
	*/
	const asm_shape GetMappingDetShape()const { return m__VJdetavshape;}
	
	/**<
     Get the width of mean face.
	*/
	const double	GetMeanFaceWidth()const{ return m_model.GetMeanShape().GetWidth();	}
	
	/**<
	 Get raw ptr of asm_model.
	*/
	const asm_model* GetModel()const { return &m_model; }
	
	/**
     Read model data from file.
	 @param filename  Filename the model located in.
	 @return false on failure, true otherwise.
    */
	bool Read(const char* filename);

	/**
     Draw point and edge on the image.
	 @param image  Image resource.
	 @param Shape  Shape data of fitting result.
	*/
	void Draw(IplImage* image, const asm_shape &Shape);

private:

	/**
     Apply optical flow between two successive frames.
	 @param Shape Data that carries initial shape and also restores result after fitting.
	 @param grayimage  Image resource.
	*/
	void OpticalFlowAlign(asm_shape& Shape, const IplImage* grayimage);

private:
	asm_model	m_model;	/**<active shape model to be trained */
	int *m_edge_start; /**< Starting index of edges */
	int *m_edge_end;   /**< Ending index of edges */
	int m_nEdge;       /**< Number of edges */
	asm_shape m__VJdetavshape;    /**< average mapping shape relative to VJ detect box*/
	scale_param m_param;			/**< point index of left and right side in the face template*/
	bool m_flag;					/**< Does the image contain face? */
	double m_dReferenceFaceWidth;	/**< reference face width */

private:
	IplImage* __lastframe;  /**< Cached variables for optical flow */
	IplImage* __pyrimg1;	/**< Cached variables for optical flow */
	IplImage* __pyrimg2;	/**< Cached variables for optical flow */
	CvPoint2D32f* __features1;	/**< Cached variables for optical flow */
	CvPoint2D32f* __features2;	/**< Cached variables for optical flow */
	char* __found_feature;	/**< Cached variables for optical flow */
	float* __feature_error;	/**< Cached variables for optical flow */
};

#endif //_ASM_FITTING_H_


