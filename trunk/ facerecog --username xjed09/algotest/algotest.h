#include "../FaceMngr/FaceMngr.h"
#include <time.h>

#define COMPILE_DATASET_ORL

extern CFaceMngr fm;
extern ofstream os;
extern CString msg;

extern CString	rootPath; // 人脸库根路径
extern DWORD	trainFlag, enrFlag, testFlag; // 选项
extern CvSize	faceSz; // 人脸尺寸

void run_orl();
void orl_list();

void run_feret();
void feret_list();

void out(LPCTSTR str);