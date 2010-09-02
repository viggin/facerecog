// 利用人脸库进行算法测试，目前支持ORL库和FERET库
// 比较奇怪的是，用Matlab 2009b 类似的测试程序运行，Matlab反倒比c快一些，猜测是Matlab矩阵操作较快
// FaceMngr需要在 FaceMngr.h 中定义 COMPILE_MNGR_FAST；FERET库的语句可能还需要测试一下

#include "algotest.h"

ofstream os;
CString msg;

CFaceMngr fm;

CString	rootPath;
DWORD	trainFlag, enrFlag, testFlag;
CvSize	faceSz;

void out(LPCTSTR str)
{
	//TRACE(str);
	os<<str;
}

void main()
{
	os.open("..\\output.txt", ios::app); // 输入信息到文件

	CString msg;
	time_t ti = time(0);
	char tmp[64]; // 记录测试时间
	strftime(tmp, sizeof(tmp), "%m/%d %X %a",localtime(&ti));
	msg.Format("\n%s\t", tmp);
	out(msg);

	out("feret\n");

	run_orl();

	os.close();
	//system("pause");
}
