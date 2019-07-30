#pragma once
#include<list>
#include"SessionElement.h"
#include"CachePackets.h"
class CSessions
{
public:

	CSessions()
	{
		NoFlag = 0;
	}

	virtual ~CSessions()
	{
	}
private:
	ClientEnum	mClientType;			//客户端类型
public:
	unsigned int NoFlag;//标识作用
	std::list<CSessionElement> mSessions;//只能使用AddSession进行添加，不允许使用如push_back等函数
	CCachePackets mCachePackets;
public:
	void SetClientType(ClientEnum m)
	{
		mClientType = m;
	}
	ClientEnum GetClientType()
	{
		return mClientType;
	}
	void AddSession(CSessionElement m)
	{
		mSessions.push_back(m);
	}
	const CSessionElement *FindSession(unsigned int _uuid)
	{
		std::list<CSessionElement>::iterator itor1=
			std::find_if(mSessions.begin(), mSessions.end(), [_uuid](CSessionElement const& obj){
			return obj.Getuuid() == _uuid;
		});
		if (itor1 != mSessions.end())
		{
			return &(*itor1);
		}
		return 0;
	}

};

