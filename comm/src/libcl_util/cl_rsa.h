#pragma once


/*******************************
rsa�㷨:
��Կ:unsigned int n,e,d. ����(n,e)Ϊ��Կ, (n,d)Ϊ˽Կ.
n �� ����������p,q(����)��ʣ��. e��ѡ,����e ��(p-1)*(q-1) ����.
n�Ķ����Ʊ�ʾʱ��ռ�õ�λ����������ν����Կ����.
d��Ҫ��(d*e)mod((p-1)*(q-1))=1 ; (mod ������%).

�ӽ���:
RSA�ӽ��ܵ��㷨��ȫ��ͬ����AΪ���ģ�BΪ���ģ���
A=B^d mod n��
B=A^a mod n��

���Ŀ��λ������С��A.
Ŀǰһ��nΪ1024 λ��Կ���ȼ���ȫ.�����ܼ���������.
��Կָ��e�ձ�ѡ�Ķ���65537��0x10001��5bits�����ڣ���ֵ�ǳ���1��3��5��17��257֮�����С����.
********************************/

/****************************************************
cl_rsa32 (n,e,d): �����ο�.�������ƽ�.

*****************************************************/
int cl_rsa32_randkey1(unsigned int& p,unsigned int& q,unsigned int& N,unsigned int& n,unsigned int& e,unsigned int& d);
int cl_rsa32_randkey(unsigned int& n,unsigned int& e,unsigned int& d);
//��16λ����,�������ֲ���.osize ����Ԥ��ָ��Ϊosize>=2size
int cl_rsa32_encrypt(unsigned int n,unsigned int e,const char* buf,int size,char* obuf,int* osize);
//��32λ����,�������ֲ���.osize ����Ԥ��ָ��Ϊosize>=size/2
int cl_rsa32_decrypt(unsigned int n,unsigned int d,const char* buf,int size,char* obuf,int* osize);

//
void cl_rsa32_test();

