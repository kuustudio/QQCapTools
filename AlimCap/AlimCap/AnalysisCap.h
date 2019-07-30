#pragma once
#include"packet.h"
#include"pcap.h"
#pragma comment(lib,"wpcap.lib")
//#include"libnet.h"
//#pragma comment(lib,"libnet.lib")

#include"Sessions.h"
#include"SnifferInfo.h"
#include"CachePackets.h"
#include"NetInfo.h"
#include"SyncPacketElement.h"
#include"IdKeys.h"
#include"logger.h"

// CAnalysisCap

class CAnalysisCap
{

public:
	CAnalysisCap();
	virtual ~CAnalysisCap();
public:
	CSessions mSessions;//用户信息
	NetCardInfos mNetCardList;//网卡列表
	CSnifferInfo mSniffer;
	IdKeys mUserKeys;//主要为本地缓存的密码信息
private:
	bool isReport;//是否生成分析报告
	LOGGER::CLogger logger;
public:
	//pcap数据包操作相关函数
	void SetFilter(pcap_t *,char *);
	void ReleaseFilter(pcap_t *);
	pcap_t *CapOpen(const char *);
	void CapClose(pcap_t *);
	void ReleaseSession();
	static void packet_handler(LPVOID param,const u_char *pkt_data, unsigned int len);
	static UINT snifferThreadFunc(LPVOID pParam);
	//pcap监听
	bool StartOpenSniffer(const char *,HWND msgHandle);
	void StopOpenSniffer();
public:
	bool Init(char *File);
	//加载网卡设备
	bool LoadAllDevs(NetCardInfos& devlist);
	bool Freealldevs();
	//moblie
	bool LoginCapMobile(pcap_t *);
	bool ScanCapMobile(pcap_t*);
	//isRealMyPacket表示确实此数据包为腾讯QQ数据包，比如由CombinPacket调用的，整个包开始已经经过了ForInterfaceMBData的判断
	bool EnterMBPacket(unsigned char *data, unsigned int size, CNetInfo n, unsigned int, bool isRealMyPacket = false);
	bool ForInterfaceMBData(unsigned char *data, unsigned int size,CNetInfo n,unsigned int);
	bool AnalysisMBPacket(CCachePackets&, CSessions &);
	bool ForTLV(unsigned char *data, unsigned int size, CSyncPacketElement, CSessionElement&);
	bool ForJCE(unsigned char *data, unsigned int size, CSyncPacketElement&, CSessionElement&);
	bool ForMBPacket(unsigned char *data, unsigned int size, CSyncPacketElement&, CSessionElement&);
	bool CombinPacket(unsigned char *data, unsigned int size, CCachePackets&,CNetInfo,unsigned int);
	//PC
	bool ScanPCCap(pcap_t *);
	bool ForInterfacePCPacket(unsigned char *data, unsigned int size, CNetInfo n, MacInfo m, unsigned int);
	bool AnalysisPCPacket(CCachePackets&, CSessions &);
	bool ForPCPacket(unsigned char *data, const unsigned int size, CSyncPacketElement&, CSessionElement&);
	//report函数
	void SetReport(bool b);
	void PacketHeadReport(unsigned int offset, unsigned int pktlen, std::string wMainVersion, std::string cmdid, std::string qq, std::string dwPubNo);
	void PacketDataReport(std::string strInfo, unsigned char* data,unsigned int len);
public:
	static bool GetMd5(IN ByteBuffer a,OUT ByteBuffer &b);
	static std::string HexToStr(BYTE *pbDest, int nLen);
	static std::string StrToHex(BYTE *pbDest, int nLen);
	static std::string HexString(BYTE *pbDest, int nLen,bool isSpace,bool isChangeLine,bool isAssic);
	static unsigned int unEncrypt(unsigned char *s, int len, unsigned char *key, unsigned char **dst, unsigned int *bytes);
};


