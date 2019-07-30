#pragma once
#include<afx.h>
#include"ByteBuffer.h"

class CQQSendPack
{
public:
	CQQSendPack();
	virtual ~CQQSendPack();
public:
	void send(CString _key,CString _id, DWORD _srcip, CString _srcport, CString _srcmac,
		DWORD _dstip,CString _dstport,CString _dstmac,CString _text);
public:
	ByteBuffer textChat(DWORD id,unsigned char *buf, unsigned int len,unsigned char *key);
	std::string TransMac(CString);
};

