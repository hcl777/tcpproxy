#pragma once
#include "uac_UDPConfig.h"
//
//Aspera רҵ�������ļ�����,2015�걻IBM 2����Ԫ�չ�.
//http://asperasoft.com/zh/site/

//���֧��������
#define UAC_FD_SIZE 2048
#define UAC_SOCK_SENDBUF 1024000

typedef int UAC_SOCKET;

//�����к���ʹ�÷������ã�����C��־�����Բ���ʹ��C��׼�ӿ�
//#ifdef __cplusplus
//extern "C"
//{
//#endif

//UAC_sockaddrʹ�������򣬲�ʹ��������
typedef struct tag_UAC_sockaddr
{
	unsigned int	ip;
	unsigned short	port;
	char			nattype; //0~6
}UAC_sockaddr;

typedef struct tag_UAC_sendspeed
{
	unsigned int speedB;
	unsigned int lost_rate; //%�ֱ�
	unsigned int rerecv_rate; //%�ֱ�
	unsigned int seq_range; //�������seq����Сseq�Ĳ�.
	unsigned int ttlms; //ms
}UAC_sendspeed_t;

typedef struct tag_UAC_recvspeed
{
	unsigned int speedB;
	unsigned int rerecv_rate; //%�ֱ�
	unsigned int seq_range; //���seq����Сseq�Ĳ�.
	unsigned int ttlms; //ms
}UAC_recvspeed_t;

//**********************************
//nattype(0~5): nat���Ϳ�����ͨ��ƥ��
//
//			nat0	nat1	nat2	nat3	nat4
// nat0		1		1		1		1		1
// nat1		1	   	1		1		1		1
// nat2		1		1		1		1		1
// nat3		1		1		1		1		0
// nat4		1		1		1		0		0
//
//**********************************


typedef struct tag_UAC_fd_set
{
	int				fd_count;
	UAC_SOCKET		fd_array[UAC_FD_SIZE];
}UAC_fd_set;

//setting
void uac_setcallback_onnatok(UAC_CALLBACK_ONNATOK fun);
void uac_setcallback_onipportchanged(UAC_CALLBACK_ONIPPORTCHANGED fun);
//get
int uac_get_nattype();
//UAC ֻ��һ������˿ڣ����������ڴ˶˿ڻ�����ģ�����
int uac_init(unsigned short bindport,const char* stunsvr,unsigned short stunport,uac_config_t* conf); //ָ��UDP�˿�
int uac_fini();
int uac_set_mtu(unsigned int m);
int uac_get_conf(uac_config_t* conf);
int uac_set_conf(uac_config_t* conf);

//UAC socket �ӿ�
UAC_SOCKET uac_accept(UAC_sockaddr* sa_client);
UAC_SOCKET uac_connect(const UAC_sockaddr* sa_client);
int uac_closesocket(UAC_SOCKET fd);
int uac_setbandwidth(UAC_SOCKET fd,int size);
int uac_setsendbuf(UAC_SOCKET fd,int size);
int uac_setrecvbuf(UAC_SOCKET fd,int size);
UAC_sendspeed_t& uac_getsendspeed(UAC_SOCKET fd);
UAC_recvspeed_t& uac_getrecvspeed(UAC_SOCKET fd);
bool uac_is_read(UAC_SOCKET fd);
bool uac_is_write(UAC_SOCKET fd);
bool uac_is_connected(UAC_SOCKET fd);
int uac_select(UAC_fd_set* rset,UAC_fd_set* wset);
int uac_send(UAC_SOCKET fd,const char* buf,int len);//����:-1 �ر�
int uac_recv(UAC_SOCKET fd,char* buf,int len); //����:-1 �ر�


//#ifdef __cplusplus
//}
//#endif

