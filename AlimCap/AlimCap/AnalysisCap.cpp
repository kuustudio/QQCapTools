// AnalysisCap.cpp : 实现文件
//

//#include "stdafx.h"
#include "AlimCap.h"
#include "AnalysisCap.h"
#include<WinCrypt.h>
#include"TeaCrypt.h"
#include"mymsg.h"
#include<fstream>
#include<iomanip>

#include"../Packetyzer/Packetyzer.h"
#pragma comment(lib,"..\\Debug\\Packetyzer.lib")


// CAnalysisCap

CAnalysisCap::CAnalysisCap()
{
	isReport = false;
}

CAnalysisCap::~CAnalysisCap()
{
}


void CAnalysisCap::SetFilter(pcap_t *fp,char *strFilter)
{
	struct bpf_program fcode;
	//compile the filter
	if(pcap_compile(fp, &fcode, strFilter, 1, 0xffffff) < 0)
	{
		AfxMessageBox("过滤器编译失败!");
		CapClose(fp);
		return;
	}
	if(pcap_setfilter(fp, &fcode)<0)
	{
		AfxMessageBox("过滤器设置失败!");
		pcap_close(fp);
		return;
	}

}

void CAnalysisCap::ReleaseFilter(pcap_t *)
{
	//pcap_freecode(
}

pcap_t *CAnalysisCap::CapOpen(const char *File)
{
	char errbuf[100]={0};
	return pcap_open_offline(File,errbuf);
}

void CAnalysisCap::CapClose(pcap_t *fp)
{
	pcap_close(fp);
}

bool CAnalysisCap::StartOpenSniffer(const char * name, HWND msgHandle)
{
	if (mSniffer.pCapture != NULL)
	{
		delete mSniffer.pCapture;
		mSniffer.pCapture = NULL;
	}
	mSniffer.mSnifferPackets.mPackets.clear();
	mSniffer.pCapture = new Packetyzer::Capture::cWinpcapCapture();
	int sel = 0;
	if (mSniffer.pCapture->isReady)
	{
		for (UINT i = 0; i< mSniffer.pCapture->nAdapters; i++)
		{
			if (strcmp(mSniffer.pCapture->Adapters[i].Name, name) == 0)
				sel = i;
		}
	}
	sel = sel + 1;
	mSniffer.sel = sel;
	mSniffer.msgHandle = msgHandle;
	//需要开启一个线程
	AfxBeginThread(snifferThreadFunc, (LPVOID)&mSniffer); //开启播放线程
	return true;
}

void CAnalysisCap::StopOpenSniffer()
{
	//先结束线程
	if (mSniffer.pCapture != NULL)
	{
		mSniffer.pCapture->StopCapture();
		delete mSniffer.pCapture;
		mSniffer.pCapture = NULL;
	}
}

UINT CAnalysisCap::snifferThreadFunc(LPVOID pParam)
{
			// TODO:
	CSnifferInfo *p = (CSnifferInfo*)pParam;
	p->pCapture->CapturePackets(pParam, packet_handler, p->sel, -1, 0);
	return 0;
}

void CAnalysisCap::packet_handler(LPVOID uParam, const u_char *pkt_data, unsigned int len)
{
	CSnifferInfo* pSnifferInfo = (CSnifferInfo*)uParam;
	cPacket* TestPacket = new cPacket((UCHAR*)pkt_data, len);

	unsigned char *pbody = NULL;
	unsigned int bodylen = 0;
	CNetInfo nTemp;
	if (TestPacket->isUDPPacket)
	{
		pbody = TestPacket->UDPData;
		bodylen = TestPacket->UDPDataSize;
		nTemp.proto = TestPacket->IPHeader->Protocol;
		nTemp.srcip = TestPacket->IPHeader->SourceAddress;
		nTemp.srcport = TestPacket->UDPHeader->SourcePort;
		nTemp.dstip = TestPacket->IPHeader->DestinationAddress;
		nTemp.dstport = TestPacket->UDPHeader->DestinationPort;
	}
	else if (TestPacket->isTCPPacket)
	{
		pbody = TestPacket->TCPData;
		bodylen = TestPacket->TCPDataSize;
		nTemp.proto = TestPacket->IPHeader->Protocol;
		nTemp.srcip = TestPacket->IPHeader->SourceAddress;
		nTemp.srcport = TestPacket->TCPHeader->SourcePort;
		nTemp.dstip = TestPacket->IPHeader->DestinationAddress;
		nTemp.dstport = TestPacket->TCPHeader->DestinationPort;
	}
	else
	{
		delete(TestPacket);
		return;
	}


	// 去除无数据包
	if (bodylen < 0x10)
	{
		delete(TestPacket);
		return;
	}

	//过滤腾讯数据包
	if (pbody[0] != 0x02 || pbody[bodylen - 1] != 0x03)
	{
		delete(TestPacket);
		return;
	}
	if (pbody[1] == 0x37 && pbody[2] == 0x27)
	{
		DWORD id = htonl(*((int *)(pbody + 0x7)));
		if (id != pSnifferInfo->filterId)
		{
			delete(TestPacket);
			return;
		}
		//if (htons(*(short*)(pbody + 3)) != 0x00CD)
		//{
		//	delete(TestPacket);
		//	return;
		//}

		CSyncPackets *p = &(pSnifferInfo->mSnifferPackets);
		p->mNetInfo = nTemp;
		memcpy(p->mMacInfo.srcMac, TestPacket->EthernetHeader->SourceHost, MAC_LEN);
		memcpy(p->mMacInfo.dstMac, TestPacket->EthernetHeader->DestinationHost, MAC_LEN);
		CSyncPacketElement mSyncPacket;
		mSyncPacket.inPacketsNo = 0;
		mSyncPacket.isStreamEnd = true;
		mSyncPacket._payload.append(pbody, bodylen);
		unsigned short cmdid = htons(*(short*)(pbody + 3));
		mSyncPacket.cmdid = cmdid;
		//取用户id
		mSyncPacket.id = id;
		if (*(pbody + 0xb) != 0)
		{
			//发送
			if (*(pbody + 0xb) == 0x2)
			{
				mSyncPacket.client2server = true;
				mSyncPacket.ciphertextoffset = 0x16;
				mSyncPacket.ciphertextlen = mSyncPacket._payload.size() - 0x16 - 1;
			}
			else if (*(pbody + 0xb) == 0x3)
			{
				mSyncPacket.client2server = true;
				mSyncPacket.ciphertextoffset = 0x2A;
				mSyncPacket.ciphertextlen = mSyncPacket._payload.size() - 0x2A - 1;
			}
			else if (*(pbody + 0xb) == 0x4)
			{
				mSyncPacket.client2server = true;
				mSyncPacket.ciphertextoffset = 0x1E;
				mSyncPacket.ciphertextlen = mSyncPacket._payload.size() - 0x1E - 1;
			}
		}
		else
		{
			mSyncPacket.client2server = false;
			mSyncPacket.ciphertextoffset = 0xE;
			mSyncPacket.ciphertextlen = mSyncPacket._payload.size() - 0xE - 1;
		}
		p->mPackets.push_back(mSyncPacket);
		COPYDATASTRUCT cpd;
		cpd.dwData = WM_SNIFFER_ADD;
		cpd.cbData = sizeof(int);
		int k = 99;
		cpd.lpData = &k;
		//标志发送字符串
		SendMessage(pSnifferInfo->msgHandle, WM_COPYDATA, 0, (LPARAM)&cpd);
	}
	delete(TestPacket);
}

void CAnalysisCap::ReleaseSession()
{
	mSessions.mSessions.clear();
	mSessions.mCachePackets.mCachePackets.clear();
	mSessions.NoFlag = 0;
}

bool CAnalysisCap::GetMd5(ByteBuffer szData, ByteBuffer &szMd5)
{
	HCRYPTPROV hCryptProv;
	HCRYPTHASH hHash;
	BYTE bHash[0x10];
	DWORD dwHashLen= 0x10; // The MD5 algorithm always returns 16 bytes.      
	DWORD cbContent = szData.size();
	BYTE* pbContent= (BYTE*)szData.contents();         
	if(CryptAcquireContext(&hCryptProv,NULL,NULL,PROV_RSA_FULL,CRYPT_VERIFYCONTEXT|CRYPT_MACHINE_KEYSET))   
	{     
		if(CryptCreateHash(hCryptProv,CALG_MD5,0, 0, &hHash))     
		{    
			if(CryptHashData(hHash, pbContent, cbContent, 0))      
			{           
				if(CryptGetHashParam(hHash, HP_HASHVAL, bHash, &dwHashLen, 0))       
				{          
					for (int i = 0; i<16; i++)        
					{   
						szMd5.append(bHash[i]);
					}       
				}      
				else 
					return false;
			}     
			else
				return false;
		}     
		else 
			return false;
	}      
	else 
		return false;
	CryptDestroyHash(hHash);       
	CryptReleaseContext(hCryptProv, 0);
	return true;
}

std::string CAnalysisCap::HexToStr(BYTE *pbSrc, int nLen)
{
	std::string str;
	char ddl, ddh;
	int i;

	for (i = 0; i<nLen; i++)
	{
		ddh = 48 + pbSrc[i] / 16;
		ddl = 48 + pbSrc[i] % 16;
		if (ddh > 57) ddh = ddh + 7;
		if (ddl > 57) ddl = ddl + 7;
		str.push_back(ddh);
		str.push_back(ddl);
	}
	return str;
}

std::string CAnalysisCap::HexString(BYTE *lpBuffer, int nCount, bool isSpace, bool isChangeLine, bool isAssic)
{
	const char szHexDigits[] = "0123456789ABCDEF";
	std::string strResult;
	unsigned int line = 0;
	unsigned int sub = nCount % 16;
	if (sub == 0)
		line = nCount / 16;
	else
		line = nCount / 16 + 1;

	for (int l = 0; l < line; l++)
	{
		if (l != line - 1)
		{
			//hex
			for (int i = 0; i < 0x10; i++)
			{
				char j = *(lpBuffer + l * 0x10 + i);
				strResult.push_back(szHexDigits[(j >> 4) & 0xf]);
				strResult.push_back(szHexDigits[j & 0xf]);
				if (isSpace)
					strResult.push_back(' ');
			}
			//assic
			if (isAssic)
			{
				for (int i = 0; i < 0x10; i++)
				{
					char c = *(lpBuffer + l * 0x10 + i);
					if (c >= 32 && c <= 126)
						strResult.push_back(c);
					else
						strResult.push_back('.');
					if ((i + 1) % 16 == 0)
						strResult.push_back('\n');
				}
			}else
				strResult.push_back('\n');
		}
		else
		{
			//添加最后一行
			if (sub == 0)
			{
				for (int i = 0; i < 0x10; i++)
				{
					char j = *(lpBuffer + l * 0x10 + i);
					strResult.push_back(szHexDigits[(j >> 4) & 0xf]);
					strResult.push_back(szHexDigits[j & 0xf]);
					if (isSpace)
						strResult.push_back(' ');
				}
				if (isAssic)
				{
					for (int i = 0; i < 0x10; i++)
					{
						char c = *(lpBuffer + l * 0x10 + i);
						if (c >= 32 && c <= 126)
							strResult.push_back(c);
						else
							strResult.push_back('.');
					}
				}else
					strResult.push_back('\n');
			}
			else
			{
				//最后一行不满
				for (int i = 0; i < 0x10; i++)
				{
					if (i < sub)
					{
						char j = *(lpBuffer + l * 0x10 + i);
						strResult.push_back(szHexDigits[(j >> 4) & 0xf]);
						strResult.push_back(szHexDigits[j & 0xf]);
						if (isSpace)
							strResult.push_back(' ');
					}
					else
					{
						if (isAssic)
						{
							strResult.push_back(' ');
							strResult.push_back(' ');
							if (isSpace)
								strResult.push_back(' ');
						}
					}
				}
				if (isAssic)
				{
					for (int i = 0; i < sub; i++)
					{
						char c = *(lpBuffer + l * 0x10 + i);
						if (c >= 32 && c <= 126)
							strResult.push_back(c);
						else
							strResult.push_back('.');
					}
				}
			}
		}
	}
	return strResult;
}

std::string CAnalysisCap::StrToHex(BYTE *pbSrc, int nLen)
{
	std::string str;
	char h1, h2;
	BYTE s1, s2;
	int i;

	for (i = 0; i<nLen/2; i++)
	{
		h1 = pbSrc[2 * i];
		h2 = pbSrc[2 * i + 1];

		s1 = toupper(h1) - 0x30;
		if (s1 > 9)
			s1 -= 7;

		s2 = toupper(h2) - 0x30;
		if (s2 > 9)
			s2 -= 7;

		str.push_back(s1 * 16 + s2);
	}
	return str;
}

bool CAnalysisCap::ScanPCCap(pcap_t *fp)
{
	//先清空数据
	ReleaseSession();

	bool result=true;
	int res=0;
	struct pcap_pkthdr *header;
	const u_char *pkt_data;
	while ((res = pcap_next_ex(fp, &header, &pkt_data)) >= 0)
	{
		mSessions.NoFlag++;
		cPacket* TestPacket = new cPacket((UCHAR*)pkt_data, header->len);
		unsigned char *pbody = NULL;
		unsigned int bodylen = 0;
		CNetInfo nTemp;
		MacInfo mTemp;
		memcpy(mTemp.srcMac, TestPacket->EthernetHeader->SourceHost, MAC_LEN);
		memcpy(mTemp.dstMac, TestPacket->EthernetHeader->DestinationHost, MAC_LEN);
		if (TestPacket->isUDPPacket)
		{
			pbody = TestPacket->UDPData;
			bodylen = TestPacket->UDPDataSize;
			nTemp.proto = TestPacket->IPHeader->Protocol;
			nTemp.srcip = htonl(TestPacket->IPHeader->SourceAddress);
			nTemp.srcport = htons(TestPacket->UDPHeader->SourcePort);
			nTemp.dstip = htonl(TestPacket->IPHeader->DestinationAddress);
			nTemp.dstport = htons(TestPacket->UDPHeader->DestinationPort);
		}
		else if (TestPacket->isTCPPacket)
		{
			pbody = TestPacket->TCPData;
			bodylen = TestPacket->TCPDataSize;
			nTemp.proto = TestPacket->IPHeader->Protocol;
			nTemp.srcip = htonl(TestPacket->IPHeader->SourceAddress);
			nTemp.srcport = htons(TestPacket->TCPHeader->SourcePort);
			nTemp.dstip = htonl(TestPacket->IPHeader->DestinationAddress);
			nTemp.dstport = htons(TestPacket->TCPHeader->DestinationPort);
			if (bodylen >= 2)
			{
				pbody += 2;
				bodylen -= 2;
			}
		}
		else
		{
			delete(TestPacket);
			continue;
		}
		if ((mSessions.mCachePackets.mFilterInfo.isDstPortCheck&&mSessions.mCachePackets.mFilterInfo.dstport != nTemp.dstport) &&
			(mSessions.mCachePackets.mFilterInfo.isDstPortCheck&&mSessions.mCachePackets.mFilterInfo.dstport != nTemp.srcport))
		{
			delete(TestPacket);
			continue;
		}
		ForInterfacePCPacket(pbody, bodylen, nTemp, mTemp, mSessions.NoFlag);
		delete(TestPacket);
	}
	return result;
}

bool CAnalysisCap::ForInterfacePCPacket(unsigned char *pbody, unsigned int bodylen, CNetInfo n, MacInfo _mac,unsigned int num)
{
	// 去除无数据包
	if (bodylen <0x10) return false;

	//过滤腾讯数据包
	if (pbody[0] != 0x02 || pbody[bodylen - 1] != 0x03) return false;
	if (pbody[1] == 0x37 /*&& pbody[2] == 0x33*/)
	{
		//日志
		//PacketHeadReport(num, bodylen, HexToStr(pbody + 1, 2), HexToStr(pbody + 3, 2), HexToStr(pbody + 7, 4), "");

		std::list<CSyncPacketElement> *p = mSessions.mCachePackets.FindPacket(n);
		if (p != NULL)
		{
			CSyncPacketElement mSyncPacket;
			mSyncPacket.inPacketsNo = num;
			mSyncPacket.isStreamEnd = true;
			mSyncPacket._payload.append(pbody, bodylen);
			p->push_back(mSyncPacket);
		}
		else
		{
			CSyncPackets m;
			CSyncPacketElement mSyncPacket;
			mSyncPacket.inPacketsNo = num;
			mSyncPacket.isStreamEnd = true;
			mSyncPacket._payload.append(pbody, bodylen);
			m.mNetInfo = n;
			m.mMacInfo = _mac;
			m.mPackets.push_back(mSyncPacket);
			mSessions.mCachePackets.mCachePackets.push_back(m);
		}
	}
	return true;
}

bool CAnalysisCap::AnalysisPCPacket(CCachePackets &thePackets, CSessions &theSessions)
{
	if (thePackets.mCachePackets.size() < 2)
	{
		AfxMessageBox("没有找到相关数据包");
		return false;
	}
	//合并session
	std::list<CSyncPackets>::iterator itr1 = thePackets.mCachePackets.begin();
	for (; itr1 != thePackets.mCachePackets.end(); itr1++)
	{
		std::list<CSyncPackets>::iterator itr2 = itr1;
		itr2++;
		for (; itr2 != thePackets.mCachePackets.end(); itr2++)
		{
			if (itr1->mNetInfo.isSameStream(itr2->mNetInfo))
			{
				//一条流，合并
				CSessionElement mSession(Enum_PC);
				mSession.mPackets.mNetInfo = itr1->mNetInfo;
				mSession.mPackets.mMacInfo = itr1->mMacInfo;
				std::list<CSyncPacketElement>::iterator itr3 = itr1->mPackets.begin();
				std::list<CSyncPacketElement>::iterator itr4 = itr2->mPackets.begin();
				for (; itr3 != itr1->mPackets.end(); itr3++)
				{
					mSession.AddPacket(*itr3);
				}
				for (; itr4 != itr2->mPackets.end(); itr4++)
				{
					mSession.AddPacket(*itr4);
				}
				mSession.mPackets.mPackets.sort();
				theSessions.AddSession(mSession);
			}
		}
	}
	std::list<CSessionElement>::iterator itorsession = mSessions.mSessions.begin();
	for (; itorsession != mSessions.mSessions.end(); itorsession++)
	{
		std::list<CSyncPacketElement>::iterator itorpacket = itorsession->mPackets.mPackets.begin();
		for (; itorpacket != itorsession->mPackets.mPackets.end(); itorpacket++)
		{
			unsigned char *pbody = (unsigned char *)itorpacket->_payload.contents();
			unsigned int bodylen = itorpacket->_payload.size();
			ForPCPacket(pbody, bodylen, *itorpacket, *itorsession);
		}
	}
	return true;
}

bool CAnalysisCap::ForPCPacket(unsigned char *pbody, const unsigned int bodylen, CSyncPacketElement& mSyncPacket, CSessionElement& mSession)
{
	if (*(pbody + 0xb) != 0)
	{
		//发送
		if (*(pbody + 0xb) == 0x2)
		{
			mSyncPacket.client2server = true;
			mSyncPacket.ciphertextoffset = 0x16;
			mSyncPacket.ciphertextlen = bodylen - mSyncPacket.ciphertextoffset - 1;
		}
		else if (*(pbody + 0xb) == 0x3)
		{
			mSyncPacket.client2server = true;
			mSyncPacket.ciphertextoffset = 0x1A;
			mSyncPacket.ciphertextlen = bodylen - mSyncPacket.ciphertextoffset - 1;
		}
		else if (*(pbody + 0xb) == 0x4)
		{
			mSyncPacket.client2server = true;
			mSyncPacket.ciphertextoffset = 0x1E;
			mSyncPacket.ciphertextlen = bodylen - mSyncPacket.ciphertextoffset - 1;
		}
		else
		{
			logger.TraceError("new head type");
			AfxMessageBox("new head type");
		}
	}
	else
	{
		mSyncPacket.client2server = false;
		mSyncPacket.ciphertextoffset = 0xE;
		mSyncPacket.ciphertextlen = bodylen - mSyncPacket.ciphertextoffset - 1;
	}

	unsigned short cmdid = htons(*(short*)(pbody + 3));
	mSyncPacket.cmdid = cmdid;
	//取用户id
	unsigned int _id = htonl(*((int *)(pbody + 0x7)));
	mSyncPacket.id = _id;
	//特殊数据包需要修正ciphertextoffset与ciphertextlen
	if (cmdid == 0x0825)
	{
		if (mSyncPacket.client2server)
		{
			PacketDataReport("0825 send:",pbody, bodylen);
			mSession.pUserInfo->SetKey0825(pbody + mSyncPacket.ciphertextoffset, KEYLEN);
			unsigned char *dst = NULL;
			unsigned int bytes = 0;
			unEncrypt(pbody + mSyncPacket.ciphertextoffset + KEYLEN, mSyncPacket.ciphertextlen - KEYLEN, mSession.pUserInfo->GetKey0825(), &dst, &bytes);
			PacketDataReport("0825 send:", dst, bytes);
			free(dst);
		}
		else
		{
			PacketDataReport("0825 recv:", pbody, bodylen);
			unsigned char *dst = NULL;
			unsigned int bytes = 0;
			unEncrypt(pbody + mSyncPacket.ciphertextoffset, mSyncPacket.ciphertextlen, mSession.pUserInfo->GetKey0825(), &dst, &bytes);
			PacketDataReport("0825 recv:", dst, bytes);
			free(dst);
		}
	}
	else if (cmdid==0x0836)
	{
		if (mSyncPacket.client2server)
		{
			//日志
			PacketDataReport("0836 send:", pbody, bodylen);
			//取出真正登录的id用户，覆盖掉id
			mSession.pUserInfo->SetId(mSyncPacket.id);//在登录包中取出id为当前登录用户id
			//发送包
			unsigned int offset = mSyncPacket.ciphertextoffset;
			unsigned short wEcdhFlag = htons(*(short*)(pbody + offset));
			offset += sizeof(short);

			unsigned int type = htons(*(short*)(pbody + offset));
			if (type != 0x0103)
			{
				return false;
			}
			offset += 0x1D;
			unsigned int len = htonl(*(int*)(pbody + offset));
			if (len != 0x00000010)
			{
				return false;
			}
			offset += sizeof(int);
			//读取0836key
			mSession.pUserInfo->SetKey0836(pbody + offset, KEYLEN);
			offset += 0x10;

			mSyncPacket.ciphertextoffset = offset;
			mSyncPacket.ciphertextlen = bodylen - mSyncPacket.ciphertextoffset - 1;
			//解密
			unsigned char *dst = NULL;
			unsigned int bytes = 0;
			IdKeyUnion mKey;
			if (mUserKeys.isFindValid(mSyncPacket.id, mSession.pUserInfo->GetClientType()))
				mKey = *mUserKeys.GetItemValid(mSyncPacket.id, mSession.pUserInfo->GetClientType());
			unEncrypt(pbody + mSyncPacket.ciphertextoffset, mSyncPacket.ciphertextlen, (unsigned char*)mKey.ShareKey.c_str(), &dst, &bytes);
			//解析dst
			if (dst!= NULL)
			{
				//日志
				PacketDataReport("0836 send:", dst, bytes);
				ForTLV(dst, bytes, mSyncPacket, mSession);
				free(dst);
				dst = NULL;
			}
		}
		else
		{
			//日志
			PacketDataReport("0836 recv:", pbody, bodylen);

			int declen = bodylen;
			//解密
			unsigned char *dst = NULL;
			unsigned int bytes = 0;
			IdKeyUnion mKey;
			if (mUserKeys.isFindValid(mSyncPacket.id, mSession.pUserInfo->GetClientType()))
				mKey = *mUserKeys.GetItemValid(mSyncPacket.id, mSession.pUserInfo->GetClientType());
			unEncrypt(pbody + mSyncPacket.ciphertextoffset, mSyncPacket.ciphertextlen, (unsigned char*)mKey.ShareKey.c_str(), &dst, &bytes);

			//日志
			PacketDataReport("0836 recv-ecdh:", dst, bytes);
			if (bytes % 8 != 0)
			{
				free(dst);
				dst = NULL;
			}
			else
			{
				unsigned char *dst2 = NULL;
				unsigned int bytes2 = 0;
				unEncrypt(dst, bytes, mSession.pUserInfo->GetTlv0006(), &dst2, &bytes2);
				free(dst);
				dst = NULL;

				PacketDataReport("0836 recv-vague:", dst2, bytes2);

				//解析key0836ret1，key0836ret2
				if (*dst2 == 0)
					ForTLV(dst2 + 1, bytes2 - 1, mSyncPacket, mSession);
				else
					ForTLV(dst2, bytes2, mSyncPacket, mSession);
				free(dst2);
				dst2 = NULL;

			}
		}
	}else if (cmdid == 0x0828)
	{

		if (mSyncPacket.client2server)
		{
			//日志
			PacketDataReport("0828 send:", pbody, bodylen);
			unsigned int offset = mSyncPacket.ciphertextoffset;
			unsigned int type = htons(*(short*)(pbody + offset));
			if (type != 0x0030)
			{
				return false;
			}
			offset += sizeof(short);
			offset += sizeof(short);
			offset += 0x3A;
			mSyncPacket.ciphertextoffset = offset;
			mSyncPacket.ciphertextlen = bodylen - mSyncPacket.ciphertextoffset-1;
			//0828数据包解密
			unsigned char *dst = NULL;
			unsigned int bytes = 0;
			unEncrypt(pbody + mSyncPacket.ciphertextoffset, mSyncPacket.ciphertextlen, mSession.pUserInfo->GetTlv0109(), &dst, &bytes);
			
			PacketDataReport("0828 send:", dst, bytes);
			
			free(dst);
			dst = NULL;
		}
		else
		{
			//日志
			PacketDataReport("0828 recv:", pbody, bodylen);
			
			int declen = bodylen;
			//0828数据包解密
			unsigned char *dst = NULL;
			unsigned int bytes = 0;
			unEncrypt(pbody + mSyncPacket.ciphertextoffset, mSyncPacket.ciphertextlen, mSession.pUserInfo->GetTlv0107(), &dst, &bytes);

			PacketDataReport("0828 recv:", dst, bytes);

			//解析SessionKey
			if (*dst == 0)
				ForTLV(dst + 1, bytes - 1, mSyncPacket, mSession);
			else
				ForTLV(dst, bytes, mSyncPacket, mSession);
			free(dst);
			dst = NULL;
		}
	}
	//取出id
	mSession.pUserInfo->SetId(mSyncPacket.id);
	return true;
}

bool CAnalysisCap::ScanCapMobile(pcap_t* fp)
{
	//先清空数据
	ReleaseSession();

	bool result = true;
	int res = 0;
	struct pcap_pkthdr *header;
	const u_char *pkt_data;
	while ((res = pcap_next_ex(fp, &header, &pkt_data)) >= 0)
	{
		mSessions.NoFlag++;
		cPacket* TestPacket = new cPacket((UCHAR*)pkt_data, header->len);
		unsigned char *pbody = NULL;
		unsigned int bodylen = 0;
		CNetInfo nTemp;
		if (TestPacket->isUDPPacket)
		{
			
			pbody = TestPacket->UDPData;
			bodylen = TestPacket->UDPDataSize;
			nTemp.proto = TestPacket->IPHeader->Protocol;
			nTemp.srcip = htonl(TestPacket->IPHeader->SourceAddress);
			nTemp.srcport = htons(TestPacket->UDPHeader->SourcePort);
			nTemp.dstip = htonl(TestPacket->IPHeader->DestinationAddress);
			nTemp.dstport = htons(TestPacket->UDPHeader->DestinationPort);
		}
		else if (TestPacket->isTCPPacket)
		{
			pbody = TestPacket->TCPData;
			bodylen = TestPacket->TCPDataSize;
			nTemp.proto = TestPacket->IPHeader->Protocol;
			nTemp.srcip = htonl(TestPacket->IPHeader->SourceAddress);
			nTemp.srcport = htons(TestPacket->TCPHeader->SourcePort);
			nTemp.dstip = htonl(TestPacket->IPHeader->DestinationAddress);
			nTemp.dstport = htons(TestPacket->TCPHeader->DestinationPort);
		}
		else
		{
			delete(TestPacket);
			continue;
		}
		if ((mSessions.mCachePackets.mFilterInfo.isDstPortCheck&&mSessions.mCachePackets.mFilterInfo.dstport != nTemp.dstport)&&
			(mSessions.mCachePackets.mFilterInfo.isDstPortCheck&&mSessions.mCachePackets.mFilterInfo.dstport != nTemp.srcport))
		{
			delete(TestPacket);
			continue;
		}
		EnterMBPacket(pbody, bodylen, nTemp, mSessions.NoFlag);
		delete(TestPacket);
	}
	return result;
}

bool CAnalysisCap::EnterMBPacket(unsigned char *pbody, unsigned int bodylen, CNetInfo n, unsigned int num, bool isRealMyPacket)
{
	bool result = false;
	if ((bodylen >= 0x14 && pbody[4] == 0 && pbody[5] == 0 && pbody[6] == 0 && (pbody[7] >= 0x8 && pbody[7] <= 0xF) && (pbody[8] >= 0x0 && pbody[8] <= 0xF ))
		|| (bodylen >= 0x14 && pbody[4] == 0x1 && pbody[5] == 0x33 && pbody[6] == 0x52 && pbody[7] == 0x39
		&& pbody[0xC] == 0x4 && pbody[0xD] == 0x4d && pbody[0xE] == 0x53 && pbody[0xF] == 0x46))
	{
		//选择腾讯数据包并对一个数据包进行封装
		ForInterfaceMBData(pbody, bodylen, n, num);
	}
	else
	{
		std::list<CSyncPacketElement> *p = mSessions.mCachePackets.FindPacket(n);
		if (p != NULL)
		{
			if (p->back().isStreamEnd != false)
			{
				if (isRealMyPacket)
				{
					//之前的数据包已经结束了,但是此数据包确实为腾讯数据包
					int len = htonl(*(int*)pbody);
					ForInterfaceMBData(pbody, bodylen, n, num);
					return true;
				}
				else
				{
					//重传的或者错误数据包
					return true;
				}
			}
			if (bodylen < p->back().streamResiduelen)
			{
				p->back()._payload.append(pbody, bodylen);
				p->back().isStreamEnd = false;
				p->back().streamResiduelen -= bodylen;
			}
			else if (bodylen == p->back().streamResiduelen)
			{
				p->back()._payload.append(pbody, bodylen);
				p->back().isStreamEnd = true;
			}
			else if (bodylen > p->back().streamResiduelen)
			{
				p->back()._payload.append(pbody, p->back().streamResiduelen);
				p->back().isStreamEnd = true;
				//新的数据包
				EnterMBPacket(pbody + p->back().streamResiduelen, bodylen - p->back().streamResiduelen, n, num);
			}
		}
	}
	return result;
}

bool CAnalysisCap::ForInterfaceMBData(unsigned char *pbody, unsigned int bodylen, CNetInfo n, unsigned int num)
{
		int len = htonl(*(int*)pbody);
		if (len == bodylen)
		{
			std::list<CSyncPacketElement> *p = mSessions.mCachePackets.FindPacket(n);
			if (p != NULL)
			{
				CSyncPacketElement mSyncPacket;
				mSyncPacket.inPacketsNo = num;
				mSyncPacket.isStreamEnd = true;
				mSyncPacket._payload.append(pbody, bodylen);
				p->push_back(mSyncPacket);
			}
			else
			{
				CSyncPackets m;
				CSyncPacketElement mSyncPacket;
				mSyncPacket.inPacketsNo = num;
				mSyncPacket.isStreamEnd = true;
				mSyncPacket._payload.append(pbody, bodylen);
				m.mNetInfo = n;
				m.mPackets.push_back(mSyncPacket);
				mSessions.mCachePackets.mCachePackets.push_back(m);
			}
		}
		else if (len < bodylen)
		{
			//五元组是否已在session中
			CombinPacket(pbody, bodylen, mSessions.mCachePackets, n, num);
		}
		else
		{
			//合并包
			std::list<CSyncPacketElement> *p = mSessions.mCachePackets.FindPacket(n);
			if (p != NULL)
			{
				CSyncPacketElement mSyncPacket;
				mSyncPacket.inPacketsNo = num;
				mSyncPacket.isStreamEnd = false;
				mSyncPacket.streamResiduelen = len - bodylen;
				mSyncPacket._payload.append(pbody, bodylen);
				p->push_back(mSyncPacket);
			}
			else
			{
				CSyncPackets m;
				CSyncPacketElement mSyncPacket;
				mSyncPacket.inPacketsNo = num;
				mSyncPacket.isStreamEnd = false;
				mSyncPacket.streamResiduelen = len - bodylen;
				mSyncPacket._payload.append(pbody, bodylen);
				m.mNetInfo = n;
				m.mPackets.push_back(mSyncPacket);
				mSessions.mCachePackets.mCachePackets.push_back(m);
			}
		}

	return true;
}

bool CAnalysisCap::AnalysisMBPacket(CCachePackets &thePackets, CSessions &theSessions)
{
	if (thePackets.mCachePackets.size() < 2)
	{
		AfxMessageBox("没有找到相关数据包");
		return false;
	}
	//合并session
	std::list<CSyncPackets>::iterator itr1 = thePackets.mCachePackets.begin();
	for (; itr1 != thePackets.mCachePackets.end(); itr1++)
	{
		std::list<CSyncPackets>::iterator itr2 = itr1;
		itr2++;
		for (; itr2 != thePackets.mCachePackets.end(); itr2++)
		{
			if (itr1->mNetInfo.isSameStream(itr2->mNetInfo))
			{
				//一条流，合并
				CSessionElement mSession(Enum_Android);
				mSession.mPackets.mNetInfo = itr1->mNetInfo;
				mSession.mPackets.mMacInfo = itr1->mMacInfo;
				std::list<CSyncPacketElement>::iterator itr3 = itr1->mPackets.begin();
				std::list<CSyncPacketElement>::iterator itr4 = itr2->mPackets.begin();
				for (; itr3 != itr1->mPackets.end(); itr3++)
				{
					mSession.AddPacket(*itr3);
				}
				for (; itr4 != itr2->mPackets.end(); itr4++)
				{
					mSession.AddPacket(*itr4);
				}
				mSession.mPackets.mPackets.sort();
				theSessions.AddSession(mSession);
			}
		}
	}

	std::list<CSessionElement>::iterator itorsession = mSessions.mSessions.begin();
	for (; itorsession != mSessions.mSessions.end(); itorsession++)
	{
		std::list<CSyncPacketElement>::iterator itorpacket = itorsession->mPackets.mPackets.begin();
		for (; itorpacket != itorsession->mPackets.mPackets.end(); itorpacket++)
		{
			unsigned char *pbody = (unsigned char *)itorpacket->_payload.contents();
			unsigned int bodylen = itorpacket->_payload.size();
			ForMBPacket(pbody, bodylen, *itorpacket, *itorsession);
		}
	}
	return true;
}

bool CAnalysisCap::ForMBPacket(unsigned char *pbody, unsigned int bodylen, CSyncPacketElement& mSyncPacket, CSessionElement& mSession)
{
	//先判断MSF数据包
	if (pbody[0xC] == 0x4 && pbody[0xD] == 0x4d && pbody[0xE] == 0x53 && pbody[0xF] == 0x46)
	{
		//MSF数据包
		mSyncPacket.serviceCmd = "MSF";
		mSyncPacket.id = htonl(*(int*)(pbody + 8));
		mSyncPacket.ciphertype = 0;
		mSyncPacket.ciphertextlen = 0;
		mSyncPacket.ciphertextoffset = 0;
		mSyncPacket.client2server = true;//此处bug，无法区分MSF发送与接收
		return true;
	}

	//设置加密类型
	mSyncPacket.ciphertype = pbody[0x8];

	//取出四字节，判断
	int value = htonl(*(int*)(pbody + 9));
	int offset = 9;
	if (value == 0x44)
	{
		mSyncPacket.client2server = true;
		offset += 0x44;
	}
	else if ((value&0x00FFFFFF)	==	0x0)
	{
		mSyncPacket.client2server = false;
	}
	else
	{
		mSyncPacket.client2server = true;
		offset += 4;
	}

	//取出QQ号码
	if (pbody[offset] == 0)
	{
		offset += 1;
		int len = htonl(*(int*)(pbody + offset));
		if (len == 5)
		{
			offset += 5;
			mSyncPacket.id = 0;
		}
		else
		{
			offset += 4;
			len -= 4;
			//取出QQ号码
			unsigned char _id[USERSIZE] = { 0 };
			memcpy(_id, pbody + offset, len);
			mSyncPacket.id = strtoul((char*)_id, 0, 10);
			mSession.pUserInfo->SetId(mSyncPacket.id);
			offset += len;
		}
	}
	else
	{
		offset += 1;
		int len = htonl(*(int*)(pbody + offset));
		if (len == 5)
		{
			offset += 5;
			mSyncPacket.id = 0;
		}
		else
		{
			offset += 4;
			len -= 4;
			//取出QQ号码
			unsigned char _id[USERSIZE] = { 0 };
			memcpy(_id, pbody + offset, len);
			mSyncPacket.id = strtoul((char*)_id, 0, 10);
			mSession.pUserInfo->SetId(mSyncPacket.id);
			offset += len;
		}
	}

	//先排除明文数据包
	if (mSyncPacket.ciphertype == 0)
	{
		//明文数据包，目前只发现Heartbeat.Alive

		//取出serviceCmd
		unsigned int inoffset = 0;
		if (mSyncPacket.client2server)
			inoffset=offset + 0x20;
		else
			inoffset = offset + 0x10;
		unsigned int size = htonl(*(int*)(pbody + inoffset));
		inoffset += 4;
		std::string str;
		char *buf = (char*)malloc(size);
		memcpy(buf, pbody + inoffset, size);
		mSyncPacket.serviceCmd.append(buf, size);
		free(buf);

		mSyncPacket.ciphertextlen = 0;
		mSyncPacket.ciphertextoffset = 0;
		return true;
	}

	//开始解密
	int declen = bodylen - offset;
	mSyncPacket.ciphertextlen = declen;
	mSyncPacket.ciphertextoffset = offset;

	if (mSyncPacket.isStreamEnd == false)
	{
		//数据包不完整,不进行解密相关处理
		return true;
	}


	unsigned char *dst = NULL;
	unsigned int bytes = 0;
	if (mSyncPacket.ciphertype == 1)
	{
		//SessionKey
		UserKey mSessionKey;
		if (mSession.pUserInfo->KeysUinion.find(mSyncPacket.id) != mSession.pUserInfo->KeysUinion.end())
			mSessionKey = mSession.pUserInfo->KeysUinion.find(mSyncPacket.id)->second;
		unEncrypt(pbody + offset, declen, mSessionKey.SessionKey, &dst, &bytes);
	}
	else if (mSyncPacket.ciphertype == 2)
	{
		//全0解密
		unEncrypt(pbody + offset, declen, mSession.pUserInfo->Key0, &dst, &bytes);
	}
	else
	{
		AfxMessageBox("new ciphertype");
		unEncrypt(pbody + offset, declen, mSession.pUserInfo->Key0, &dst, &bytes);
	}
	//解析dst
	ForJCE(dst, bytes, mSyncPacket,mSession);
	free(dst);
	dst = NULL;

	return true;
}

bool CAnalysisCap::CombinPacket(unsigned char *pbody, unsigned int bodylen, CCachePackets& m, CNetInfo n,unsigned int num)
{
	//len<bodylen
	std::list<CSyncPacketElement> *p = mSessions.mCachePackets.FindPacket(n);
	if (p != NULL)
	{
		int len = htonl(*(int*)pbody);
		CSyncPacketElement mSyncPacket;
		mSyncPacket.inPacketsNo = num;
		mSyncPacket.isStreamEnd = true;
		mSyncPacket._payload.append(pbody, len);
		EnterMBPacket(pbody + len, bodylen - len, n, num, true);
	}
	else
	{
		int len = htonl(*(int*)pbody);
		CSyncPackets m;
		CSyncPacketElement mSyncPacket;
		mSyncPacket.inPacketsNo = num;
		mSyncPacket.isStreamEnd = true;
		mSyncPacket.streamResiduelen = 0;
		mSyncPacket._payload.append(pbody, len);
		m.mNetInfo = n;
		m.mPackets.push_back(mSyncPacket);
		mSessions.mCachePackets.mCachePackets.push_back(m);
		EnterMBPacket(pbody + len, bodylen - len, n, num, true);
	}
	return true;
}

bool CAnalysisCap::ForTLV(unsigned char *data, unsigned int len, CSyncPacketElement mSyncPacket, CSessionElement &mSession)
{
	unsigned char *p = data;
	int offset = 0;
	do
	{
		unsigned short cmd = htons(*(short*)p);
		p += sizeof(short);
		switch (cmd)
		{
			case 0x0006:
			{
				unsigned short size = htons(*(short*)p);
				p += sizeof(short);
				//解密vague_key
				unsigned char *dst = NULL;
				unsigned int bytes = 0;
				std::string key;
				if (mUserKeys.isFindValid(mSyncPacket.id, mSession.pUserInfo->GetClientType()))
					key = mUserKeys.GetItemValid(mSyncPacket.id, mSession.pUserInfo->GetClientType())->PwdHash;
				unEncrypt(p, size, (unsigned char *)key.c_str(), &dst, &bytes);
				PacketDataReport("TLV0006:", dst, bytes);
				//取出vague_key
				mSession.pUserInfo->SetTlv0006(dst + bytes - KEYLEN, KEYLEN);
				free(dst);
				dst = NULL;
				p += size;
			}break;
			case 0x0109:
			{
				unsigned short size = htons(*(short*)p);
				p += sizeof(short);
				//取出key0836ret1
				mSession.pUserInfo->SetTlv0109(p + 0x2, KEYLEN);
				p += size;
			}break;
			case 0x0107:
			{
				unsigned short size = htons(*(short*)p);
				p += sizeof(short);
				//取出key0836ret2
				mSession.pUserInfo->SetTlv0107(p + 0x1a, KEYLEN);
				p += size;
			}break;
			case 0x010c:
			{
				unsigned short size = htons(*(short*)p);
				p += sizeof(short);
				mSession.pUserInfo->SetTlv010C(mSyncPacket.id, p, KEYLEN);
				p += size;
			}break;
			case 0x0106:
			{
				unsigned short size = htons(*(short*)p);
				p += sizeof(short);
				//解密_tgtgt_key
				unsigned char *dst = NULL;
				unsigned int bytes = 0;
				std::string key;
				if (mUserKeys.isFindValid(mSyncPacket.id, mSession.pUserInfo->GetClientType()))
					key = mUserKeys.GetItemValid(mSyncPacket.id, mSession.pUserInfo->GetClientType())->PwdHash;
				unEncrypt(p, size, (unsigned char *)key.c_str(), &dst, &bytes);
				PacketDataReport("TLV0106:", dst, bytes);
				//取出_tgtgt_key
				memcpy(mSession.pUserInfo->_tgtgt_key, dst + 0x33, KEYLEN);
				free(dst);
				dst = NULL;
				p += size;
			}break;
			case 0x0119:
			{
				unsigned short size = htons(*(short*)p);
				p += sizeof(short);
				unsigned char *dst = NULL;
				unsigned int bytes = 0;
				unEncrypt(p, size, mSession.pUserInfo->_tgtgt_key, &dst, &bytes);
				PacketDataReport("TLV0119:", dst, bytes);
				ForTLV(dst + 0x2, bytes - 0x2, mSyncPacket, mSession);
				free(dst);
				dst = NULL;
				p += size;
			}break;
			case 0x0305:
			{
				//移动版取出SessionKey
				unsigned short size = htons(*(short*)p);
				p += sizeof(short);
				UserKey key;
				memcpy(key.SessionKey, p, KEYLEN);
				mSession.pUserInfo->KeysUinion.insert(std::pair<unsigned int, UserKey>(mSyncPacket.id, key));
				p += size;
				mSession.pUserInfo->isCanDecrypt = true;
			}break;
			default:
			{
				unsigned short size = htons(*(short*)p);
				p += sizeof(short);
				p += size;
			}break;
		}
	} while (p-data<len);
	return true;
}

bool CAnalysisCap::ForJCE(unsigned char *data, unsigned int len, CSyncPacketElement &mSyncPacket, CSessionElement &mSession)
{
	unsigned char *p = data;
	int offset = 0;
	int size = htonl(*(int*)data);
	unsigned int _firstsize = size;
	offset += 4;
	if (size < len)
	{
		//多个包
		int seq = htonl(*(int*)(data+offset));
		//seq id判断
		if ((seq&0xFFFFFF00)==0)
		{
			//This is One shit data,fucking this programer!
			size = htonl(*(int*)(data + offset));
			offset += 4;
			size -= 4;
			//取出serviceCmd
			char *buf = (char*)malloc(size);
			memcpy(buf, data + offset, size);
			mSyncPacket.serviceCmd.append(buf, size);
			free(buf);
			offset += size;
		}
		else
		{
			//seq id
			offset += 4;
			if (mSyncPacket.client2server)
				offset += 0x10;
			offset += 4;
			size = htonl(*(int*)(data + offset));
			offset += size;
			size = htonl(*(int*)(data + offset));
			offset += 4;
			size -= 4;
			//取出serviceCmd
			char *buf = (char*)malloc(size);
			memcpy(buf, data + offset, size);
			mSyncPacket.serviceCmd.append(buf, size);
			free(buf);
			offset += size;

		}
		offset = _firstsize;
		if (mSyncPacket.serviceCmd == "wtlogin.login")
		{
			//取出真正登录的id用户，覆盖掉id
			mSession.pUserInfo->SetId(mSyncPacket.id);//在登录包中取出id为当前登录用户id

			size = htonl(*(int*)(data + offset));
			if (size != len - offset) return false;
			offset += 4;
			if (*(data + offset) != 0x02 && *(data + offset + size) != 0x3) return false;
			int declen = 0;
			if (mSyncPacket.client2server)
			{
				offset += 0x4b;
				declen = size - 0x4 - 0x4b - 0x1;
			}
			else
			{
				offset += 0x10;
				declen = size - 0x4 - 0x10 - 0x1;
			}
			//sharekey解密
			unsigned char *dst = NULL;
			unsigned int bytes = 0;
			IdKeyUnion mKey;
			if (mUserKeys.isFindValid(mSyncPacket.id, mSession.pUserInfo->GetClientType()))
				mKey = *mUserKeys.GetItemValid(mSyncPacket.id, mSession.pUserInfo->GetClientType());
			unEncrypt(data + offset, declen, (unsigned char*)mKey.ShareKey.c_str(), &dst, &bytes);
			
			if (mSyncPacket.client2server)
			{
				if (*(dst) == 0)
				{
					ForTLV(dst + 0x4, bytes - 0x4, mSyncPacket, mSession);
				}
				else
				{
					mSession.pUserInfo->isCanDecrypt = false;
					return false;
				}

			}
			else
			{
				//使用tgtgt_key解密
				if (*(dst + 2) == 0)
				{
					ForTLV(dst + 0x5, bytes - 0x5, mSyncPacket, mSession);
				}
				else
				{
					mSession.pUserInfo->isCanDecrypt = false;
					return false;
				}
			}
			free(dst);
			dst = NULL;
		}
	}
	return true;
}

bool CAnalysisCap::Init(char *msg)
{
	return true;
}

bool CAnalysisCap::LoadAllDevs(NetCardInfos& devlist)
{
	Packetyzer::Capture::cWinpcapCapture capture;
	if (capture.isReady)
	{
		for (UINT i = 0; i< capture.nAdapters; i++)
		{
			NetCardInfo n;
			n.description = capture.Adapters[i].Name;
			n.name = capture.Adapters[i].ID;
			devlist.push_back(n);
		}

		return true;
	}
	//char errbuf[PCAP_ERRBUF_SIZE];
	//int i = 0;
	//pcap_if_t *d = NULL;
	//if (pcap_findalldevs(&g_tbNetCardInfo, errbuf) == -1)//返回网卡列表，alldevs指向表头
	//{
	//	AfxMessageBox("Error in pcap_findalldevs");
	//	return false;
	//}
	//for (d = g_tbNetCardInfo; d; d = d->next)
	//{
	//	if (d->description)
	//	{
	//		NetCardInfo n;
	//		n.description = d->description;
	//		n.name = d->name;
	//		devlist.push_back(n);
	//	}
	//}
	return false;
}

bool CAnalysisCap::Freealldevs()
{
	return true;
}

unsigned int CAnalysisCap::unEncrypt(unsigned char *s, int len, unsigned char *key, unsigned char **dst, unsigned int *bytes)
{
	CTeaCrypt mTea;
	mTea.InitTeaOne();
	unsigned int ret = mTea.Tea_decrypt(s, len, key, dst, bytes);
	return ret;
}

void CAnalysisCap::SetReport(bool b)
{
	isReport = b;
}

void CAnalysisCap::PacketHeadReport(unsigned int offset, unsigned int pktlen, std::string wMainVersion, std::string type, std::string qq, std::string dwPubNo)
{
	if (isReport)
		logger.TraceInfo("pkt number:%d,wMainVersion:%s,type:%s,qq:%s,dwPubNo:%s", offset, wMainVersion.c_str(), type.c_str(), qq.c_str(), dwPubNo.c_str());
}

void CAnalysisCap::PacketDataReport(std::string strInfo, unsigned char* data, unsigned int len)
{
	if (isReport)
		logger.TraceData(strInfo.c_str(), HexString(data, len, true, true, false).c_str(), 0);
}