#include "clim_client.h"

//客户端1个连接，采用异步select 模式

clim_client::clim_client(void)
	:m_state(CL_DISCONNECTED)
	,m_fd(INVALID_SOCKET)
{
}
clim_client::~clim_client(void)
{
}
void clim_client::reset()
{
}
int	clim_client::open(const char* svr,CLIM_MESSAGE func,void* func_param)
{
	
	return 0;
}
void clim_client::close()
{

}
void clim_client::root()
{
}
int clim_client::send(char* data,int size)
{
	return 0;
}


