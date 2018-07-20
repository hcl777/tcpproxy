#pragma once

//服务器端

//接收到数据包
typedef struct tag_clims_packet
{
	int			size;
	char*		data;
}clims_packet_t;

//3个消息
enum CLIMS_MESSAGE{ 
	CLIMS_MSG_ONLINE=1,	//pdata = NULL;
	CLIMS_MSG_OFFLINE,	//pdata = NULL;
	CLIMS_MSG_PACKET		//pdata = (clim_packet_t*)
};
typedef void* CLIMS_HANDLE;
typedef void (*CLIMS_MSGCALL)(CLIMS_MESSAGE msg,int peerid,void* pdata,void* func_param);

CLIMS_HANDLE	clims_open(unsigned short port,CLIMS_MSGCALL fun,void* func_param);
void			clims_close(CLIMS_HANDLE h);
void			clims_root(CLIMS_HANDLE h); //一次工作循环
int				clims_send(CLIMS_HANDLE h,int peerid,char* data,int size);

