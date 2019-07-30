#pragma once
#include<string>
#include"ByteBuffer.h"

class CSyncPacketElement
{
public:

	CSyncPacketElement()
	{
	}

	virtual ~CSyncPacketElement()
	{
	}
public:
	unsigned int   inPacketsNo;
	unsigned short cmdid;
	unsigned int id;
	std::string serviceCmd;
	bool client2server;
	ByteBuffer _payload;
	unsigned char ciphertype;	//0=数据包未加密,1=SessionKey加密，2=全0密钥加密,其它未知
	unsigned int ciphertextoffset;
	unsigned int ciphertextlen;
	bool	isStreamEnd;
	unsigned int streamResiduelen;
public:
	bool operator<(const CSyncPacketElement &t1)const
	{
		return inPacketsNo < t1.inPacketsNo;
	}
};

