 /*
	调用CFaceAlign,CLightPrep,CFaceFeature,CSubspace四个类，
	实现建立人脸库、训练、识别等具体功能。
*/

#pragma once

#define MNGR_HEADER

#include "FaceMngr.h"

#ifdef COMPILE_MNGR_FAST_FAST

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
	SFInfo(int i,CString s):classId(i),picPath(s){}
};

typedef vector<SFInfo>::iterator	sfi_iter;


class DLLEXP CFaceMngr
{
public:
	CFaceMngr(void);

	~CFaceMngr(void);

	bool	Init(CvSize faceSz);

	double run_orl( LPCTSTR rp, vector<SFInfo> & trainList, vector<SFInfo> & testList, int * resultList );

private:
	int		NormFaceRecog(CvMat *faceImg32);


	CvMat		*tfaceImg8, *tfaceImg32, *tfeature, *tmodel; // 中间变量
	CvMat		*m_models;

	CFaceAlign		*align;
	CLightPrep		*light;
	CFaceFeature	*feature;
	CSubspace		*ss;

	CvSize	m_faceSz;
	int		m_featureSz, m_modelSz; // 原始特征维数和降维后维度
	int		m_trainNum, m_classNum; // 训练样本数和训练类别数

	int		*trainIds;
};

#endif