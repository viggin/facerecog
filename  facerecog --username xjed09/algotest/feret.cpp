// FERET库测试函数。用到了一些文件列表txt，列表比起原始的灰度FERET库的list略有修正
#include "algotest.h"

#ifdef COMPILE_DATASET_FERET

vector<SFInfo>	trainList; // 训练文件信息列表
vector<SFInfo>	faList, fbList, fcList, dupIList; // 模板和测试文件信息列表
vector<SMatch>	fbRes, fcRes, dupIRes; // 匹配结果
int				trainNum, faNum, fbNum, fcNum, dupINum; // 各子库图片数


void run_feret()
{
	faceSz = cvSize(65, 80);
	rootPath = "..\\..\\feret_tif\\gray feret\\data\\";
	trainFlag = FM_DO_NORM | FM_SHOW_DETAIL;
	enrFlag = testFlag = trainFlag;

	if(! fm.Init(faceSz)) exit(1);

	feret_list();
	tic(); // 记录用时
	fm.Train(rootPath, trainList, trainFlag); // 训练
	fm.BatchPicEnroll(rootPath, faList, enrFlag); // 注册模板

	// 测试FB、fc、duplicate I子库
	double rb = fm.BatchPicRecog(rootPath, fbList, fbRes, testFlag),
		rc = fm.BatchPicRecog(rootPath, fcList, fcRes, testFlag),
		rdI = fm.BatchPicRecog(rootPath, dupIList, dupIRes, testFlag);

	double t = toc();
	msg.Format("rate = %f, %f, %f, time = %fs\n", rb, rc, rdI, t); // 输出
	out(msg);
}

// 生成文件信息列表
void feret_list()
{
	//CString namesPath = rootPath + "partitions\\by_previously_reported\\feret\\",
		//coordPath = rootPath + "name_value\\";
	//ifstream isNames, isCoord;
	//char name[20];

	/*isNames.open(namesPath + "train_fafb.txt");
	trainNum = 996;
	for (int i = 0; i < trainNum; i++)
	{
		isNames.getline(name, 20);
		isCoord.open(coordPath + name + ".gnd");

	}*/
	ifstream is;
	char name[30];
	int id, leyex, leyey, reyex, reyey;

	is.open("feret_train_list.txt"); // 读入预先准备的文件
	trainNum = 996;
	for (int i = 0; i < trainNum; i++)
	{
		is>>name>>id>>leyex>>leyey>>reyex>>reyey;
		strcat_s(name, ".tif");
		trainList.push_back(SFInfo(id, name, 
			cvPoint2D32f(leyex,leyey), cvPoint2D32f(reyex,reyey))); // 加入列表
	}
	is.close();

	is.open("feret_fa_list.txt");
	faNum = 1196;
	for (int i = 0; i < faNum; i++)
	{
		is>>name>>id>>leyex>>leyey>>reyex>>reyey;
		strcat_s(name, ".tif");
		faList.push_back(SFInfo(id, name, 
			cvPoint2D32f(leyex,leyey), cvPoint2D32f(reyex,reyey)));
	}
	is.close();

	is.open("feret_fb_list.txt");
	fbNum = 1195;
	for (int i = 0; i < fbNum; i++)
	{
		is>>name>>id>>leyex>>leyey>>reyex>>reyey;
		strcat_s(name, ".tif");
		fbList.push_back(SFInfo(id, name, 
			cvPoint2D32f(leyex,leyey), cvPoint2D32f(reyex,reyey)));
	}
	is.close();

	is.open("feret_fc_list.txt");
	fcNum = 194;
	for (int i = 0; i < fcNum; i++)
	{
		is>>name>>id>>leyex>>leyey>>reyex>>reyey;
		strcat_s(name, ".tif");
		fcList.push_back(SFInfo(id, name, 
			cvPoint2D32f(leyex,leyey), cvPoint2D32f(reyex,reyey)));
	}
	is.close();

	is.open("feret_dupI_list.txt");
	dupINum = 722;
	for (int i = 0; i < dupINum; i++)
	{
		is>>name>>id>>leyex>>leyey>>reyex>>reyey;
		strcat_s(name, ".tif");
		dupIList.push_back(SFInfo(id, name, 
			cvPoint2D32f(leyex,leyey), cvPoint2D32f(reyex,reyey))); // 加入列表
	}
	is.close();
}

#endif