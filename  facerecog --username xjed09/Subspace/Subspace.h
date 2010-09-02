/*
	计算子空间和投影。目前使用的是Fisherface方法
	使用方法：
		Train 或 LoadDataFromFile -> Project -> CalcVectorDist
	特征向量都应是列向量。
*/

#pragma once

#include "../tools/tools.h"
#include "dggev.h"

#define CV_COEF_FC1 CV_64FC1 // 投影矩阵的精度，depth = 32 or 64, 64 seems to be better


class DLLEXP CSubspace
{
public:
	CSubspace(void);
	 ~CSubspace(void);

	/*
		对inputs计算主分量，结果存入 mu_total 和 W_pcaT。
		inputs:	特征向量按列排列成的矩阵
		postPcaDimCoef:	如果是整数，则直接指示PCA后的维度；
						如果是0~1的小数，则按特征值所占比例保留子空间维度（目前使用.97）
	*/
	void Pca(CvMat *inputs, double postPcaDimCoef);


	/*
		inputs:		类似Pca，但每行均值应是0(即应事先减去均值向量)，inputs会被改变。
		trainIds:	训练样本的类别ID，数组长度应与inputs的列数相同。
		postLdaDim:	LDA后的维度。如果取0，则自动选为 类别数-1；
					（见P. N. Belhumeur，Eigenfaces vs. fisherfaces: recognition using 
					class specific linear projection）
					此时如果 类别数-1 < 3，为防止维数过小，维数取为 min(训练样本数, 3)。

		Sw,Sb与Matlab基本一致，clapack计算的广义特征值、向量，可能与Matlab有区别。
		调用CalcSwSb和CalcLdaSpace，训练结果保存到 W_fldT
	*/
	void FisherLda(CvMat *inputs, int *trainIds, int postLdaDim = 0);

	// 调用 Pca 和 FisherLda，参数类似 FisherLda，训练结果保存到 W_pcafldT
	void Train(CvMat *inputs, int *trainIds);


	// inputs 可以是一个向量或列向量排列成的矩阵
	void Project(CvMat *inputs, CvMat *results);


	// 返回子空间维度
	int GetSubspaceDim();


	// 如果特征维数大于1，采用余弦距离的相反数（距离越小越相似）；否则采用欧氏距离。
	double CalcVectorDist(CvMat *target, CvMat *query);


	// 以字符串和二进制混合的方式，导入/导出：矩阵精度，原始特征维度，降维后维度，mu_total，W_pcafldT
	bool WriteDataToFile(ofstream &os);
	bool ReadDataFromFile(ifstream &is);


	void Release();

private:

	// 计算类内和类间散布矩阵
	void CalcSwSb(CvMat *inputs, int *trainIds, CvMat *Sw, CvMat *Sb);

	// 计算Fisherface子空间
	void CalcLdaSpace(CvMat *Sw64, CvMat *Sb64, int postLdaDim);

public:

	CvMat	*mu_total;
	CvMat	*W_pcaT, *W_fldT, *W_pcafldT;
	int		inputDim, postPcaDim, postLdaDim;
	int		trainNum, classNum; // 训练时的样本数和类别数
	bool	m_bTrained;
};
