#pragma once

//ip packet analysis. IP ������


struct cl_ip_iaddr
{
	int protocol;
	unsigned int snip;
	unsigned int dnip;
	unsigned short snport;
	unsigned int dnport;
	int pk_size;
};


//big endian
//20 byte
struct cl_ip_header
{
  unsigned char ihl:4, version:4;	//��4B:�ײ�����(��λΪ32bit,��4�ֽ�)����4B:�汾 
  unsigned char tos;		//��������
  unsigned short tot_len;	//���ܳ��ȣ���IPͷ+IP��������
  unsigned short id;		//Ψһ��ʶ��ip����һ����0��ʼ��������
  unsigned short frag_off;	//��13λ��ƫ�ƣ���3B:��־(0,DF,MF��DF=1��ʾ��֧��·�ɷ�Ƭ��MF=1��ʾ��Ƭ��
  unsigned char ttl;		//����ʱ��
  unsigned char protocol;	//Э�飺1-ICMP��2-IGMP 6-TCP��17-UDP,88-IGRP,89-OSPF
  unsigned short check;		//IP�ײ������
  unsigned int saddr;		//Դ��ַIPV4
  unsigned int daddr;		//Ŀ�ĵ�ַ
  /*The options start here. */
};
//20 byte
struct cl_tcp_header
{
  unsigned short source; //Դ�˿�
  unsigned short dest;	//Ŀ��˿�
  unsigned int seq;	//�������к�
  unsigned int ack_seq;	//ACK���к�
  //���¼�����LITTLE_ENDIAN
  //��4B ��������4B:TCPͷ��, ...��2B����
  unsigned short res1:4, doff:4, fin:1, syn:1, rst:1, psh:1,
                 ack:1, urg:1,res2:2; //
  unsigned short window;	//���ʹ���(���շ������С)
  unsigned short check; //�����,(�ײ�+����)��У���
  unsigned short urg_ptr; //����ָ�룬ָ��������ݵ���ʼλ��
  //ѡ�
  //(kind��length) = (0,1����������) ,��1��1�޲��������),��2(MTU),4�� ��(3(������������),4����(4��SACK),2����(8��ʱ���),10��
};
//8 byte
struct cl_udp_header
{
  unsigned short source; //Դ�˿�
  unsigned short dest;	//Ŀ��˿�
  unsigned short len;	//���ݳ���,��UDPͷ����
  unsigned short chsum;	// У���
};

//***************************************************************
class cl_ipa
{
private:
	cl_ipa(void){}
	~cl_ipa(void){}
public:
	static int get_ip_iaddr(cl_ip_iaddr& ipi,const char* buf,int size);
	static void dump_ippacket(const char* buf,int size);
	static void dump_ippacket2(const char* buf,int size);

};

