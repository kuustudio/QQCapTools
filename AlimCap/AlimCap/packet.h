#pragma once
#include<string>
#include<list>
#include<algorithm>
#include<map>
#include"ByteBuffer.h"

namespace Packetyzer
{
	namespace Capture
	{
		class cWinpcapCapture;
	}
}

//‘§∂®“Â
#define USERSIZE	0x20
#define KEYLEN		0x10
#define MD5LEN		0x10

enum ClientEnum
{
	Enum_PC =		0,
	Enum_Android =	1,
	Enum_IOS =		2,
	Enum_Unkonw =	0xF
};

typedef struct _UserId
{
	bool	isMain;
	unsigned int id;
	unsigned char szUserId[USERSIZE];
	_UserId()
	{
		isMain = false;
		id = 0;
		memset(szUserId, 0, USERSIZE);
	}
}UserId, *PUserId;

typedef struct _UserKey
{
	unsigned char SessionKey[KEYLEN];
	_UserKey()
	{
		memset(SessionKey, 0, KEYLEN);
	}
}UserKey, *PUserKey;

#define MAC_LEN	0x6
typedef struct _MacInfo
{
	unsigned char srcMac[MAC_LEN];
	unsigned char dstMac[MAC_LEN];

}MacInfo, *PMacInfo;

typedef struct _NetCardInfo
{
	std::string name;
	std::string description;
}NetCardInfo, *PNetCardInfo;

typedef std::vector<NetCardInfo> NetCardInfos;
