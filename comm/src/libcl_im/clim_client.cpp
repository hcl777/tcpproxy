#include "clim_client.h"

//�ͻ���1�����ӣ������첽select ģʽ

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


