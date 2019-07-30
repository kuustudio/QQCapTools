#pragma once
#include<list>
#include"packet.h"
#include"SyncPacketElement.h"
#include"NetInfo.h"

class CSyncPackets
{
public:

	CSyncPackets()
	{
	}

	virtual ~CSyncPackets()
	{
	}
public:
	MacInfo		mMacInfo;
	CNetInfo	mNetInfo;
	std::list<CSyncPacketElement> mPackets;
};

