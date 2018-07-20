#pragma once

//客户端

//接收到数据包
typedef struct tag_clim_packet
{
	int			size;
	char*		data;
}clim_packet_t;

//3个消息
enum CLIM_MESSAGE{ 
	CLIM_MSG_ONLINE=1,	//pdata = NULL;
	CLIM_MSG_OFFLINE,	//pdata = NULL;
	CLIM_MSG_PACKET		//pdata = (clim_packet_t*)
};
typedef void* CLIM_HANDLE;
typedef void (*CLIM_MSGCALL)(CLIM_MESSAGE msg,void* pdata,void* func_param);

CLIM_HANDLE		clim_open(const char* svr,CLIM_MSGCALL fun,void* func_param);
void			clim_close(CLIM_HANDLE h);
void			clim_root(CLIM_HANDLE h); //一次工作循环
int				clim_send(CLIM_HANDLE h,char* data,int size);


