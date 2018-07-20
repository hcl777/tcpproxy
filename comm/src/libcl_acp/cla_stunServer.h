#pragma once
#include "cla_proto.h"


//NAT 的UDP简单穿越
//STUN --- Simple Traversal of UDP over NATs
//STUN --- Simple Traversal of User Datagram Protocol (UDP) Through Network Address Translators (NATs)

//nat1: 完全透明NAT(Full Cone NAT)
//nat2: 受限NAT(Restricted Cone)
//nat3: 端口受限NAT(Port Restricted Cone)
//nat4: 对称NAT(Symmetric)
//nat5: 无法接收外部包
//nat6: 末知类型

//**********************************
//nat类型可连接通性匹配
//
//        nat1   nat2   nat3   nat4
// nat1    1      1      1      1
// nat2    1      1      1      1
// nat3    1      1      1      0
// nat4    1      1      0      0
//
//**********************************

//**********************************
//连接类型: 
//01, 11,12,13,14,21,22,23,24,31,32,33,41,42
//01 表示 代理连接. 其它则十位表示自己的NAT类型,个位表示对方NAT类型.
//
//**********************************

//注意: 
//1.有这样的NAT，当绑定的端口是7108，而外网也实际也只能向这个机器7108发送数据才收得到，而从这机器发出的数据包到外面，外面看到的却是另一
//个端口如12345，而外面直接回复12345结果收不到。
//2.有一种nat4,它同时向两个IP探NA4包时，结果回来是一个端口结果误判为NAT3，但真实连接时又分配不同端口，即实质是NAT4类型
//3.有些客户连IP1与连IP2，外部所看到的IP不一样

//包标志: STUN_HEAD_STX   0xbf

/**********************************************************************************************************
协议说明：
	stun server由两个不同IP地址的服务器对组成,设A、B服务器。
	A、B分别绑定1个端口 --- A(IP:P+随机),B(IP:P+随机)，随机端口只用于回复restricted-cone(NAT2)
协议：
1.req-my-addr / rsp-my-addr				：获取本地绑定的外网IP：PORT
2.req-stunb-addr / rsp-stunb-addr					：获取stun server :B的信息
3.req-full-cone / rsp-full-cone 					：探NAT1。 发S1，S4回，收到即是NAT1
4.req-symmetric / rsp-symmetric						：探NAT4。 发S1、S3，S1、S3回IP：PORT，不相同即是NA4
5.req-restricted-cone /rsp-restricted-cone			：探NAT2。 发S1、S2回，收到即NA2，不然为NAT3
**********************************************************************************************************/
//如果stun server压力过大，可以考虑返回其它nat1的一对服务器实现探测



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
	int					m_fd2;	//只发送NAT2包，不接收
	sockaddr_in			m_stunB_addr;
};

