#include "QQSendPack.h"
#include"../Packetyzer/Packetyzer.h"
#include"TeaCrypt.h"
#include"AnalysisCap.h"


CQQSendPack::CQQSendPack()
{
}


CQQSendPack::~CQQSendPack()
{
}

void CQQSendPack::send(CString _key,CString _id, DWORD _srcip, CString _srcport, CString _srcmac,
DWORD _dstip, CString _dstport, CString _dstmac,CString _text)
{
	_srcip = htonl(_srcip);
	_dstip = htonl(_dstip);
	std::string  srcip = inet_ntoa(*(in_addr*)(&_srcip));
	std::string dstcip = inet_ntoa(*(in_addr*)(&_dstip));
	DWORD srcport = atoll(_srcport);
	DWORD dstport = atoll(_dstport);
	DWORD id = atoll(_id);
	std::string srcmac = TransMac(_srcmac);
	std::string dstmac = TransMac(_dstmac);

	std::string strHex = CAnalysisCap::StrToHex((BYTE*)_key.GetBuffer(0), _key.GetLength());
	std::string data = _text;
	ByteBuffer b = textChat(id, (unsigned char*)data.c_str(), data.size(), (unsigned char*)strHex.c_str());

	Packetyzer::Generators::cPacketGen PG(GENERATE_UDP);
	PG.SetMACAddress(srcmac.c_str(), dstmac.c_str());
	PG.SetIPAddress(srcip.c_str(), dstcip.c_str());
	PG.SetPorts(srcport, dstport);
	PG.CustomizeUDP((UCHAR*)b.contents(), b.size());


	Packetyzer::Send::cWinpcapSend send;
	if (send.isReady)
	{
		cPacket tmp(PG.GeneratedPacket, PG.GeneratedPacketSize);
		if (send.SendPacket(1, &tmp))
			MessageBox(0,"发送成功","消息",MB_OK);
		else
			MessageBox(0, "发送失败", "消息", MB_OK);
	}
}

ByteBuffer CQQSendPack::textChat(DWORD id, unsigned char *buf, unsigned int len, unsigned char *key)
{
	ByteBuffer data;
	data.append(htonl(2799044801));
	data.append(htonl(id));
	//char tbuf1[] = { 0x00, 0x00, 0x00, 0x0d, 0x00, 0x01, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x01, 0x04 };
	char tbuf1[] = { 0x00, 0x00, 0x00, 0x08, 0x00, 0x01, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00};
	data.append(tbuf1, sizeof(tbuf1));
	data.append(char(0x37));
	data.append(char(0x27));
	data.append(htonl(2799044801));
	data.append(htonl(id));

	ByteBuffer _md5;
	_md5.append(int(htonl(id)));
	_md5.append(key, 0x10);
	ByteBuffer _hash;
	CAnalysisCap::GetMd5(_md5, _hash);
	//计算MD5
	data.append(_hash.contents(), _hash.size());
	data.append(htons(0x000b));
	data.append(short(rand()));
	time_t timep;
	timep = time(0);
	data.append(htonl(timep));
	char tbuf3[] = "\x02\x22\x00\x00\x00\x00\x01\x00\x00\x00\x01\x4d\x53\x47\x00\x00\x00\x00\x00";
	data.append(tbuf3, sizeof(tbuf3)-1);
	data.append(htonl(timep));
	char tbuf4[] = { 0x9E, 0x9B, 0xC4, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x0A, 0x00, 0x86, 0x22, 0x00, 0x0C, 0xE5, 0xBE, 0xAE, 0xE8, 0xBD, 0xAF, 0xE9, 0x9B, 0x85, 0xE9, 0xBB, 0x91, 0x00, 0x00, 0x01 };
	data.append(tbuf4, sizeof(tbuf4));
	data.append(htons(len + 3));
	data.append(char(0x01));
	//增加聊天内容
	data.append(htons(len));
	data.append(buf, len);

	unsigned char *dst = (unsigned char*)malloc(1024);
	unsigned int bytes = 0;
	CTeaCrypt mTeaCrypt;
	mTeaCrypt.InitTeaOne();
	mTeaCrypt.Tea_encrypt((unsigned char *)data.contents(), data.size(), key, dst, &bytes);
	data.clear();

	data.append(char(0x02));
	data.append(char(0x37));
	data.append(char(0x27));
	data.append(char(0x00));
	data.append(char(0xcd));
	data.append(short(rand()));
	data.append(htonl(2799044801));

	char tbuf5[] = { 0x02, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x00, 0x00, 0x68, 0x69 };
	data.append(tbuf5, sizeof(tbuf5));

	data.append(dst, bytes);
	free(dst);
	dst = NULL;
	data.append(char(0x03));
	

	return data;
}

std::string CQQSendPack::TransMac(CString _mac)
{
	std::string strMac = "";
	for (int i = 0; i <_mac.GetLength(); i++)
	{
		if (i % 2 == 0&&i!=0)
			strMac.push_back(':');
		strMac.push_back(_mac[i]);
	}
	return strMac;
}