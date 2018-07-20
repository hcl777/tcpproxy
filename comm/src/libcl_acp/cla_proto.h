#pragma once
#include "clacp.h"
#include "cl_bstream.h"
#include "cl_incnet.h"

//���ֽ�
typedef unsigned char cla_cmd_t;
#define cla_cmd_t_sc static const cla_cmd_t

//***************************************************************************************
//��1�ֽڣ�
cla_cmd_t_sc CLA_PTL_STUN_CMD		= 0x2f; //10 Ϊ stun��
	
//������ ��
cla_cmd_t_sc CLA_PTL_CNN_			= 0x40;	// Ϊcnn�� = 64
cla_cmd_t_sc CLA_PTL_PROXY_SEND		= CLA_PTL_CNN_ + 1; //����������ת,��ͷ��һ��cla_addr
cla_cmd_t_sc CLA_PTL_PROXY_RECV		= CLA_PTL_CNN_ + 2; //��ת��Ŀ�Ľ���,��ͷ��һ��cla_addr
cla_cmd_t_sc CLA_PTL_CNN_HOLE		= CLA_PTL_CNN_ + 3; //��stun proxy,
cla_cmd_t_sc CLA_PTL_CNN_HOLE_ACK	= CLA_PTL_CNN_ + 4; //��stun proxy���Լ���nat4�Ļ������Բ��ظ�����stun proxy,
cla_cmd_t_sc CLA_PTL_CNN_MYADDR		= CLA_PTL_CNN_ + 5; //��proxy���������Լ��ĵ�ַ
cla_cmd_t_sc CLA_PTL_CNN_MYADDR_ACK	= CLA_PTL_CNN_ + 6; //
cla_cmd_t_sc CLA_PTL_CNN_PROXY		= CLA_PTL_CNN_ + 7; //��stun proxy,
cla_cmd_t_sc CLA_PTL_CNN_PROXY_ACK	= CLA_PTL_CNN_ + 8; //cnnid,��proxyaddr��

cla_cmd_t_sc CLA_PTL_CNN_NAT		= CLA_PTL_CNN_ + 9; //cnnid,���ô�cnnid
cla_cmd_t_sc CLA_PTL_CNN_SYN		= CLA_PTL_CNN_ + 10; //
cla_cmd_t_sc CLA_PTL_CNN_ACK		= CLA_PTL_CNN_ + 11;
cla_cmd_t_sc CLA_PTL_CNN_LIVE		= CLA_PTL_CNN_ + 12; 
cla_cmd_t_sc CLA_PTL_CNN_CLS		= CLA_PTL_CNN_ + 13; 
cla_cmd_t_sc CLA_PTL_CNN_DATA		= CLA_PTL_CNN_ + 14;
cla_cmd_t_sc CLA_PTL_CNN_DATA_ACK	= CLA_PTL_CNN_ + 15;
//---------------------------------------------------------------------------------------

//***************************************************************************************
//stun����ڶ��ֽ�
cla_cmd_t_sc CLA_PTL_STUN_			= 100;
//get my addr,keepalive
cla_cmd_t_sc CLA_PTL_STUN_LIVE		= CLA_PTL_STUN_ + 1;	//�հ�
cla_cmd_t_sc CLA_PTL_STUN_LIVE_ACK	= CLA_PTL_STUN_ + 2;

//check_nat
cla_cmd_t_sc CLA_PTL_STUN_B			= CLA_PTL_STUN_ + 3;		//�հ�
cla_cmd_t_sc CLA_PTL_STUN_B_ACK		= CLA_PTL_STUN_ + 4;
cla_cmd_t_sc CLA_PTL_STUN_NAT1		= CLA_PTL_STUN_ + 5;		//�հ�
cla_cmd_t_sc CLA_PTL_STUN_NAT1_ACK	= CLA_PTL_STUN_ + 6;		//stuna call stunc ת
cla_cmd_t_sc CLA_PTL_STUN_NAT4		= CLA_PTL_STUN_ + 7;		//�հ�
cla_cmd_t_sc CLA_PTL_STUN_NAT4_ACK	= CLA_PTL_STUN_ + 8;		
cla_cmd_t_sc CLA_PTL_STUN_NAT2		= CLA_PTL_STUN_ + 9;		//�հ�
cla_cmd_t_sc CLA_PTL_STUN_NAT2_ACK	= CLA_PTL_STUN_ + 10;		//stuna1 call stuna2 ת
//---------------------------------------------------------------------------------------

//�ж��޷�������i��baseǰ������base����Ҫ���Ƕ�ʱ���ڲ��ᷢ������仯�����
#define cla_before(i,base) (((int)(i) - (int)(b)) < 0)
#define cla_after(i,base) (((int)(i) - (int)(b)) > 0)

typedef struct tag_cla_session
{
	sint32		sid;      //Դ�ỰsessionID
	sint32		did;      //Ŀ�ĻỰsessionID
	tag_cla_session(void):sid(0),did(0){}
}cla_session_t;


int operator << (cl_ptlstream& ps, const sockaddr_in& addr);
int operator >> (cl_ptlstream& ps, sockaddr_in& addr);

int operator << (cl_ptlstream& ps, const cla_addr& inf);
int operator >> (cl_ptlstream& ps, cla_addr& inf);

int operator << (cl_ptlstream& ps, const cla_session_t& inf);
int operator >> (cl_ptlstream& ps, cla_session_t& inf);

/*
˵����
�� proxy_send ��ת��Ϊ proxy_recv ��
addr_io������ ����Ϊ��Դ��ַ��ת�������Ϊ����Ŀ�ĵأ�����ԭ��ַ����proxy��ͷ��
*/
void cla_ptl_proxy_send2recv(char* buf,sockaddr_in& addr_io);


