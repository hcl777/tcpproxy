#include "cla_stunClient.h"
#include "cl_util.h"
#include "cl_net.h"
#include "cla_config.h"

cla_stunClient::cla_stunClient()
{
	m_cn_state = CN_IDLE;
	m_cn_scount = 0;
	memset(&m_stunA,0,sizeof(sockaddr_in));
	memset(&m_stunB,0,sizeof(sockaddr_in));
	m_stunA.sin_family = AF_INET;
	m_stunB.sin_family = AF_INET;
}

cla_stunClient::~cla_stunClient(void)
{
}
int cla_stunClient::init(int fd,const char* svr)
{
	if(!svr || 0==strlen(svr))
		return -1;
	m_fd = fd;
	m_mya.ip = 0;
	m_mya.port = 0;
	m_stunip = cl_util::get_string_index(svr,0,":");
	m_stunA.sin_port =  htons((unsigned short)atoi(cl_util::get_string_index(svr,1,":").c_str()));
	m_stunA.sin_addr.s_addr = cl_net::ip_aton_try_explain_ex(m_stunip.c_str());
	//每20秒执行1次LIVE
	cla_timerSngl::instance()->register_timer(static_cast<cla_timerHandler*>(this),TIMER_LIVE,20000);
	 //type未探测成功的才会继续探测
	cla_timerSngl::instance()->register_timer(static_cast<cla_timerHandler*>(this),TIMER_CHECK_NAT,120000);
	checknat_begin();
	return 0;
}
void cla_stunClient::fini()
{
	cla_timerSngl::instance()->unregister_all(static_cast<cla_timerHandler*>(this));
	m_fd = INVALID_SOCKET;
}
sockaddr_in& cla_stunClient::stunA()
{
	//尝试解释域名
	if(0==m_stunA.sin_addr.s_addr && !m_stunip.empty())
		m_stunA.sin_addr.s_addr = cl_net::ip_aton_try_explain_ex(m_stunip.c_str());
	return m_stunA;
}
int cla_stunClient::send_ptl(cla_cmd_t cmd,sockaddr_in& a)
{
	char buf[32];
	buf[0] = CLA_PTL_STUN_CMD;
	buf[1] = cmd;
	//DEBUGMSG("# send_ptl(%d) \n",(int)cmd);
	return sendto(m_fd,buf,2,0,(sockaddr*)&a,sizeof(a));
}

void cla_stunClient::on_myaddr(const cla_addr& a)
{
	if(m_mya!=a)
	{
		m_mya = a;
		DEBUGMSG("# on ipport(%s:%d) \n",cl_net::ip_ntoa(a.ip),(int)ntohs(a.port));
		if(g_cla_conf.callback_onipportchanged)
			g_cla_conf.callback_onipportchanged(m_mya.ip,m_mya.port);
	}
}
void cla_stunClient::checknat_end(int nattype)
{
	if(CN_IDLE==m_cn_state)
		return;
	m_cn_scount = 0;
	m_cn_state = CN_IDLE;
	cla_timerSngl::instance()->unregister_timer(static_cast<cla_timerHandler*>(this),TIMER_NAT_RESEND);

	if(6==g_cla_conf.nattype || 5!=nattype)
	{
		//如果之前未获得过nattype(6),则所有类似都通知，否则只有不等5时才会响应处理和通知
		DEBUGMSG("# on nattype=%d \n",nattype);
		g_cla_conf.nattype = nattype;
		if(g_cla_conf.callback_onnatok)
			g_cla_conf.callback_onnatok(nattype);
	}
	if(g_cla_conf.nattype>0 && g_cla_conf.nattype<5)
		cla_timerSngl::instance()->unregister_timer(static_cast<cla_timerHandler*>(this),TIMER_CHECK_NAT);
}
void cla_stunClient::checknat_begin()
{
	DEBUGMSG("# checknat_begin ... \n");
	m_na4_portA = m_na4_portB = 0;
	m_cn_scount = 0;
	m_cn_state = CN_STUNB;
	cla_timerSngl::instance()->register_timer(static_cast<cla_timerHandler*>(this),TIMER_NAT_RESEND,600);
}
void cla_stunClient::on_data(char* buf,int size,const cla_addr& from)
{
	cla_cmd_t cmd;
	cla_addr a;
	cl_ptlstream ps(buf,size,size);
	ps>>cmd;
	if(0!=ps>>a)
		return;
	switch(cmd)
	{
	case CLA_PTL_STUN_LIVE_ACK:
		on_myaddr(a);
		break;
	case CLA_PTL_STUN_B_ACK:
		{
			m_stunB.sin_addr.s_addr = a.ip;
			m_stunB.sin_port = a.port;
			m_cn_scount = 0;
			m_cn_state = CN_NAT1;
		}
		break;
	case CLA_PTL_STUN_NAT1_ACK:
		{
			on_myaddr(a);
			checknat_end(1);
		}
		break;
	case CLA_PTL_STUN_NAT4_ACK:
		{
			on_myaddr(a);
			if(from.ip==m_stunA.sin_addr.s_addr && from.port==m_stunA.sin_port)
				m_na4_portA = a.port;
			if(from.ip==m_stunB.sin_addr.s_addr && from.port==m_stunB.sin_port)
				m_na4_portB = a.port;
			if(m_na4_portA && m_na4_portB)
			{
				if(m_na4_portA!=m_na4_portB)
					checknat_end(4);
				else
				{
					m_cn_scount = 0;
					m_cn_state = CN_NAT2;
				}
			}
		}
		break;
	case CLA_PTL_STUN_NAT2_ACK:
		checknat_end(2);
		break;
	default:
		DEBUGMSG("# *** unkown packet!!!\n");
		break;
	}
	assert(0==ps.length());
}
void cla_stunClient::on_timer(int e)
{
	switch(e)
	{
	case TIMER_LIVE:
		send_ptl(CLA_PTL_STUN_LIVE,stunA());
		break;
	case TIMER_CHECK_NAT:
		if(CN_IDLE==m_cn_state)
			checknat_begin();
		break;
	case TIMER_NAT_RESEND:
		checknat_resend();
		break;
	default:
		assert(0);
		break;
	}
}
void cla_stunClient::checknat_resend()
{
	////发4个包超时
	switch(m_cn_state)
	{
	case CN_STUNB:
		if(m_cn_scount++ > 3)
			checknat_end(5);
		else
			send_ptl(CLA_PTL_STUN_B,stunA());
		break;
	case CN_NAT1:
		if(m_cn_scount++ > 3)
		{
			m_cn_scount = 0;
			m_cn_state = CN_NAT4;
			checknat_resend();
		}
		else
			send_ptl(CLA_PTL_STUN_NAT1,stunA());
		break;
	case CN_NAT4:
		if(m_cn_scount++ > 3)
			checknat_end(5);
		else
		{
			if(0==m_na4_portA)
				send_ptl(CLA_PTL_STUN_NAT4,stunA());
			if(0==m_na4_portB)
				send_ptl(CLA_PTL_STUN_NAT4,m_stunB);
		}
		break;
	case CN_NAT2:
		if(m_cn_scount++ > 3)
			checknat_end(3);
		else
			send_ptl(CLA_PTL_STUN_NAT2,stunA());
		break;
	default:
		assert(0);
		break;
	}
}

//*****************************************************
void test_stunClient()
{
	char buf[1024];
	int n;
	sockaddr_in from;
	socklen_t fromlen;
	cla_addr a,a2;
	cla_stunClient stunc;
	cl_ptlstream ps;
	
	sockaddr_in addr;
	memset(&addr,0,sizeof(addr));
	socklen_t addrlen = sizeof(addr);
	addr.sin_family = AF_INET;

	fromlen = sizeof(sockaddr_in);
	memset(&from,0,fromlen);
	int fd = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
	cl_net::sock_set_nonblock(fd);

	//bind
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(7532);
	n = bind(fd,(sockaddr*)&addr,addrlen);

	//test proxy
	unsigned int nip;
	unsigned short nport;
	if(0!=cl_net::sock_get_myaddr(fd,nip,nport))
	{
		nport = htons(7532);
	}
	nip = inet_addr("127.0.0.1");
	nport = htons(7532);
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	addr.sin_port = htons(8337);
	buf[0] = CLA_PTL_PROXY_SEND;
	memcpy(buf+1,&nip,4);
	memcpy(buf+5,&nport,2);
	sendto(fd,buf,9,0,(sockaddr*)&addr,addrlen);

	stunc.init(fd,"127.0.0.1:8337");
	while(1)
	{
		if(cl_net::sock_select_readable(fd,0)>0)
		{
			if((n=recvfrom(fd,buf,1024,0,(sockaddr*)&from,&fromlen))>1)
			{
				a.ip = from.sin_addr.s_addr;
				a.port = from.sin_port;
				if(CLA_PTL_PROXY_RECV==buf[0])
				{
					ps.attach(buf+1,n-1,n-1);
					ps >> a2;
					printf("# recv proxy recv from(%s:%d) \n",cl_net::ip_ntoa(a2.ip),(int)ntohs(a2.port));
				}
				else
					stunc.on_data(buf+1,n-1,a);
			}
		}
		cla_timerSngl::instance()->handle_root();
		Sleep(1);
	}
}
