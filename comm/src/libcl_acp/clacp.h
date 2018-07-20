#pragma once
#include "cla_config.h"

typedef int CLA_SOCKET;

typedef struct tag_cla_addr
{
	unsigned int	ip;
	unsigned short	port;
	unsigned char	ntype;	//����ʱʹ��
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
	unsigned int lost_rate; //%�ֱ�
	unsigned int rerecv_rate; //%�ֱ�
	unsigned int seq_range; //�������seq����Сseq�Ĳ�.
	unsigned int ttlms; //ms
}cla_sendspeed_t;

typedef struct tag_cla_recvspeed
{
	unsigned int speedB;
	unsigned int rerecv_rate; //%�ֱ�
	unsigned int seq_range; //���seq����Сseq�Ĳ�.
	unsigned int ttlms; //ms
}cla_recvspeed_t;


unsigned int cla_ipaton(const char* ip);
unsigned short cla_htons(unsigned short i);



// noblocking ,ȫ���������ӿ�
int			cla_init(unsigned short port,const char* stunsvr,const cla_config_t& conf);
int			cla_fini();
int			cla_get_config(cla_config_t& conf);
int			cla_set_config(const cla_config_t& conf);
void		cla_set_mtu(int mtu);
/*
����ˣ� ��connect(),Ȼ����is_write() �� select() ���д����������д������Ƿ����ӳɹ���
���նˣ� ֱ��accept() ��fd,����� send recv
*/
CLA_SOCKET	cla_accept(cla_addr* from);
CLA_SOCKET	cla_connect(const cla_addr* to,const cla_addr* proxy);
int			cla_closesocket(CLA_SOCKET fd);
int			cla_select(cla_fdset* rset,cla_fdset* wset);
int			cla_send(CLA_SOCKET fd,const char* buf,int len);//����:-1 �ر�
int			cla_recv(CLA_SOCKET fd,char* buf,int len); //����:-1 �ر�

bool		cla_is_connected(CLA_SOCKET fd);
bool		cla_is_write(CLA_SOCKET fd);
bool		cla_is_read(CLA_SOCKET fd);

int					cla_set_sendbuf(CLA_SOCKET fd,int size);
int					cla_set_recvbuf(CLA_SOCKET fd,int size);
int					cla_set_bandwidth(CLA_SOCKET fd,int size);
int					get_sendspeed(CLA_SOCKET fd,cla_sendspeed_t& s);
int					get_recvspeed(CLA_SOCKET fd,cla_recvspeed_t& s);

