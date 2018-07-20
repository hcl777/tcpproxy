#pragma once
#include "cla_timer.h"
#include "cl_incnet.h"
#include "cla_proto.h"

class cla_stunClient : cla_timerHandler
{
public:
	cla_stunClient();
	virtual ~cla_stunClient(void);
	
	enum {TIMER_LIVE=1,TIMER_CHECK_NAT=2,TIMER_NAT_RESEND=3};
	enum {CN_IDLE=0,CN_STUNB,CN_NAT1,CN_NAT4,CN_NAT2};
public:
	int init(int fd,const char* svr);
	void fini();
	sockaddr_in& stunA(); //尝试解释域名
	void on_data(char* buf,int size,const cla_addr& from);
	virtual void on_timer(int e);
private:
	int send_ptl(cla_cmd_t cmd,sockaddr_in& a);
	void on_myaddr(const cla_addr& a);
	void checknat_end(int nattype);
	void checknat_begin();
	void checknat_resend();
private:
	int				m_fd;
	int				m_cn_state; //check nat state
	int				m_cn_scount; //check nat resend count
	unsigned short	m_na4_portA,m_na4_portB; //记录返回的端口
	string			m_stunip;
	cla_addr		m_mya;
	sockaddr_in		m_stunA,m_stunB;
};

//test
void test_stunClient();

