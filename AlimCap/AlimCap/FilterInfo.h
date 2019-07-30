#pragma once
#include "NetInfo.h"
class CFilterInfo :
	public CNetInfo
{
public:

	CFilterInfo()
	{
		isSrcIpCheck = false;
		isSrcPortCheck = false;
		isDstIpCheck = false;
		isDstPortCheck = false;
	}

	virtual ~CFilterInfo()
	{
	}
public:
	bool isSrcIpCheck;
	bool isSrcPortCheck;
	bool isDstIpCheck;
	bool isDstPortCheck;
};

