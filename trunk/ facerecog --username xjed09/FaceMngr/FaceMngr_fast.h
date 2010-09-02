/*
	针对人脸库测试用，减少了一些类型检测等，并且将模板用CvMat而不是vector存储，节约了一点时间。
	FaceAlign需要用COMPILE_USE_COORD.
	用法：Train->(BatchPicEnroll)->BatchPicRecog
*/

#pragma once

#define MNGR_HEADER

#include "FaceMngr.h"

#ifdef COMPILE_MNGR_FAST

#include "../FaceAlign/FaceAlign.h"
#include "../LightPrep/LightPrep.h"
#include "../FaceFeature/FaceFeature.h"
#include "../Subspace/Subspace.h"
#include "../tools/tools.h"

#pragma comment(lib, "../Subspace/dggev.lib")

struct SFInfo // Struct of File Info，训练、人脸库批量测试时用于输入文件信息的结构体
{
	int		classId;
	CString picPath;
	CvPoint2D32f leye;
	CvPoint2D32f reye;
	SFInfo(int i,CString s, CvPoint2D32f l = cvPoint2D32f(0,0), CvPoint2D32f r = cvPoint2D32f(0,0)):
		classId(i), picPath(s), leye(l), reye(r){}
};

struct SMatch // 识别后用于记录信息的结构体
{
	int		classId;
	CString	picPath; // 源图片路径
	double	dist;    // 匹配距离
};

typedef vector<SFInfo>::iterator	sfi_iter;
typedef vector<SMatch>::iterator	smt_iter;


class DLLEXP CFaceMngr
{
public:
	CFaceMngr(void);
	~CFaceMngr(void);

	// 调用CFaceAlign,CLightPrep,CFaceFeature的初始化函数，正确返回true
	bool	Init(CvSize faceSz = cvSize(0,0));


/// 注册

	// 对图像进行几何、光照归一化，成功返回true。flag定义见FaceMngr.h，例如，pic需要几何归一化，不需要光照归一化，则设flag = FM_DO_FACE_ALIGN
	bool	Pic2NormFace(CvArr *pic, CvMat *faceImg8, DWORD flag = FM_DO_NORM, CvPoint2D32f *leftEye = NULL, CvPoint2D32f *rightEye = NULL);


	// 对归一化人脸提取原始特征向量并进行降维，需要先训练，否则返回false
	bool	NormFace2Model(CvArr *faceImg8, CvMat *model);


	// 批量注册，注意后一次运行会删去前一次的注册数据。
	// flag可以是Pic2NormFace所用flag和FM_SHOW_DETAIL的组合。
	bool	BatchPicEnroll(LPCTSTR rootPath, vector<SFInfo> &enrList, DWORD flag);


/// 训练

	/*
		给定所有训练文件的根路径，和他们的子路径+类别，训练投影矩阵。
		例如，rootPath = D:\\face\\，paths = {{"John_1.bmp",1}, {"John_2.bmp",1}, {"Mike_1.bmp",5},...}
		flag可以是Pic2NormFace所用flag和FM_SAVE_NORM_FACE, FM_SHOW_DETAIL, FM_TRAIN_SAVE2MODEL的组合
		目前只从图片中检测最大的人脸用于训练。
	*/
	bool	Train(LPCTSTR rootPath, vector<SFInfo> &trainList, DWORD flag);


	// 返回是否已训练
	bool	HasTrained();


/// 识别

public:

	// 注册和训练完毕后，对输入的model进行匹配，匹配结果信息存入info
	// 暂时只返回相似度最大的，也可以改为返回前几名
	bool	ModelRecognize(CvMat *model, SMatch *info);


	// 调用 Pic2NormFace, NormFace2Model, ModelRecognize，直接对未归一化图片进行识别，结果存入matchId
	// flag与Pic2NormFace所用flag相同
	bool	PicRecognize(CvArr *pic, DWORD flag, SMatch *info, CvPoint2D32f *leye = NULL, CvPoint2D32f *reye = NULL);


	// 调用PicRecognize进行批量识别，返回正确率。
	// testList中应存有正确Id，以便计算正确率；resList是和testList等长的数组，返回matchId
	// flag可以是Pic2NormFace所用flag和FM_SHOW_DETAIL的组合。
	double	BatchPicRecog( LPCTSTR rootPath, vector<SFInfo> &testList, vector<SMatch> &resList, DWORD flag );

/// 输入/输出

	bool	WriteMatToFile( ofstream &os );
	bool	ReadMatFromFile( ifstream &is );

/// 变量

	CvMat		*tfaceImg8, *tfaceImg32, *tfeature, *tmodel, *m_models; // 中间变量

	CFaceAlign		*align;
	CLightPrep		*light;
	CFaceFeature	*feature;
	CSubspace		*ss;

	CvSize	m_faceSz;
	int		m_featureSz, m_modelSz; // 原始特征维数和降维后维度
	int		m_trainNum, m_trainclsNum; // 训练样本数和训练类别数
	int		m_enrNum; // 注册样本数
	int		*m_ids; // 模板库的类别列表

};

#endif