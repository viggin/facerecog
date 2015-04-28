/*
	一些定义、声明
*/
#pragma once

#include "stdafx.h"
#include "WebCam.h"
#include "../FaceMngr/FaceMngr.h"

extern CFaceMngr	*g_faceMngr;
extern bool			g_bHasWebcam;
extern CWebCam		g_webcam;
extern CString		g_strPicPath, g_strFacePath;

//extern vector<SIdxStr>		g_lstName;
