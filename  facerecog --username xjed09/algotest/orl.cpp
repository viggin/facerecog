// ORL库的测试函数
#include "algotest.h"

#ifdef COMPILE_DATASET_ORL

vector<SFInfo>	trainList;
vector<SFInfo>	testList;
vector<SMatch>	resList;


void run_orl()
{
	faceSz = cvSize(92, 112)*.5;
	rootPath = "D:\\facePics\\orl_png\\";
	trainFlag = FM_DO_NOT_NORM | FM_SHOW_DETAIL | FM_TRAIN_SAVE2MODEL; // ORL库不用归一化，且训练用的样本即是模板
	testFlag = FM_DO_NOT_NORM | FM_SHOW_DETAIL;

	if(! fm.Init(faceSz)) exit(1);

	int runTimes = 1; // 重复测试次数
	double rate = 0;
	for (int i = 0; i < runTimes; i++)
	{
		orl_list();
		tic();
		fm.Train(rootPath, trainList, trainFlag);
		double r = fm.BatchPicRecog(rootPath, testList, resList, testFlag);
		double t = toc();
		msg.Format("rate = %f, time = %fs\n", r, t);
		out(msg);
		rate += r/runTimes;
	}

	msg.Format("mean rate: %f\n", rate);
	out(msg);
}

// 产生文件信息列表
void orl_list()
{
	trainList.clear();
	testList.clear();
	resList.clear();

	int classNum = 40, classPopu = 10, trainNum = 2; // 类别数，每类样本总数，其中用来训练的样本数
	int v1[] = {1,5,9,2,3,4,6,7,8,10};

	CString picPath;
	//srand((UINT)time(0)); // 必须有此句，下一句才能每次产生不同的顺序
	//random_shuffle( v1, v1+10 ); // 随机打乱顺序

	for (int i = 1; i <= classNum; i++)
	{
		for (int j = 0; j < trainNum; j++)
		{
			picPath.Format("s%d\\%d.png", i, v1[j]);
			trainList.push_back(SFInfo(i, picPath));
		}
		for (int j = trainNum; j < classPopu; j++)
		{
			picPath.Format("s%d\\%d.png", i, v1[j]);
			testList.push_back(SFInfo(i, picPath));
		}
	}
}

#endif