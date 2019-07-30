#pragma once
#include"packet.h"
class CUserInfo
{
public:

	CUserInfo()
	{
		mClientType = Enum_PC;
		isCanDecrypt = false;
		memset(Key0, 0, sizeof(Key0));
		memset(_tgtgt_key, 0, sizeof(_tgtgt_key));
	}

	virtual ~CUserInfo()
	{
	}
public:
	ClientEnum	mClientType;			//客户端类型
	bool		isCanDecrypt;			//是否可解密
	unsigned char Key0[KEYLEN];			//全0密钥
	unsigned char _tgtgt_key[KEYLEN];	//tgtgt key
	std::map<unsigned int, UserKey>	KeysUinion;//存储当前获取的用户密码信息
private:
	std::list<UserId> idList;
public:
	void SetId(unsigned int id)
	{
		if (!isFind(id) && id != 0)
		{
			UserId mUserId;
			mUserId.id = id;
			_ultoa(id, (char*)mUserId.szUserId, 10);
			idList.push_back(mUserId);
		}
	}
	UserId GetMainId()
	{
		//UserId m;
		//FindMain(m);
		return *idList.begin();
	}
	std::list<UserId> &GetId(){ return idList; };

	ClientEnum GetClientType(){ return mClientType; };

	bool isFind(unsigned int id)
	{
		if (idList.size() != 0)
		{
			std::list<UserId>::iterator itor =
			std::find_if(idList.begin(), idList.end(), [id](UserId const& obj){
				return obj.id == id;
			});
			if (itor != idList.end())
				return true;
		}
		return false;
	}

	bool FindMain(UserId& m)
	{
		if (idList.size() != 0)
		{
			std::list<UserId>::iterator itor = idList.begin();
			for (; itor != idList.end(); itor++)
			{
				if (itor->isMain == true)
				{
					m = *itor;
					return true;
				}
			}
		}
		return false;
	}
public:
	virtual void SetKey0825(unsigned char* buffer, unsigned int len){};
	virtual unsigned char* GetKey0825(){ return 0; };
	virtual void SetKey0836(unsigned char* buffer, unsigned int len){};
	virtual unsigned char* GetKey0836(){ return 0; };
	virtual void SetTlv0107(unsigned char *buffer, unsigned int len){};
	virtual unsigned char* GetTlv0107(){ return 0; };
	virtual void SetTlv010C(unsigned int id,unsigned char *buffer, unsigned int len){};
	virtual void SetTlv0109(unsigned char *buffer, unsigned int len){};
	virtual unsigned char* GetTlv0109(){ return 0; };
	virtual void SetTlv0006(unsigned char *buffer, unsigned int len){};
	virtual unsigned char* GetTlv0006(){ return 0; };
};

