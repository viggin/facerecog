#define DLLSRC

#include "FaceMngr_complete.h"

#ifdef COMPILE_MNGR_COMPLETE


CFaceMngr::CFaceMngr(void)
{
	tfaceImg8 = tfaceImg32 = tfeature = tmodel = NULL;
	align = NULL;
	light = NULL;
	feature = NULL;
	ss = NULL;
}

CFaceMngr::~CFaceMngr(void)
{
	cvReleaseMat(&tfaceImg8);
	cvReleaseMat(&tfaceImg32);
	cvReleaseMat(&tfeature);

	delete align;
	delete light;
	delete feature;
	delete ss;

	ClearList();
}

bool CFaceMngr::Init( CvSize faceSz /*= cvSize(0,0)*/ )
{
	align = new CFaceAlign;
	if (! align->Init(faceSz)) return false;
	m_faceSz = align->m_faceSz;

	light = new CLightPrep;
	if (! light->Init(m_faceSz, true)) return false;

	feature = new CFaceFeature;
	if (! (m_featureSz = feature->Init(m_faceSz, light->m_mask))) return false;

	ss = new CSubspace;

	tfaceImg8 = cvCreateMat(m_faceSz.height, m_faceSz.width, CV_8UC1);
	tfaceImg32 = cvCreateMat(m_faceSz.height, m_faceSz.width, CV_32FC1);
	tfeature = cvCreateMat(m_featureSz, 1, CV_FT_FC1);

	stamp = 0;

	return true;
}

bool CFaceMngr::Pic2NormFace( CvArr *pic, CvMat *faceImg8, DWORD flag /*= FM_DO_NORM*/, 
							 CvPoint2D32f *leftEye /*= NULL*/, CvPoint2D32f *rightEye /*= NULL*/ )
{
	IplImage *tpic8 = NULL;
	bool isrgb = false;
	if (! CV_IS_GRAY(pic))
	{
		isrgb = true;
		CvSize picSz = cvGetSize(pic);
		tpic8 = cvCreateImage(picSz, 8, 1);
		cvCvtColor(pic, tpic8, CV_BGR2GRAY);
	}
	else tpic8 = (IplImage *)pic;

	if (flag & FM_DO_FACE_ALIGN)
	{
#ifdef COMPILE_ALIGN_COORD
		if ( ! align->GetFace(tpic8, faceImg8, leftEye, rightEye) ) // 当使用FaceAlign_Coord时
#else
		if ( ! align->GetFace(tpic8, faceImg8, flag & FM_ALIGN_USE_BUF) ) // 没有给定的双眼坐标时
#endif
		{
			if(isrgb) cvReleaseImage(&tpic8);
			return false;
		}
	}
	else cvResize(tpic8, faceImg8, CV_INTER_NN); // if flag & FM_DO_FACE_ALIGN

	if (flag & FM_DO_LIGHT_PREP)
		light->RunLightPrep(faceImg8);
		
	if (isrgb) cvReleaseImage(&tpic8);
	return true;
}

bool CFaceMngr::NormFace2Model( CvArr *faceImg8, CvMat *model )
{
	if (! ss->m_bTrained)
	{
		::MessageBox1("Can't generate model: training hasn't done!");
		return false;
	}
	cvConvertScale(faceImg8, tfaceImg32, 1.0/255);
	feature->GetFeature(tfaceImg32, tfeature);
	ss->Project(tfeature, model);
	return true;
}

void CFaceMngr::SaveToModel( LPCTSTR path, int classId, CvMat *model )
{
	SModel sm;
	sm.model = model;
	sm.classId = classId;
	sm.picPath = path;

	m_lstModel.push_back(sm);
}

bool CFaceMngr::SavePicToModel( LPCTSTR strPath, int classId, DWORD flag )
{
	CString path = strPath;
	IplImage *pic = cvLoadImage(path, CV_LOAD_IMAGE_GRAYSCALE);
	if (!pic) return false;
	if (! Pic2NormFace(pic, tfaceImg8, flag))
	{
		cvReleaseImage(&pic); // 没找到脸
		return false;
	}

	CvMat *model = cvCreateMat(m_modelSz, 1, CV_MODEL_FC1);
	cvReleaseImage(&pic);

	if (! NormFace2Model(tfaceImg8, model))	
	{
		cvReleaseMat(&model);
		return false;
	}
	SaveToModel(path, classId, model);
	if (flag & FM_SAVE_NORM_FACE)
	{
		int p = path.ReverseFind('\\');
		CString fd = path.Left(p+1), fn = path.Right(path.GetLength() - p - 1);
		::CreateDirectory(fd + FACE_REL_PATH, NULL);
		cvSaveImage(fd + FACE_REL_PATH + fn, tfaceImg8);
	}

	return true;
}

void CFaceMngr::ClearList()
{
	smd_iter	iter = m_lstModel.begin();
	for (; iter != m_lstModel.end(); iter++)
	{
		cvReleaseMat(&(iter->model));
	}
	m_lstModel.clear();
}

int CFaceMngr::GetModelCount()
{
	return m_lstModel.size();
}

bool CFaceMngr::Train( LPCTSTR rootPath, vector<SFInfo> &paths, DWORD flag )
{
	/* 初始化 */

	ClearList(); // 既然重新训练，肯定要清除以前的Model
	m_rtPath = rootPath;
	m_rtPath.TrimRight('\\');
	m_rtPath += '\\';
	if (flag & FM_SAVE_NORM_FACE) ::CreateDirectory(m_rtPath + FACE_REL_PATH, NULL);

	/* 读入图像并转化为原始特征 */

	//if (flag & FM_SHOW_DETAIL) cout<<"Computing Feature..."<<endl;
	sfi_iter iter = paths.begin();
	for (; iter != paths.end(); iter++)
	{
		CString path = m_rtPath + (iter->picPath).TrimLeft('\\');
		IplImage *pic = cvLoadImage(path, CV_LOAD_IMAGE_GRAYSCALE);
		if (!pic) continue; // 不是图片文件

		if (! Pic2NormFace(pic, tfaceImg8, flag, &(iter->leye), &(iter->reye))) 
		{
			cvReleaseImage(&pic);
			continue; // 没找到脸
		}
		if (flag & FM_SAVE_NORM_FACE)
			cvSaveImage(m_rtPath + FACE_REL_PATH + iter->picPath, tfaceImg8);

		cvConvertScale(tfaceImg8, tfaceImg32, 1.0/255);
		CvMat *ft = cvCreateMat(m_featureSz, 1, CV_FT_FC1);
		feature->GetFeature(tfaceImg32, ft);
		if (flag & FM_SAVE_REL_PATH) path = iter->picPath;
		SaveToModel(path, iter->classId, ft);

		cvReleaseImage(&pic);
	}
	if (GetModelCount() <= 1) 
	{
		CString msg;
		msg.Format("Too few \"%s faces\" were found.", (flag & FM_DO_FACE_ALIGN) ? 
			"aligned" : "pictures that contain");
		::MessageBox1(msg);
		ClearList();
		return false;
	}

	/* 训练 */

	//if (flag & FM_SHOW_DETAIL) cout<<"Computing Subspace..."<<endl;
	m_trainNum = GetModelCount();
	CvMat	*inputs = cvCreateMat(m_featureSz, m_trainNum, CV_FT_FC1);
	int		*trainIds = new int[m_trainNum];
	FormTrainMat(inputs, trainIds);
	ss->Train(inputs, trainIds);
	m_modelSz = ss->GetSubspaceDim();
	m_trainclsNum = ss->classNum;

	if (flag & FM_TRAIN_SAVE2MODEL) TrainResSave2Model();
	/* 清理 */
	else ClearList();

	delete []trainIds;
	cvReleaseMat(&inputs);
	tmodel = cvCreateMat(m_modelSz, 1, CV_MODEL_FC1);
	stamp = cvGetTickCount()%100000000;

	return true;
}

bool CFaceMngr::HasTrained()
{
	return ss->m_bTrained;
}

void CFaceMngr::FormTrainMat( CvMat *inputs, int *trainIds )
{
	smd_iter	iter = m_lstModel.begin();
	CvMat	sub, *src;
	int i = 0;

	for (; iter != m_lstModel.end(); iter++)
	{
		src = iter->model;
		cvGetCol(inputs, &sub, i);
		cvCopy(src, &sub);
		trainIds[i++] = iter->classId;
	}
}

void CFaceMngr::TrainResSave2Model()
{
	int		mdSz = ss->GetSubspaceDim();
	smd_iter	iter = m_lstModel.begin();
	for (; iter != m_lstModel.end(); iter++)
	{
		// 将训练时存入模板库中的原始特征投影、更换
		CvMat *model = cvCreateMat(mdSz, 1, CV_MODEL_FC1);
		ss->Project(iter->model, model);
		cvReleaseMat(&(iter->model));
		iter->model = model;
	}
}

bool CFaceMngr::ModelRecognize( CvMat *model, SMatch *info )
{
	if (GetModelCount() == 0)
	{
		::MessageBox1("There are no models in the memory! Please enroll first.");
		return false;
	}

	SModel	*minpm;
	smd_iter	iter = m_lstModel.begin();
	double	minDist = 1e9, curVal; // minDist should be among -1~1 for angle metric
	for (; iter != m_lstModel.end(); iter++)
	{
		curVal = ss->CalcVectorDist(iter->model, model);
		if (curVal < minDist)
		{
			minDist = curVal;
			minpm = &(*iter);
		}
	}

	info->classId = minpm->classId;
	info->dist = minDist;
	info->picPath = minpm->picPath;
	return true;
}

bool CFaceMngr::PicRecognize( CvArr *pic, DWORD flag, SMatch *info )
{
	if (GetModelCount() == 0)
	{
		::MessageBox1("There are no models in the memory! Please enroll first.");
		return false;
	}

	if (! Pic2NormFace(pic, tfaceImg8, flag))
	{
		::MessageBox1("Failed processing face detection or alignment.");
		return false;
	}

	if ( NormFace2Model(tfaceImg8, tmodel) &&
			ModelRecognize(tmodel, info)		)
	{
		return true;
	}
	return false;
}

double CFaceMngr::BatchPicRecog( LPCTSTR rootPath, vector<SFInfo> &testList, vector<SMatch> &resList, DWORD flag )
{
	sfi_iter testIter = testList.begin();
	SMatch info;
	int correctNum = 0;
	for (; testIter != testList.end(); testIter++)
	{
		CString path = rootPath;
		path += '\\';
		path += testIter->picPath;
		IplImage *img = cvLoadImage(path, CV_LOAD_IMAGE_GRAYSCALE);
		if (! PicRecognize(img, flag, &info))
			info.classId = FM_RECOG_NOT_DONE;
		resList.push_back(info);
		if (info.classId == testIter->classId) correctNum++;
	}
	return correctNum/testList.size();
}

bool CFaceMngr::WriteMatToFile( ofstream &os )
{
	if (!HasTrained())
	{
		::MessageBox1("You haven't trained yet!");
		return false;
	}
	WriteIntText(os, stamp, "stamp:");
	ss->WriteDataToFile(os);
	return true;
}

bool CFaceMngr::WriteModelToFile( ofstream &os )
{
	if (GetModelCount() == 0)
	{
		::MessageBox1("There are no models in the memory!.");
		return false;
	}

	WriteIntText(os, stamp, "stamp:");

	int dataBytes = CV_MODEL_FC1 == CV_32FC1 ? 4:8; // 确保存储时的数据精度和程序当前设置的一致
	WriteIntText(os, dataBytes, "dataBytes:");
	WriteIntText(os, GetModelCount(), "modelNum:");
	WriteIntText(os, m_modelSz, "modelSz:");
	WriteStringLine(os, "data:");

	smd_iter	iter = m_lstModel.begin();
	for (; iter != m_lstModel.end(); iter++)
	{
		WriteIntText(os, iter->classId);
		WriteCvMatBin(os,iter->model);
		WriteStringLine(os, iter->picPath+"\n");
	}
	return true;
}

bool CFaceMngr::ReadMatFromFile( ifstream &is )
{
	int new_sig;
	ReadIntText(is, new_sig);
	if (new_sig != stamp) 
	{
		ClearList();
		stamp = new_sig;
	}

	bool ret = ss->ReadDataFromFile(is);
	if (ss->inputDim != m_featureSz)
	{
		ss->Release();
		MessageBox1("Feature size not match!");
		return false;
	}
	m_modelSz = ss->GetSubspaceDim();
	cvReleaseMat(&tmodel);
	tmodel = cvCreateMat(m_modelSz, 1, CV_MODEL_FC1);
	return ret;
}

int CFaceMngr::ReadModelFromFile( ifstream &is )
{
	int new_sig;
	ReadIntText(is, new_sig);

	int dataBytes, modelNum;
	ReadIntText(is, dataBytes);
	if ((dataBytes == 4 && CV_MODEL_FC1 == CV_64FC1) ||
		(dataBytes == 8 && CV_MODEL_FC1 == CV_32FC1))
	{
		::MessageBox1("float/double not match!");
		return 0;
	}

	if (new_sig != stamp) // 如果相同，就append models，否则新建模板库，清空训练结果(需要重新训练)
	{
		ClearList();
		ss->Release();
		stamp = new_sig;
	}

	ReadIntText(is, modelNum);
	ReadIntText(is, m_modelSz);

	CString tmp;
	for (int i = 0; i < modelNum; i++)
	{
		SModel sm;
		ReadStringLine(is, tmp);
		ReadIntText(is, sm.classId, false);
		sm.model = cvCreateMat(m_modelSz, 1, CV_MODEL_FC1);
		ReadCvMatBin(is, sm.model, false);
		ReadStringLine(is, sm.picPath);
		m_lstModel.push_back(sm);
	}

	return modelNum;
}

#endif