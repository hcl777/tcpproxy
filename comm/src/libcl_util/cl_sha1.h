#pragma once


//**************************
typedef unsigned int u32;
typedef struct {
	u32  h0,h1,h2,h3,h4;
	u32  nblocks;
	unsigned char buf[64];
	int  count;
	u32 x[16]; //��ʱ������
} SHA1_CONTEXT;

void sha1_init( SHA1_CONTEXT *hd );
void sha1_write( SHA1_CONTEXT *hd, const unsigned char *inbuf, int inlen);
void sha1_final(SHA1_CONTEXT *hd);
//**************************


//**************************
//�����Ǽ򵥰�װ����
//sOutStr��sOutBuf����ͬʱΪ�գ�sOutStr��С41���ȣ�sOutBuf��С20���ȣ�iSleepMSec��ʾÿ����һ����ʱ��Ϣһ��
int Sha1_BuildFile(const char* sFile,char* sOutStr,char* sOutBuf,int iSleepMSec=-1,bool pmsg=true);  
int Sha1_BuildBuffer(const char* buf,int bufLen,char* sOutStr,char* sOutBuf);
int Sha1_Buf2Str(char* sOutStr, const char* sInBuf);
int Sha1_Str2Buf(char* sOutBuf, const char* sInStr);
