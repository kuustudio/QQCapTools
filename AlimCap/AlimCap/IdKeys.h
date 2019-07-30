#pragma once
#include"packet.h"

class IdKeyUnion
{
private:
	unsigned int uuid;
public:
	unsigned int	id;
	ClientEnum	cType;
	bool	isValid;
	std::string PwdHash;
	std::string ShareKey;
	std::string SessionKey;
public:
	IdKeyUnion()
	{
		GUID guid;
		CoCreateGuid(&guid);
		uuid = guid.Data1 + guid.Data2 + guid.Data3;
		id = 0;
		cType = ClientEnum::Enum_Unkonw;
		isValid = false;
		PwdHash = std::string(0x10, 0);
		ShareKey = std::string(0x10, 0);
		SessionKey = std::string(0x10, 0);
	}
public:
	unsigned int Getuuid() const{ return uuid; };
};


class IdKeys
{
public:
	IdKeys()
	{
	}

	virtual ~IdKeys()
	{
	}
public:
	std::list<IdKeyUnion> _IdKeyUnions; //只能使用AddSession进行添加，不允许使用如push_back等函数
public:
	void AddItem(IdKeyUnion m)
	{
		_IdKeyUnions.push_back(m);
	}
	void DeleteItem(std::list<IdKeyUnion>::iterator itr){ _IdKeyUnions.erase(itr); };
	bool isFind(unsigned int qq,ClientEnum type)
	{
		std::list<IdKeyUnion>::iterator itr = _IdKeyUnions.begin();
		for (; itr != _IdKeyUnions.end(); itr++)
		{
			if (itr->id == qq&&itr->cType == type)
				return true;
		}
		return false;
	}
	std::list<IdKeyUnion>::iterator GetItem(unsigned int qq,ClientEnum type)
	{
		std::list<IdKeyUnion>::iterator itr = _IdKeyUnions.begin();
		for (; itr != _IdKeyUnions.end(); itr++)
		{
			if (itr->id == qq&&itr->cType == type)
				return itr;
		}
		return _IdKeyUnions.end();
	};
	bool isFindValid(unsigned int qq, ClientEnum type)
	{
		std::list<IdKeyUnion>::iterator itr = _IdKeyUnions.begin();
		for (; itr != _IdKeyUnions.end(); itr++)
		{
			if (itr->id == qq&&itr->cType == type&&itr->isValid)
				return true;
		}
		return false;
	}
	std::list<IdKeyUnion>::iterator GetItemValid(unsigned int qq, ClientEnum type)
	{
		std::list<IdKeyUnion>::iterator itr = _IdKeyUnions.begin();
		for (; itr != _IdKeyUnions.end(); itr++)
		{
			if (itr->id == qq&&itr->cType == type&&itr->isValid)
				return itr;
		}
		return _IdKeyUnions.end();
	};
	bool isFindByuuid(unsigned int _uuid)
	{
		std::list<IdKeyUnion>::iterator itor1 =
			std::find_if(_IdKeyUnions.begin(), _IdKeyUnions.end(), [_uuid](IdKeyUnion const& obj){
			return obj.Getuuid() == _uuid;
		});
		if (itor1 != _IdKeyUnions.end())
		{
			return true;
		}
		return false;
	}
	std::list<IdKeyUnion>::iterator GetItemByuuid(unsigned int _uuid)
	{
		std::list<IdKeyUnion>::iterator itor1 =
			std::find_if(_IdKeyUnions.begin(), _IdKeyUnions.end(), [_uuid](IdKeyUnion const& obj){
			return obj.Getuuid() == _uuid;
		});
		return itor1;
	}
};

