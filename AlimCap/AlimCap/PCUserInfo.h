#pragma once
#include"packet.h"
#include "UserInfo.h"
class CPCUserInfo :
	public CUserInfo
{
public:

	CPCUserInfo()
	{
	}
	virtual ~CPCUserInfo()
	{
	}
public:
	unsigned char vague_key[KEYLEN];
	unsigned char key0825[KEYLEN];
	unsigned char key0836[KEYLEN];
	unsigned char key0836ret1[KEYLEN];
	unsigned char key0836ret2[KEYLEN];
public:
	void SetTlv0107(unsigned char* buffer, unsigned int len)
	{
		memcpy(key0836ret2, buffer, len);
	}
	unsigned char* GetTlv0107()
	{
		return key0836ret2;
	}
	void SetTlv010C(unsigned int _id,unsigned char *buffer, unsigned int len)
	{
		UserKey key;
		memcpy(key.SessionKey, buffer + 0x2, KEYLEN);
		KeysUinion.insert(std::pair<unsigned int, UserKey>(_id, key));
		isCanDecrypt = true;
	}

	void SetTlv0109(unsigned char* buffer, unsigned int len)
	{
		memcpy(key0836ret1, buffer, len);
	}
	unsigned char* GetTlv0109()
	{
		return key0836ret1;
	}
	void SetTlv0006(unsigned char* buffer, unsigned int len)
	{
		memcpy(vague_key, buffer, len);
	}
	unsigned char* GetTlv0006()
	{
		return vague_key;
	}
	void SetKey0825(unsigned char* buffer, unsigned int len)
	{
		memcpy(key0825, buffer, len);
	}
	unsigned char* GetKey0825()
	{
		return key0825;
	}
	void SetKey0836(unsigned char* buffer, unsigned int len)
	{
		memcpy(key0836, buffer, len);
	}
	unsigned char* GetKey0836()
	{
		return key0836;
	}
};

