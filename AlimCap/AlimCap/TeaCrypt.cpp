#include<WinSock2.h>
#include "TeaCrypt.h"
#include<stdlib.h>
#include<math.h>

#pragma comment(lib,"Ws2_32.lib")


CTeaCrypt::CTeaCrypt(void):dwTeaRound(0),dwDelta(0)
{
}
CTeaCrypt::~CTeaCrypt(void)
{
}

void CTeaCrypt::InitTeaOne()
{
	dwTeaRound=TEA_ROUNDS_ONE;
	dwDelta=TEA_DELTA_ONE;
}

void CTeaCrypt::InitTeaTwo()
{
	dwTeaRound=TEA_ROUNDS_TWO;
	dwDelta=TEA_DELTA_TWO;
}


/*
*随机密钥初始化函数
*/
void CTeaCrypt::initkey(unsigned char *key,unsigned int len)
{
	memset(key,0,TEA_KEY_LEN);
	srand(0x5f4567);
	srand(GetTickCount()+rand());
	for(int i=0;i<len;i++)
	{
		int seed=rand();
		unsigned char r=seed;
		memcpy(key+i,&r,0x1);
	}
}

/*
*
*此加密为未知加密方法
*
*/
unsigned int CTeaCrypt::encrypt_vague(unsigned char *szBuff,unsigned int len)
{
	if(len<0x40)
	{
		return 0;
	}
	FILETIME ft={0};
	GetSystemTimeAsFileTime(&ft);
	ft.dwLowDateTime+=0x2AC18000;
	ft.dwHighDateTime+=0xFE624E21;
	DWORD dwLow=ft.dwLowDateTime;
	DWORD dwHigh=ft.dwHighDateTime;
	DWORD dwRet=0;
	__asm
	{
		pushad;
		mov ecx,0x989680;
		mov eax,dwHigh;
		xor edx,edx;
		div ecx;
		mov eax,dwLow;
		div ecx;
		mov dwRet,eax;
		mov edx,eax;
		xor edi,edi;
		mov eax,0x89be;
		xor ecx,ecx;
LableLoop:
		lea esi,dword ptr ds:[ecx+eax]
		imul eax,esi;
		inc ecx;
		cmp ecx,7;
		jb LableLoop;
		inc eax;
		imul eax,edx;
		imul eax,edx;
		//eax为结果
		//-897168924
		mov dwRet,eax;
		popad;
	}
	strcpy((LPSTR)szBuff,"  -");
	ZeroMemory(szBuff+3,15);//清0
	unsigned char *szNow=szBuff+3;
	while (dwRet!=0)
	{
		unsigned int dwYu=dwRet%0xa;
		*szNow=dwYu+'0';
		szNow++;
		dwRet=dwRet/0xa;
	}
	return 1;
}

/*
*加密函数
*/
void CTeaCrypt::encrypt_tea(unsigned long *in, unsigned long *key, unsigned long *out)
{
	unsigned long code[4];
	register unsigned long i = dwTeaRound, j = 0, m, n;

	m = htonl(in[0]);
	n = htonl(in[1]);
	code[0] = htonl(key[0]); code[1] = htonl(key[1]);
	code[2] = htonl(key[2]); code[3] = htonl(key[3]);
	while(i-->0) {
		j+=dwDelta;
		m += ((n>>5)+code[1]) ^ ((n<<4) +code[0]) ^ (j+n);
		n += ((m>>5)+code[3]) ^ ((m<<4) +code[2]) ^ (j+m);
	}
	out[0] = htonl(m);
	out[1] = htonl(n);
}
unsigned int CTeaCrypt::Tea_encrypt(unsigned char *in, int inlen, unsigned char *key, unsigned char *out, unsigned int *outlen)
{
	register int m, i, j, count, p = 1;
	unsigned char q[12], *q1, *q2, *inp;
	unsigned char mkey[8];

	m = (inlen+10)%8;
	if (m)  m = 8-m;
	q[0] = (rand()&0xf8) | m;
	i = j = 1;
	while(m>0) {
		q[i++] = rand()&0xff;
		m --;
	}
	count = *outlen = 0;
	q2 = q1 = out;
	memset(mkey, 0, sizeof(mkey));
	while( p <= 2 ) {
		if (i < 8) {
			q[i++] = rand()&0xff;
			p ++;
		}
		if (i == 8) {
			for (i = 0; i < 8; i ++)
				q[i] ^= mkey[i];
			encrypt_tea((unsigned long *)q, (unsigned long *)key, (unsigned long *)out);
			for (i = 0; i < 8; i ++)
				q1[i] ^= mkey[i];
			q2 = q1;
			q1 += 8;
			count += 8;
			memcpy(mkey, q, 8);
			j = i = 0;
		}
	}
	inp = in;
	while (inlen > 0) {
		if (i < 8) {
			q[i] = inp[0];
			inp ++;
			i ++;
			inlen --;
		}
		if (i == 8) {
			for (i = 0; i < 8; i ++)  {
				if (j) q[i] ^= mkey[i];
				else q[i] ^= q2[i];
			}
			j = 0;
			encrypt_tea((unsigned long *)q, (unsigned long *)key, (unsigned long *)q1);
			for (i = 0; i < 8; i ++)
				q1[i] ^= mkey[i];
			count += 8;
			memcpy(mkey, q, 8);
			q2 = q1;
			q1 += 8;
			i = 0;
		}
	}
	p = 1;
	while (p < 8) {
		if (i < 8) {
			memset(q+i, 0, 4);
			p++;
			i++;
		}
		if (i == 8) {
			for (i = 0; i < 8; i ++)
				q[i] ^= q2[i];
			encrypt_tea((unsigned long *)q, (unsigned long *)key, (unsigned long *)q1);
			for (i = 0; i < 8; i ++)
				q1[i] ^= mkey[i];
			memcpy(mkey, q, 8);
			count += 8;
			q2 = q1;
			q1 += 8;
			i = 0;
		}
	}
	*outlen = count;

	return 1;
}

/*
*
解密函数
*
*/
void CTeaCrypt::decrypt_tea(unsigned long *in, unsigned long *key, unsigned long *out) 
{
	
	unsigned long code[4]; 
	register unsigned long n=dwTeaRound, sum, y, z, delta = dwDelta; 
	
	sum = delta * n;
	y = htonl(in[0]); 
	z = htonl(in[1]); 
	
	code[0] = htonl(key[0]); code[1] = htonl(key[1]); 
	code[2] = htonl(key[2]); code[3] = htonl(key[3]); 
	
	while(n-->0) 
	{ 
		z -= ((y>>5)+code[3])^((y<<4)+code[2])^(sum+y); 
		y -= ((z>>5)+code[1])^((z<<4)+code[0])^(sum+z); 
		sum -= delta; 
	} 
	out[0] = htonl(y); 
	out[1] = htonl(z); 
} 

unsigned int CTeaCrypt::Tea_decrypt(IN unsigned char *in,IN int inlen,IN unsigned char *key,OUT unsigned char **out,OUT unsigned int *bytes)
{
	unsigned char q[8], mkey[8], *q1, *q2, *outp; 
	register int count, i, j, p; 
	
	if (inlen%8 || inlen<16) return 0; 
	/* get basic information of the packet */ 
	decrypt_tea((unsigned long *)in, (unsigned long *)key, (unsigned long *)q); 
	j = q[0]&0x7; 
	count = inlen - j - 10; 
	*bytes=count;
	*out=(unsigned char *)malloc(*bytes);
	if(*out==NULL) return 0;
	memset(*out,0,*bytes);


	if (count < 0) return 0; 
	
	memset(mkey, 0, 8); 
	q2 = mkey; 
	i = 8; p = 1; 
	q1 = in+8; 
	j ++; 
	while (p <= 2)
	{
		if (j < 8)
		{
			j ++; 
			p ++; 
		}
		else if (j == 8)
		{ 
			q2 = in; 
			for (j = 0; j < 8; j ++ )
			{ 
				if (i + j >= inlen) return 0; 
				q[j] ^= q1[j]; 
			} 
			decrypt_tea((unsigned long *)q, (unsigned long *)key,
				(unsigned long *)q ); 
			i += 8; 
			q1 += 8; 
			j = 0; 
		} 
	} 
	outp = *out; 
	while(count !=0)
	{ 
		if (j < 8)
		{ 
			outp[0] = q2[j] ^ q[j]; 
			outp ++; 
			count --; 
			j ++; 
		}
		else if (j == 8)
		{ 
			q2 = q1-8; 
			for (j = 0; j < 8; j ++ )
			{ 
				if (i + j >= inlen)
					return 0; 
				q[j] ^= q1[j]; 
			} 
			decrypt_tea((unsigned long *)q, (unsigned long *)key,
				(unsigned long *)q ); 
			i += 8; 
			q1 += 8; 
			j = 0; 
		} 
	} 
	for (p = 1; p < 8; p ++)
	{ 
		if (j < 8)
		{ 
			if (q2[j]^q[j]) 
				return 0; 
			j ++; 
		}
		else if (j == 8 )
		{ 
			q2 = q1; 
			for (j = 0; j < 8; j ++ )
			{ 
				if (i + j >= inlen)
					return 0; 
				q[j] ^= q1[j]; 
			} 
			decrypt_tea((unsigned long *)q, (unsigned long *)key, 
				(unsigned long *)q ); 
			i += 8; 
			q1 += 8; 
			j = 0; 
		} 
	} 
	return 1; 

}


