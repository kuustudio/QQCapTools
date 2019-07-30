#pragma once
#include"SyncPackets.h"
#include"FilterInfo.h"
class CCachePackets
{
public:

	CCachePackets()
	{
	}

	virtual ~CCachePackets()
	{
	}
public:
	std::list<CSyncPackets> mCachePackets;
	CFilterInfo mFilterInfo;
public:
	std::list<CSyncPacketElement>* FindPacket(CNetInfo n)
	{
		std::list<CSyncPackets>::iterator itr=
		std::find_if(mCachePackets.begin(), mCachePackets.end(), [n](CSyncPackets const& obj){
			return obj.mNetInfo == n;
		});
		if (itr != mCachePackets.end())
			return &itr->mPackets;
		return NULL;
	}
};

