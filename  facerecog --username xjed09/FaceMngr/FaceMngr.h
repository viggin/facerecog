#ifdef MNGR_HEADER

#define COMPILE_MNGR_COMPLETE

#define CV_MODEL_FC1 CV_64FC1 // 存储数据精度

// flags used in Pic2NormFace, Enroll, Train, PicRecognize...
#define FM_DO_NOT_NORM		0 // 不进行几何和光照归一化
#define FM_DO_FACE_ALIGN	1 // 进行几何归一化
#define FM_DO_LIGHT_PREP	2 // 进行光照归一化
#define FM_ALIGN_USE_BUF	4 // 几何归一化时使用平滑
#define FM_DO_NORM		(FM_DO_FACE_ALIGN | FM_DO_LIGHT_PREP)
// 以上flag用于归一化时
#define FM_SAVE_NORM_FACE	8 // 注册或训练时保存归一化人脸到文件
#define FM_TRAIN_SAVE2MODEL	16 // 训练时将训练图像保存到模板库
#define FM_SAVE_REL_PATH	32 // 训练时不保存完整路径
#define FM_SHOW_DETAIL		64 // cout一些信息



// 一些意外情况下的class ID
#define FM_UNKNOWN_CLASS_ID	-1 
#define FM_RECOG_NOT_DONE	-3

#else

#include "FaceMngr_complete.h"
#include "FaceMngr_fast.h"
//#include "FaceMngr_fast_fast.h"

#endif