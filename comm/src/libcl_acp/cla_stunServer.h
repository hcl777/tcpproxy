#pragma once
#include "cla_proto.h"


//NAT ��UDP�򵥴�Խ
//STUN --- Simple Traversal of UDP over NATs
//STUN --- Simple Traversal of User Datagram Protocol (UDP) Through Network Address Translators (NATs)

//nat1: ��ȫ͸��NAT(Full Cone NAT)
//nat2: ����NAT(Restricted Cone)
//nat3: �˿�����NAT(Port Restricted Cone)
//nat4: �Գ�NAT(Symmetric)
//nat5: �޷������ⲿ��
//nat6: ĩ֪����

//**********************************
//nat���Ϳ�����ͨ��ƥ��
//
//        nat1   nat2   nat3   nat4
// nat1    1      1      1      1
// nat2    1      1      1      1
// nat3    1      1      1      0
// nat4    1      1      0      0
//
//**********************************

//**********************************
//��������: 
//01, 11,12,13,14,21,22,23,24,31,32,33,41,42
//01 ��ʾ ��������. ������ʮλ��ʾ�Լ���NAT����,��λ��ʾ�Է�NAT����.
//
//**********************************

//ע��: 
//1.��������NAT�����󶨵Ķ˿���7108��������Ҳʵ��Ҳֻ�����������7108�������ݲ��յõ���������������������ݰ������棬���濴����ȴ����һ
//���˿���12345��������ֱ�ӻظ�12345����ղ�����
//2.��һ��nat4,��ͬʱ������IP̽NA4��ʱ�����������һ���˿ڽ������ΪNAT3������ʵ����ʱ�ַ��䲻ͬ�˿ڣ���ʵ����NAT4����
//3.��Щ�ͻ���IP1����IP2���ⲿ��������IP��һ��

//����־: STUN_HEAD_STX   0xbf

/**********************************************************************************************************
Э��˵����
	stun server��������ͬIP��ַ�ķ����������,��A��B��������
	A��B�ֱ��1���˿� --- A(IP:P+���),B(IP:P+���)������˿�ֻ���ڻظ�restricted-cone(NAT2)
Э�飺
1.req-my-addr / rsp-my-addr				����ȡ���ذ󶨵�����IP��PORT
2.req-stunb-addr / rsp-stunb-addr					����ȡstun server :B����Ϣ
3.req-full-cone / rsp-full-cone 					��̽NAT1�� ��S1��S4�أ��յ�����NAT1
4.req-symmetric / rsp-symmetric						��̽NAT4�� ��S1��S3��S1��S3��IP��PORT������ͬ����NA4
5.req-restricted-cone /rsp-restricted-cone			��̽NAT2�� ��S1��S2�أ��յ���NA2����ȻΪNAT3
**********************************************************************************************************/
//���stun serverѹ�����󣬿��Կ��Ƿ�������nat1��һ�Է�����ʵ��̽��



class cla_stunServer
{
public:
	cla_stunServer(void);
	~cla_stunServer(void);
public:

	int open(cla_addr& stunA,cla_addr& stunB);
	void close();
	int loop();
private:
	template<typename T>
	static int sendto_T1(int fd,cla_cmd_t cmd,const T& inf,const sockaddr_in& to_addr,cl_ptlstream& ps)
	{
		ps.seekw(1);
		ps << cmd;
		ps << inf;
		return sendto(fd,ps.buffer(),ps.length(),0,(sockaddr*)&to_addr,sizeof(to_addr));
	}

private:
	bool				m_brun;
	int					m_fd;
	int					m_fd2;	//ֻ����NAT2����������
	sockaddr_in			m_stunB_addr;
};

