#include "cla_proxyServer.h"
#include "cl_util.h"
#include "cl_net.h"
#include "cl_inifile.h"
#include "cla_proto.h"
#include "cl_socketbase.h"

cla_proxyServer::cla_proxyServer(void)
	:m_brun(false)
	,m_fd(INVALID_SOCKET)
{
}


cla_proxyServer::~cla_proxyServer(void)
{
	assert(INVALID_SOCKET==m_fd);
}
int cla_proxyServer::open(unsigned short port)
{
	if(m_brun)
		return 1;
	if(0==port) return -1;
	if(0!=cl_socketbase::open_udp_sock(m_fd,htons(port),INADDR_ANY,1024000,1024000))
		return -1;

	m_brun = true;
	return 0;
}
void cla_proxyServer::close()
{
	if(!m_brun)
		return;
	m_brun = false;
	cl_socketbase::close_sock(m_fd);
}

int cla_proxyServer::loop()
{
	char buf[4096];
	int size;
	sockaddr_in addr;
	socklen_t addrlen;
	cl_ptlstream ps(buf,4096);
	
	addrlen = sizeof(addr);
	memset(&addr,0,addrlen);

	while(m_brun)
	{
		size = recvfrom(m_fd,buf,4096,0,(sockaddr*)&addr,&addrlen);
		if(size>0)
		{
			//注意,char 与unsigned char 越界即错误
			if(CLA_PTL_PROXY_SEND==buf[0] && size>6)
			{
				cla_ptl_proxy_send2recv(buf,addr);
				sendto(m_fd,buf,size,0,(sockaddr*)&addr,addrlen);
			}
			else if(CLA_PTL_CNN_MYADDR==buf[0] && 5==size)
			{
				buf[0] = CLA_PTL_CNN_MYADDR_ACK;
				ps.seekw(5); //原来的32位index值不变
				ps << addr;
				sendto(m_fd,buf,11,0,(sockaddr*)&addr,addrlen);
			}
			else
			{
				DEBUGMSG("# *** unkown packet!!!\n");
			}
		}
	}
	return 0;
}

//**********************************************************************************
void cla_test_proxyServer(const char* ipport,bool loop)
{
	//测试发一个包myaddr，一个proxy包
	char buf[1024];
	unsigned int ip = cl_net::ip_atoh(cl_util::get_string_index(ipport,0,":").c_str());
	unsigned short port = cl_util::atoi(cl_util::get_string_index(ipport,1,":").c_str());
	int n;
	unsigned int ip2;
	unsigned short port2;
	cl_ptlstream ps;
	sockaddr_in addr;
	socklen_t addrlen = sizeof(addr);
	cla_addr claddr;

	memset(&addr,0,addrlen);

	SOCKET fd = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
	if(INVALID_SOCKET==fd)
		return;
	cl_socketbase::set_timeout(fd,3000);
	do
	{
		buf[0] = CLA_PTL_CNN_MYADDR;
		cl_socketbase::sendto(fd,buf,5,ip,port);
		if((n = cl_socketbase::recvfrom(fd,buf,1024,&ip2,&port2))<=0)
		{
			printf("*** recvfrom() fail! \n");
			break;
		}
		if(CLA_PTL_CNN_MYADDR_ACK != buf[0] ||11!=n)
		{
			printf("*** unkown packet!(%s:%d) \n",cl_net::ip_htoa(ip2),(int)port2);
			break;
		}
		ps.attach(buf+5,1024,n-5);
		ps >> claddr;
		printf("MYADDR ok: %s:%d \n",cl_net::ip_ntoa(claddr.ip),(int)ntohs(claddr.port));

		ps.attach(buf,1024,0);
		ps << CLA_PTL_PROXY_SEND;
		ps << claddr;
		cl_socketbase::sendto(fd,buf,1024,ip,port);
		if((n = cl_socketbase::recvfrom(fd,buf,1024,&ip2,&port2))<=0)
		{
			printf("*** recvfrom() fail! \n");
			break;
		}
		if(CLA_PTL_PROXY_RECV != buf[0] ||n<7)
		{
			printf("*** unkown packet!(%s:%d) \n",cl_net::ip_htoa(ip2),(int)port2);
			break;
		}
		ps.attach(buf+1,1024,n-1);
		if(0==ps>>claddr)
		{
			printf("PROXY_RECV ok: from(%s:%d) \n",cl_net::ip_ntoa(claddr.ip),(int)ntohs(claddr.port));
		}
		Sleep(1);
	}while(loop);

	closesocket(fd);
}

