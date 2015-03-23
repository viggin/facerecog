#define DLLSRC

#include "../FaceAlign/FaceAlign.h"
#include "../LightPrep/LightPrep.h"
#include "../FaceFeature/FaceFeature.h"
#include "../Subspace/Subspace.h"

#include "FaceMngr.h"
#include "../tools/AlgorithmSelect.h"
#include "PickEyeDlg.h"


//#ifdef _DEBUG
//#define new DEBUG_NEW
//#undef THIS_FILE
//static char THIS_FILE[] = __FILE__;
//#endif

bool CFaceMngr::Pic2NormFace( CvArr *pic, CvMat *faceImg8, DWORD flag /*= FM_DO_NORM*/, 
			CvPoint2D32f &leftEye /*= cvPoint2D32f(0,0)*/, CvPoint2D32f &rightEye /*= cvPoint2D32f(0,0)*/ )
{
	IplImage *tpic8 = NULL;
	bool isrgb = false;
	if (! CV_IS_GRAY(pic))
	{
		isrgb = true;
		CvSize picSz = cvGetSize(pic);
		tpic8 = cvCreateImage(picSz,IPL_DEPTH_8U,1);
		cvCvtColor(pic, tpic8, CV_BGR2GRAY);
	}
	else tpic8 = (IplImage *)pic;

	if (flag & FM_DO_FACE_ALIGN)
	{
		if ( ! f.GetFace(tpic8, faceImg8, (flag & FM_ALIGN_USE_BUF)!=0, leftEye, rightEye) ) // face detection failed
		{
			if(isrgb) cvReleaseImage(&tpic8);
			return false;
		}
	}
	else cvResize(tpic8, faceImg8, CV_INTER_LINEAR); // if FM_DO_FACE_ALIGN flag is not set

	if (flag & FM_DO_LIGHT_PREP)
		f.RunLightPrep(faceImg8);
		
	if (isrgb) cvReleaseImage(&tpic8);
	return true;
}

bool CFaceMngr::NormFace2Model( CvArr *faceImg8, CvMat *model )
{
	if (! m_bTrained)
	{
		::MessageBox1("Can't generate model: training hasn't done!");
		return false;
	}
	cvConvertScale(faceImg8, tfaceImg32, 1.0/255);
	f.GetFeature(tfaceImg32, tfeature);
	f.Project(tfeature, model);
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

		MessageBox1("The program failed to detect faces from some pictures. Please help us with it:\n"
			"Use your mouse to click on the right and left eyes in these pictures. Press ENTER to submit, ESC to undo.");

		CPickEyeDlg ped;
		if (! ped.Pick(pic))
		{
			cvReleaseImage(&pic);
			return false;
		}
		if (! Pic2NormFace(pic, tfaceImg8, flag, cvPointTo32f(ped.m_leftEye), cvPointTo32f(ped.m_rightEye)))
		{
			cvReleaseImage(&pic); // still failed
			return false;
		}
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
	/* init */

	ClearList(); // clear previous gallery

	CString loadImgRootPath = rootPath;
	CString saveImgRootPath;
	if (!loadImgRootPath.IsEmpty())
	{
		loadImgRootPath.TrimRight('\\');
		loadImgRootPath += '\\';
		saveImgRootPath = loadImgRootPath + FACE_REL_PATH;
	}
	else
	{
		saveImgRootPath = CString("D:\\") + FACE_REL_PATH;
	}
	if (flag & FM_SAVE_NORM_FACE) ::CreateDirectory(saveImgRootPath, NULL);


	/* read pictures, compute features */

	//if (flag & FM_SHOW_DETAIL) cout<<"Computing Feature..."<<endl;
	sfi_iter iter = paths.begin();
	for (; iter != paths.end(); iter++)
	{
		CString path = loadImgRootPath + (iter->picPath).TrimLeft('\\');
		IplImage *pic = cvLoadImage(path, CV_LOAD_IMAGE_GRAYSCALE);
		if (!pic) continue; // not a picture

		if (! Pic2NormFace(pic, tfaceImg8, flag)) 
		{
			cvReleaseImage(&pic);
			continue; // face detection failed
		}
		if (flag & FM_SAVE_NORM_FACE)
		{
			CString picPath = (iter->picPath);
			picPath.Replace('\\','_');
			picPath.Replace(':','_');
			picPath = picPath.Right(200);
			cvSaveImage(saveImgRootPath + picPath, tfaceImg8);
		}
		cvConvertScale(tfaceImg8, tfaceImg32, 1.0/255);
		CvMat *ft = cvCreateMat(m_featureSz, 1, CV_FT_FC1);
		f.GetFeature(tfaceImg32, ft);
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

	/* training */

	//if (flag & FM_SHOW_DETAIL) cout<<"Computing Subspace..."<<endl;
	m_trainNum = GetModelCount();
	CvMat	*inputs = cvCreateMat(m_featureSz, m_trainNum, CV_FT_FC1);
	int		*trainIds = new int[m_trainNum];
	FormTrainMat(inputs, trainIds);
	m_modelSz = f.CalcSubspace(inputs, trainIds);

	if (flag & FM_TRAIN_SAVE2MODEL) TrainResSave2Model();
	/* clear */
	else ClearList();

	delete []trainIds;
	cvReleaseMat(&inputs);
	tmodel = cvCreateMat(m_modelSz, 1, CV_MODEL_FC1);
	m_stamp = cvGetTickCount()%100000000;
	m_bTrained = true;

	return true;
}

bool CFaceMngr::HasTrained()
{
	return m_bTrained;
}

void CFaceMngr::FormTrainMat( CvMat *inputs, int *trainIds )
{
	smd_iter	iter = m_lstModel.begin();
	CvMat	sub, *src;
	int i = 0;
	m_trainclsNum = 0;

	for (; iter != m_lstModel.end(); iter++)
	{
		src = iter->model;
		cvGetCol(inputs, &sub, i);
		cvCopy(src, &sub);

		bool flag = false;
		for (int j = 0;j < i;j++)
		{
			if (trainIds[j] == iter->classId)
			{
				flag = true;
				break;
			}
		}
		if (!flag) m_trainclsNum++;
		trainIds[i++] = iter->classId;
	}
}

void CFaceMngr::TrainResSave2Model()
{
	int		mdSz = f.GetModelSize();
	smd_iter	iter = m_lstModel.begin();
	for (; iter != m_lstModel.end(); iter++)
	{
		// project the feature vectors of the training samples
		CvMat *model = cvCreateMat(mdSz, 1, CV_MODEL_FC1);
		f.Project(iter->model, model);
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

	smd_iter	iter = m_lstModel.begin();
	SModel	*minpm = &(*iter);
	double	minDist = 1e9, curVal; // minDist should be among 0~1
	for (; iter != m_lstModel.end(); iter++)
	{
		curVal = f.CalcVectorDist(iter->model, model);
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

bool CFaceMngr::ModelRecognizeMulti( CvMat *model, CvMat *matchOrder )
{
	int modelCnt = GetModelCount();
	if (modelCnt == 0)
	{
		::MessageBox1("There are no models in the memory! Please enroll first.");
		return false;
	}

	CvMat *matchScores = cvCreateMat(1, modelCnt, CV_32FC1);
	float *p = matchScores->data.fl;
	smd_iter	iter = m_lstModel.begin();
	for (; iter != m_lstModel.end(); iter++)
	{
		*p = float(f.CalcVectorDist(iter->model, model));
		p++;
	}

	cvSort(matchScores, NULL, matchOrder, CV_SORT_ASCENDING|CV_SORT_EVERY_ROW);
	return true;
}

bool CFaceMngr::PicRecognizeMulti( CvArr *pic, DWORD flag, CvMat *matchOrder, 
			CvPoint2D32f &leftEye /*= cvPoint2D32f(0,0)*/, CvPoint2D32f &rightEye /*= cvPoint2D32f(0,0)*/ )
{
	if (GetModelCount() == 0)
	{
		::MessageBox1("There are no models in the memory! Please enroll first.");
		return false;
	}

	if (! Pic2NormFace(pic, tfaceImg8, flag, leftEye, rightEye))
	{
		::MessageBox1("Failed processing face detection or alignment.");
		return false;
	}

	if ( NormFace2Model(tfaceImg8, tmodel) &&
			ModelRecognizeMulti(tmodel, matchOrder)		)
	{
		return true;
	}
	return false;
}

bool CFaceMngr::WriteMatToFile( ofstream &os )
{
	if (!HasTrained())
	{
		::MessageBox1("You haven't trained yet!");
		return false;
	}
	WriteIntText(os, m_stamp, "stamp:");
	WriteStringLine(os, "Algorithms:");
	WriteStringLine(os, m_dllList[0]);
	WriteStringLine(os, m_dllList[1]);
	WriteStringLine(os, m_dllList[2]);
	WriteStringLine(os, m_dllList[3]);
	f.WriteDataToFile(os);
	return true;
}

bool CFaceMngr::WriteModelToFile( ofstream &os )
{
	if (GetModelCount() == 0)
	{
		::MessageBox1("There are no models in the memory!.");
		return false;
	}

	WriteIntText(os, m_stamp, "stamp:");
	WriteStringLine(os, "Algorithms:");
	WriteStringLine(os, m_dllList[0]);
	WriteStringLine(os, m_dllList[1]);
	WriteStringLine(os, m_dllList[2]);
	WriteStringLine(os, m_dllList[3]);

	//int dataBytes = CV_MODEL_FC1 == CV_32FC1 ? 4:8;
	WriteIntText(os, CV_MODEL_FC1, "dataBytes:");
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
	if (new_sig != m_stamp) 
	{
		ClearList();
		m_stamp = new_sig;
	}

	CString tmp;
	ReadStringLine(is, tmp);
	if (tmp.IsEmpty())
	{
		ReadStringLine(is, tmp);
	}
	for (int i = 0; i < 4; i++)
	{
		ReadStringLine(is, tmp);
		tmp.TrimRight();
		int l = tmp.GetLength();
		if (tmp != m_dllList[i]) // tmp maybe end with \n
		{
			//if (CString(m_dllList[3]) != "Subspace_DirectMatch.dll")
			{
				char str[200];
				sprintf_s(str, 200, 
					"Algorithms are not the same!\n The program is using %s but the file to load uses %s", m_dllList[i],tmp);
				MessageBox1(str);
				return false;
			}

		}
	}
	bool ret = f.ReadDataFromFile(is);
	if (f.GetFtDim() != m_featureSz)
	{
		f.ReleaseSubspace();
		MessageBox1("Feature size not match!");
		return false;
	}
	m_modelSz = f.GetModelSize();
	cvReleaseMat(&tmodel);
	tmodel = cvCreateMat(m_modelSz, 1, CV_MODEL_FC1);
	m_bTrained = true;
	return ret;
}

int CFaceMngr::ReadModelFromFile( ifstream &is )
{
	int new_sig;
	ReadIntText(is, new_sig);

	if (new_sig != m_stamp) // if same, append to current gallery; otherwise clear the gallery and current classifier(need to re-train)
	{
		int ans = MessageBox1("the gallery to load does not match the current classifier. "
			"Click YES to clear the current gallery and classifier, NO to quit loading."
			, MB_YESNO|MB_ICONQUESTION);
		if (ans == IDNO) return 0;
		ClearList();
		f.ReleaseSubspace();
		m_stamp = new_sig;
	}

	CString tmp;
	ReadStringLine(is, tmp);
	for (int i = 0; i < 4; i++)
	{
		ReadStringLine(is, tmp);
		if (tmp != m_dllList[i])
		{
			char str[200];
			sprintf_s(str, 200,
				"Algorithms are not the same!\n The program is using %s but the file to load uses %s", m_dllList[i],tmp);
			MessageBox1(str);
			return false;
		}
	}

	int dataBytes, modelNum;
	ReadIntText(is, dataBytes);
	if (dataBytes == 4 && CV_MODEL_FC1 == dataBytes)
	{
		::MessageBox1("data type (float/double/uchar) not match!");
		return 0;
	}

	ReadIntText(is, modelNum);
	ReadIntText(is, m_modelSz);

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

