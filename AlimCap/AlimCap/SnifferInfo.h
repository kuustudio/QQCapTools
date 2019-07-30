#pragma once
#include<Windows.h>
#include"SyncPackets.h"
class CSnifferInfo
{
public:

	CSnifferInfo()
	{
		filterId = 0;
		sel = 0;
		pCapture = NULL;
	}

	virtual ~CSnifferInfo()
	{
	}
public:
	HWND msgHandle;
	CSyncPackets mSnifferPackets;
	Packetyzer::Capture::cWinpcapCapture *pCapture;
	unsigned int sel;
	DWORD filterId;
};

