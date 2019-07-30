#pragma once
#include"UserInfo.h"
#include"SyncPackets.h"
#include"PCUserInfo.h"
#include"AndroidUserInfo.h"
#include"IosUserInfo.h"

class CSessionElement
{
private:
	CSessionElement()
	{
		pUserInfo = 0;
		GUID guid;
		CoCreateGuid(&guid);
		uuid = guid.Data1 + guid.Data2 + guid.Data3;
	}
public:
	CSessionElement(const CSessionElement& C)
	{
		uuid = C.uuid;
		mPackets = C.mPackets;
		switch (C.pUserInfo->mClientType)
		{
		case Enum_PC:
		{
			pUserInfo = new CPCUserInfo((*static_cast<CPCUserInfo*>(C.pUserInfo)));
			pUserInfo->mClientType = Enum_PC;
		}break;
		case Enum_Android:
		{
			pUserInfo = new CAndroidUserInfo((*static_cast<CAndroidUserInfo*>(C.pUserInfo)));
			pUserInfo->mClientType = Enum_Android;
		}break;
		case Enum_IOS:
		{
			pUserInfo = new CIosUserInfo((*static_cast<CIosUserInfo*>(C.pUserInfo)));
			pUserInfo->mClientType = Enum_IOS;
		}break;
		}
	}
	CSessionElement(ClientEnum m)
	{
		pUserInfo = 0;
		GUID guid;
		CoCreateGuid(&guid);
		uuid = guid.Data1 + guid.Data2 + guid.Data3;
		SetClientType(m);
	}

	virtual ~CSessionElement()
	{
		if (pUserInfo)
		{
			delete pUserInfo;
			pUserInfo = 0;
		}
	}
private:
	unsigned int uuid;
public:
	CUserInfo	*pUserInfo;
	CSyncPackets mPackets;
	void AddPacket(CSyncPacketElement m)
	{
		mPackets.mPackets.push_back(m);
	}
	void SetClientType(ClientEnum m)
	{
		switch (m)
		{
			case Enum_PC:
			{
				pUserInfo = new CPCUserInfo();
				pUserInfo->mClientType = Enum_PC;
			}break;
			case Enum_Android:
			{
				pUserInfo = new CAndroidUserInfo();
				pUserInfo->mClientType = Enum_Android;
			}break;
			case Enum_IOS:
			{
				pUserInfo = new CIosUserInfo();
				pUserInfo->mClientType = Enum_IOS;
			}break;
		}
	}
	unsigned int Getuuid() const{ return uuid; };
};

