#pragma once
#define TEA_KEY_LEN	0x10

#define TEA_ROUNDS_ONE	0x10
#define TEA_DELTA_ONE	0x9e3779b9

#define TEA_ROUNDS_TWO	0x20
#define TEA_DELTA_TWO	0x57E89147


class CTeaCrypt
{
public:
	CTeaCrypt(void);
	~CTeaCrypt(void);
private:
	unsigned int dwTeaRound;
	unsigned int dwDelta;
private:
	void InitTea(unsigned int mRound,unsigned int mDelta){dwTeaRound=mRound;dwDelta=mDelta;};
	void decrypt_tea(unsigned long *in, unsigned long *key, unsigned long *out);
public:
	void InitTeaOne();			//默认16轮tea算法
	void InitTeaTwo();			//默认32轮tea算法
public:
	unsigned int Tea_encrypt(unsigned char *s,int len,unsigned char *key,unsigned char *dst,unsigned int *bytes);
	unsigned int Tea_decrypt(unsigned char *s,int len,unsigned char *key,unsigned char **dst,unsigned int *bytes);
	unsigned int encrypt_vague(unsigned char *s,unsigned int len);
	void encrypt_tea(unsigned long *in, unsigned long *key, unsigned long *out);

public:
	static void initkey(unsigned char *key,unsigned int len);
};


