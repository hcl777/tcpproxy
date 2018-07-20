#pragma once

#include "cl_httprsp.h"
#include "cl_synchro.h"
#include "cl_thread.h"

class cl_httpserver : public cl_thread
{
public:
	cl_httpserver(void);
	~cl_httpserver(void);
	typedef struct tag_task_info
	{
		int state; //0空闲,1有任务
		SOCKET s;
		sockaddr_in addr;
		cl_httprsp* h;
		cl_Semaphore sem;
	}task_info_t;

	//计算IP数
	typedef struct tag_ipcount
	{
		unsigned int nip;
		int count;
	}ipcount_t;

public:
	int open(unsigned short port,const char *ip=0,FUN_HANDLE_HTTP_REQ_PTR fun=0,void* fun_param=NULL,bool multi_thread=true,int max_client_num=20,const char* strver=NULL);
	int stop();
	bool is_open() const {return m_brun;}

	virtual int work(int e);

	int get_client_num()const {return m_client_num;}
	int get_ip_num()const {return m_client_ips.size();}
private:
	int accpet_root();
	int get_idle();
	
	void add_client_ip(unsigned int nip);
	void del_client_ip(unsigned int nip);
private:
	char			m_ip[64];
	bool			m_brun;

	int				m_sock;
	task_info_t**	m_ti;
	bool			m_multi_thread;
	int				m_max_client_num;
	cl_SimpleMutex	m_mt;
	int				m_client_num;
	list<ipcount_t>	m_client_ips;
	FUN_HANDLE_HTTP_REQ_PTR m_fun_handle_http_req;
	void*			m_fun_param;
	cl_httppubconf_t	m_conf;
};

