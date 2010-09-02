 /*
	调用CFaceAlign,CLightPrep,CFaceFeature,CSubspace四个类，
	实现建立人脸库、训练、识别等具体功能。
*/

#pragma once

#define MNGR_HEADER

#include "FaceMngr.h"

#ifdef COMPILE_MNGR_COMPLETE

#include "../FaceAlign/FaceAlign.h" // 不把这些include语句放到 FaceMngr.h 中的原因是，为了减少#ifdef前面的语句的编译时间
#include "../LightPrep/LightPrep.h"
#include "../FaceFeature/FaceFeature.h"
#include "../Subspace/Subspace.h"
#include "../tools/tools.h"

#pragma comment(lib, "../FaceAlign/asmlibrary.lib")
#pragma comment(lib, "../Subspace/dggev.lib")

#pragma warning(disable: 4251) // 不显示信息：class“std::vector<_Ty>”需要有 dll 接口由 class“CFaceMngr”的客户端使用

struct SModel // 模板库中用于记录信息的结构体
{
	int		classId;
	CString	picPath; // 源图片路径
	CvMat	*model;
};

struct SMatch // 识别后用于记录信息的结构体
{
	int		classId;
	CString	picPath; // 源图片路径
	double	dist;    // 匹配距离
};

struct SFInfo // Struct of File Info，训练、人脸库批量测试时用于输入文件信息的结构体
{
	int		classId;
	CString picPath;
	CvPoint2D32f leye;
	CvPoint2D32f reye;
	SFInfo(int i,CString s, CvPoint2D32f l = cvPoint2D32f(0,0), CvPoint2D32f r = cvPoint2D32f(0,0)):
		classId(i), picPath(s), leye(l), reye(r){}
};

typedef vector<SModel>::iterator	smd_iter;
typedef vector<SFInfo>::iterator	sfi_iter;
typedef vector<SMatch>::iterator	smt_iter;

#define DEF_PIC_PATH	"facePics\\" // 默认图片路径和归一化人脸存储路径
#define FACE_REL_PATH	"faces\\" // 程序中约定：路径的左侧不带'\',文件夹的右侧带'\'


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


	// 将模板信息和特征向量存入模板库。
	void	SaveToModel(LPCTSTR path, int classId, CvMat *model);


	/*
		将图片中的人脸（如果有的话）加入模板库
		strPath:	图片路径
		classId:	人脸类别序号，可以是FM_UNKNOWN_CLASS_ID
		flag:		可以是Pic2NormFace所用flag和FM_SAVE_NORM_FACE的组合
	*/
	bool	SavePicToModel(LPCTSTR strPath, int classId, DWORD flag);
	

	// 清空模板库
	void	ClearList();


	// 返回模板数量
	int		GetModelCount();


/// 训练

	/*
		给定所有训练文件的根路径，和他们的子路径+类别，训练投影矩阵。
		例如，rootPath = D:\\face\\，paths = {{"John_1.bmp",1}, {"John_2.bmp",1}, {"Mike_1.bmp",5},...}
		flag可以是Pic2NormFace所用flag和FM_SAVE_NORM_FACE, FM_SHOW_DETAIL, FM_TRAIN_SAVE2MODEL的组合
		目前只从图片中检测最大的人脸用于训练。
	*/
	bool	Train(LPCTSTR rootPath, vector<SFInfo> &paths, DWORD flag);


	// 返回是否已训练
	bool	HasTrained();
private:

	// 根据 m_lstModel 生成用于训练的矩阵 inputs 和类别ID trainIds
	void	FormTrainMat(CvMat *inputs, int *trainIds);


	// 训练后，将训练样本投影并存入模板库
	void	TrainResSave2Model();


/// 识别

public:

	// 注册和训练完毕后，对输入的model进行匹配，匹配结果信息存入info
	// 暂时只返回相似度最大的，也可以改为返回前几名
	bool	ModelRecognize(CvMat *model, SMatch *info);


	// 调用 Pic2NormFace, NormFace2Model, ModelRecognize，直接对未归一化图片进行识别，结果存入info
	// flag与Pic2NormFace所用flag相同
	bool	PicRecognize(CvArr *pic, DWORD flag, SMatch *info);


	// 对testList中的文件批量识别，结果存入resList，flag与Pic2NormFace所用flag相同
	double	BatchPicRecog( LPCTSTR rootPath, vector<SFInfo> &testList, vector<SMatch> &resList, DWORD flag );

/// 输入/输出

	bool	WriteMatToFile( ofstream &os );
	bool	WriteModelToFile( ofstream &os );
	bool	ReadMatFromFile( ifstream &is );
	int		ReadModelFromFile( ifstream &is );

/// 变量

	vector<SModel>	m_lstModel; // 模板库

	CvMat		*tfaceImg8, *tfaceImg32, *tfeature, *tmodel; // 中间变量

	CFaceAlign		*align;
	CLightPrep		*light;
	CFaceFeature	*feature;
	CSubspace		*ss;

	CString m_rtPath;
	CvSize	m_faceSz;
	int		m_featureSz, m_modelSz; // 原始特征维数和降维后维度
	int		m_trainNum, m_trainclsNum; // 训练样本数和训练类别数

	int	stamp; // 为了防止矩阵和model弄混，每次训练得到的矩阵都会有一个stamp(当时的时间)，存储到文件中。
};

#endif