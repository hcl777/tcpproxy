#pragma once
#include "cla_config.h"

typedef int CLA_SOCKET;

typedef struct tag_cla_addr
{
	unsigned int	ip;
	unsigned short	port;
	unsigned char	ntype;	//连接时使用
}cla_addr;
inline bool operator==(const cla_addr& a,const cla_addr& b){return (a.ip==b.ip && a.port==b.port);}
inline bool operator!=(const cla_addr& a,const cla_addr& b){return (a.ip!=b.ip || a.port!=b.port);}

#define CLA_FD_SIZE 2048
#define CLA_SOCK_SENDBUF 1024000
#define CLA_SOCK_RECVBUF 1024000

typedef struct tag_cla_fdset
{
	int				fd_count;
	CLA_SOCKET		fd_array[CLA_FD_SIZE];
}cla_fdset;

typedef struct tag_cla_sendspeed
{
	unsigned int speedB;
	unsigned int lost_rate; //%分比
	unsigned int rerecv_rate; //%分比
	unsigned int seq_range; //发送最大seq与最小seq的差.
	unsigned int ttlms; //ms
}cla_sendspeed_t;

typedef struct tag_cla_recvspeed
{
	unsigned int speedB;
	unsigned int rerecv_rate; //%分比
	unsigned int seq_range; //最大seq与最小seq的差.
	unsigned int ttlms; //ms
}cla_recvspeed_t;


unsigned int cla_ipaton(const char* ip);
unsigned short cla_htons(unsigned short i);



// noblocking ,全部非阻塞接口
int			cla_init(unsigned short port,const char* stunsvr,const cla_config_t& conf);
int			cla_fini();
int			cla_get_config(cla_config_t& conf);
int			cla_set_config(const cla_config_t& conf);
void		cla_set_mtu(int mtu);
/*
发起端： 先connect(),然后用is_write() 或 select() 查可写结果，如果可写，则查是否连接成功。
接收端： 直接accept() 到fd,则可以 send recv
*/
CLA_SOCKET	cla_accept(cla_addr* from);
CLA_SOCKET	cla_connect(const cla_addr* to,const cla_addr* proxy);
int			cla_closesocket(CLA_SOCKET fd);
int			cla_select(cla_fdset* rset,cla_fdset* wset);
int			cla_send(CLA_SOCKET fd,const char* buf,int len);//返回:-1 关闭
int			cla_recv(CLA_SOCKET fd,char* buf,int len); //返回:-1 关闭

bool		cla_is_connected(CLA_SOCKET fd);
bool		cla_is_write(CLA_SOCKET fd);
bool		cla_is_read(CLA_SOCKET fd);

int					cla_set_sendbuf(CLA_SOCKET fd,int size);
int					cla_set_recvbuf(CLA_SOCKET fd,int size);
int					cla_set_bandwidth(CLA_SOCKET fd,int size);
int					get_sendspeed(CLA_SOCKET fd,cla_sendspeed_t& s);
int					get_recvspeed(CLA_SOCKET fd,cla_recvspeed_t& s);

