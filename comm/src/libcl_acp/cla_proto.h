#pragma once
#include "clacp.h"
#include "cl_bstream.h"
#include "cl_incnet.h"

//首字节
typedef unsigned char cla_cmd_t;
#define cla_cmd_t_sc static const cla_cmd_t

//***************************************************************************************
//第1字节：
cla_cmd_t_sc CLA_PTL_STUN_CMD		= 0x2f; //10 为 stun类
	
//连接连 条
cla_cmd_t_sc CLA_PTL_CNN_			= 0x40;	// 为cnn类 = 64
cla_cmd_t_sc CLA_PTL_PROXY_SEND		= CLA_PTL_CNN_ + 1; //发送请求中转,包头是一个cla_addr
cla_cmd_t_sc CLA_PTL_PROXY_RECV		= CLA_PTL_CNN_ + 2; //中转发目的接收,包头是一个cla_addr
cla_cmd_t_sc CLA_PTL_CNN_HOLE		= CLA_PTL_CNN_ + 3; //经stun proxy,
cla_cmd_t_sc CLA_PTL_CNN_HOLE_ACK	= CLA_PTL_CNN_ + 4; //经stun proxy，自己是nat4的话，可以不回复。走stun proxy,
cla_cmd_t_sc CLA_PTL_CNN_MYADDR		= CLA_PTL_CNN_ + 5; //向proxy服务器查自己的地址
cla_cmd_t_sc CLA_PTL_CNN_MYADDR_ACK	= CLA_PTL_CNN_ + 6; //
cla_cmd_t_sc CLA_PTL_CNN_PROXY		= CLA_PTL_CNN_ + 7; //经stun proxy,
cla_cmd_t_sc CLA_PTL_CNN_PROXY_ACK	= CLA_PTL_CNN_ + 8; //cnnid,经proxyaddr回

cla_cmd_t_sc CLA_PTL_CNN_NAT		= CLA_PTL_CNN_ + 9; //cnnid,不用带cnnid
cla_cmd_t_sc CLA_PTL_CNN_SYN		= CLA_PTL_CNN_ + 10; //
cla_cmd_t_sc CLA_PTL_CNN_ACK		= CLA_PTL_CNN_ + 11;
cla_cmd_t_sc CLA_PTL_CNN_LIVE		= CLA_PTL_CNN_ + 12; 
cla_cmd_t_sc CLA_PTL_CNN_CLS		= CLA_PTL_CNN_ + 13; 
cla_cmd_t_sc CLA_PTL_CNN_DATA		= CLA_PTL_CNN_ + 14;
cla_cmd_t_sc CLA_PTL_CNN_DATA_ACK	= CLA_PTL_CNN_ + 15;
//---------------------------------------------------------------------------------------

//***************************************************************************************
//stun类包第二字节
cla_cmd_t_sc CLA_PTL_STUN_			= 100;
//get my addr,keepalive
cla_cmd_t_sc CLA_PTL_STUN_LIVE		= CLA_PTL_STUN_ + 1;	//空包
cla_cmd_t_sc CLA_PTL_STUN_LIVE_ACK	= CLA_PTL_STUN_ + 2;

//check_nat
cla_cmd_t_sc CLA_PTL_STUN_B			= CLA_PTL_STUN_ + 3;		//空包
cla_cmd_t_sc CLA_PTL_STUN_B_ACK		= CLA_PTL_STUN_ + 4;
cla_cmd_t_sc CLA_PTL_STUN_NAT1		= CLA_PTL_STUN_ + 5;		//空包
cla_cmd_t_sc CLA_PTL_STUN_NAT1_ACK	= CLA_PTL_STUN_ + 6;		//stuna call stunc 转
cla_cmd_t_sc CLA_PTL_STUN_NAT4		= CLA_PTL_STUN_ + 7;		//空包
cla_cmd_t_sc CLA_PTL_STUN_NAT4_ACK	= CLA_PTL_STUN_ + 8;		
cla_cmd_t_sc CLA_PTL_STUN_NAT2		= CLA_PTL_STUN_ + 9;		//空包
cla_cmd_t_sc CLA_PTL_STUN_NAT2_ACK	= CLA_PTL_STUN_ + 10;		//stuna1 call stuna2 转
//---------------------------------------------------------------------------------------

//判断无符号整数i在base前还是在base后，主要考虑短时间内不会发生大差距变化的情况
#define cla_before(i,base) (((int)(i) - (int)(b)) < 0)
#define cla_after(i,base) (((int)(i) - (int)(b)) > 0)

typedef struct tag_cla_session
{
	sint32		sid;      //源会话sessionID
	sint32		did;      //目的会话sessionID
	tag_cla_session(void):sid(0),did(0){}
}cla_session_t;


int operator << (cl_ptlstream& ps, const sockaddr_in& addr);
int operator >> (cl_ptlstream& ps, sockaddr_in& addr);

int operator << (cl_ptlstream& ps, const cla_addr& inf);
int operator >> (cl_ptlstream& ps, cla_addr& inf);

int operator << (cl_ptlstream& ps, const cla_session_t& inf);
int operator >> (cl_ptlstream& ps, cla_session_t& inf);

/*
说明：
将 proxy_send 包转换为 proxy_recv 包
addr_io参数： 输入为来源地址，转换后填充为发送目的地，并将原地址填入proxy包头。
*/
void cla_ptl_proxy_send2recv(char* buf,sockaddr_in& addr_io);


