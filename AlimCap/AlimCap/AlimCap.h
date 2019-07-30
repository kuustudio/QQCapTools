
// AlimCap.h : PROJECT_NAME 应用程序的主头文件
//

#pragma once

#include<afx.h>
#include<afxshellmanager.h>
#include<Windows.h>
#include<afxdisp.h>

#include "resource.h"		// 主符号
#include"AnalysisCap.h"


// CAlimCapApp:
// 有关此类的实现，请参阅 AlimCap.cpp
//

class CAlimCapApp : public CWinApp
{
public:
	CAlimCapApp();

// 重写
public:
	virtual BOOL InitInstance();

// 实现

	DECLARE_MESSAGE_MAP()

public:
	//全局cap分析对象
	CAnalysisCap CACap;
};

extern CAlimCapApp theApp;