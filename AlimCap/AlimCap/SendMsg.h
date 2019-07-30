#pragma once
#include<Windows.h>
#include"packet.h"
#include"NetInfo.h"
class CSendMsg
{
public:

	CSendMsg()
	{
	}

	virtual ~CSendMsg()
	{
	}
public:
	MacInfo mMacInfo;
	CNetInfo	mNetInfo;
	LPVOID	pMsgDialog;
	HWND	callHandle;
	unsigned int nIndex;
};

