#pragma once
 
//��md5����汾��cl_md5Ҫ��ܶ�.
//100MB����:
//win c++����: md5=1.5��,sha1=1.5��
//linux c++����: md5=1.7��. sha1=1.9��
//linux�Դ�: md5=1��, sha1=1.5��.

typedef struct
{
    unsigned int count[2];
    unsigned int state[4];
    unsigned char buffer[64];   

}CL_MD5_CTX;
                                           
void cl_md5_init(CL_MD5_CTX *context);
void cl_md5_update(CL_MD5_CTX *context,unsigned char *input,unsigned int inputlen);
void cl_md5_final(CL_MD5_CTX *context,unsigned char digest[16]);

void cl_md5_hx2str(unsigned char digest[16],char outstr[33]);
char* cl_md5_buffer(unsigned  char* buf,int size,char outstr[33]);
