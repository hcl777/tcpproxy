#include "cla_stunServer.h"
#include "cl_incnet.h"
#include "cl_util.h"
#include "cl_net.h"
#include "cl_socketbase.h"


cla_stunServer::cla_stunServer(void)
	:m_brun(false)
	,m_fd(INVALID_SOCKET)
	,m_fd2(INVALID_SOCKET)
{
}


cla_stunServer::~cla_stunServer(void)
{
	assert(INVALID_SOCKET==m_fd);
}
int cla_stunServer::open(cla_addr& stunA,cla_addr& stunB)
{
	if(m_brun)
		return 1;
	memset(&m_stunB_addr,0,sizeof(m_stunB_addr));
	m_stunB_addr.sin_family = AF_INET;
	m_stunB_addr.sin_addr.s_addr = stunB.ip;
	m_stunB_addr.sin_port = stunB.port;
	
	//m_fd2处理nat2的包
	int bufsize = 1024000; //缓冲设为1M
	m_fd2 = (int)socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
	if(INVALID_SOCKET == m_fd2)
		return -1;
	setsockopt(m_fd2, SOL_SOCKET, SO_SNDBUF, ( const char* )&bufsize, sizeof(int)); 

	if(0!=cl_socketbase::open_udp_sock(m_fd,stunA.port,stunA.ip,bufsize,bufsize))
	{
		cl_socketbase::close_sock(m_fd2);
		return -1;
	}
	m_brun = true;
	return 0;
}
void cla_stunServer::close()
{
	if(!m_brun)
		return;
	m_brun = false;
	cl_socketbase::close_sock(m_fd);
	cl_socketbase::close_sock(m_fd2);
}

int cla_stunServer::loop()
{
	char buf[4096];
	int size;
	sockaddr_in addr;
	socklen_t addrlen;
	cl_ptlstream ps(1024);
	cl_ptlstream psrcv(buf,4096,4096);
	ps << CLA_PTL_STUN_CMD;
	
	addrlen = sizeof(addr);
	memset(&addr,0,addrlen);
	while(m_brun)
	{
		size = recvfrom(m_fd,buf,4096,0,(sockaddr*)&addr,&addrlen);
		if(size>0)
		{
			if(CLA_PTL_PROXY_SEND==buf[0] && size>7)
			{
				cla_ptl_proxy_send2recv(buf,addr);
				sendto(m_fd,buf,size,0,(sockaddr*)&addr,addrlen);
			}
			else if(CLA_PTL_STUN_CMD==buf[0])
			{
				switch(buf[1])
				{
				case CLA_PTL_STUN_LIVE:
					sendto_T1(m_fd,CLA_PTL_STUN_LIVE_ACK,addr,addr,ps);
					break;
				case CLA_PTL_STUN_B:
					sendto_T1(m_fd,CLA_PTL_STUN_B_ACK,m_stunB_addr,addr,ps);
					break;
				case CLA_PTL_STUN_NAT1:
					sendto_T1(m_fd,CLA_PTL_STUN_NAT1_ACK,addr,m_stunB_addr,ps);
					break;
				case CLA_PTL_STUN_NAT1_ACK:
					if(size==8)
					{
						psrcv.seekr(2);
						psrcv >> addr;
						sendto_T1(m_fd,CLA_PTL_STUN_NAT1_ACK,addr,addr,ps);
					}
					break;
				case CLA_PTL_STUN_NAT4:
					sendto_T1(m_fd,CLA_PTL_STUN_NAT4_ACK,addr,addr,ps);
					break;
				case CLA_PTL_STUN_NAT2:
					sendto_T1(m_fd2,CLA_PTL_STUN_NAT2_ACK,addr,addr,ps);
					break;
				default:
					DEBUGMSG("# *** unkown packet!!!\n");
					break;
				}
			}
			else
			{
				DEBUGMSG("# *** unkown packet!!!\n");
			}
		}
	}
	return 0;
}

