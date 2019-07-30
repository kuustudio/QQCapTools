#pragma once
class CNetInfo
{
public:

	CNetInfo()
	{
	}

	virtual ~CNetInfo()
	{
	}
public:
	unsigned int	srcip;
	unsigned short	srcport;
	unsigned int	dstip;
	unsigned short	dstport;
	unsigned int proto;
public:
	bool operator==(const CNetInfo &t1)const{
		return (srcip == t1.srcip&&srcport == t1.srcport&&
			dstip == t1.dstip&&dstport == t1.dstport&&proto == t1.proto) /*|| (srcip == t1.dstip&&srcport == t1.dstport&&
																		 dstip == t1.srcip&&dstport == t1.srcport&&proto == t1.proto)*/;
	}
	bool operator!=(const CNetInfo &t1)const{
		return (srcip == t1.srcip || srcport == t1.srcport ||
			dstip == t1.dstip || dstport == t1.dstport || proto == t1.proto);
	}
public:
	bool isSameStream(const CNetInfo &t1)
	{
		return (srcip == t1.srcip&&srcport == t1.srcport&&
			dstip == t1.dstip&&dstport == t1.dstport&&proto == t1.proto) || (srcip == t1.dstip&&srcport == t1.dstport&&
			dstip == t1.srcip&&dstport == t1.srcport&&proto == t1.proto);
	}
};

