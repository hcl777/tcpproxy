#include "clacp.h"

#include "cla_sockpool.h"
#include "cl_net.h"
#include "cl_bstream.h"

unsigned int cla_ipaton(const char* ip)
{
	return cl_net::ip_aton(ip);
}
unsigned short cla_htons(unsigned short i)
{
	return cl_bstream::htob16(i);
}

bool g_cla_binit = false;
int cla_init(unsigned short port,const char* stunsvr,const cla_config_t& conf)
{
	if(g_cla_binit)
		return -1;
	g_cla_conf = conf;
	if(0!=cla_sockpoolSngl::instance()->init(port,stunsvr))
		return -1;
	g_cla_binit = true;
	return 0;
}
int cla_fini()
{
	if(!g_cla_binit)
		return 0;
	cla_sockpoolSngl::instance()->fini();
	cla_sockpoolSngl::destroy();
	g_cla_binit = false;
	return 0;
}
int	cla_get_config(cla_config_t& conf)
{
	conf = g_cla_conf;
	return 0;
}
int	cla_set_config(const cla_config_t& conf)
{
	g_cla_conf = conf;
	return 0;
}
void cla_set_mtu(int mtu)
{
	cla_sockpoolSngl::instance()->set_mtu(mtu);
}
CLA_SOCKET	cla_accept(cla_addr* from)
{
	return cla_sockpoolSngl::instance()->accept(from);
}
CLA_SOCKET	cla_connect(const cla_addr* to,const cla_addr* proxy)
{
	return cla_sockpoolSngl::instance()->connect(to,proxy);
}
int	cla_closesocket(CLA_SOCKET fd)
{
	return cla_sockpoolSngl::instance()->closesocket(fd);
}
int	cla_select(cla_fdset* rset,cla_fdset* wset)
{
	return cla_sockpoolSngl::instance()->select(rset,wset);
}
int	cla_send(CLA_SOCKET fd,const char* buf,int len)
{
	return cla_sockpoolSngl::instance()->send(fd,buf,len);
}
int	cla_recv(CLA_SOCKET fd,char* buf,int len)
{
	return cla_sockpoolSngl::instance()->recv(fd,buf,len);
}

bool cla_is_connected(CLA_SOCKET fd)
{
	return cla_sockpoolSngl::instance()->is_connected(fd);
}
bool cla_is_write(CLA_SOCKET fd)
{
	return cla_sockpoolSngl::instance()->is_write(fd);
}
bool cla_is_read(CLA_SOCKET fd)
{
	return cla_sockpoolSngl::instance()->is_read(fd);
}

int	cla_set_sendbuf(CLA_SOCKET fd,int size)
{
	return cla_sockpoolSngl::instance()->set_sendbuf(fd,size);
}
int	cla_set_recvbuf(CLA_SOCKET fd,int size)
{
	return cla_sockpoolSngl::instance()->set_recvbuf(fd,size);
}
int	cla_set_bandwidth(CLA_SOCKET fd,int size)
{
	return cla_sockpoolSngl::instance()->set_bandwidth(fd,size);
}
int	get_sendspeed(CLA_SOCKET fd,cla_sendspeed_t& s)
{
	return cla_sockpoolSngl::instance()->get_sendspeed(fd,s);
}
int	get_recvspeed(CLA_SOCKET fd,cla_recvspeed_t& s)
{
	return cla_sockpoolSngl::instance()->get_recvspeed(fd,s);
}



