#include "cl_sourceHttp.h"
#include "Httpc.h"

cl_sourceHttp::cl_sourceHttp(void)
	:m_brun(false)
	,m_fd(0)
{
}


cl_sourceHttp::~cl_sourceHttp(void)
{
}

int cl_sourceHttp::open(const char* url,CLFUNC_ON_HTTP_DATA on_data,void* on_data_param)
{
	if(m_brun)
		return 1;
	strcpy(m_url,url);
	m_func_on_data = on_data;
	m_func_param = on_data_param;
	m_brun = true;
	this->activate();
	return 0;
}
int cl_sourceHttp::close()
{
	if(m_brun)
	{
		m_brun = false;
		int fd = m_fd;
		m_fd = 0;
		if(fd>0)
		{
			Httpc::http_close(fd);
		}
		wait();
	}
	return 0;
}
int cl_sourceHttp::work(int e)
{
	Httpc::httpc_response_t rsp;
	char *buf = new char[10240];
	int size;
	while(m_brun)
	{
		m_fd = Httpc::http_open_request(m_url,NULL,0);
		if(m_fd<=0)
		{
			Sleep(2000);
			continue;
		}
		if(0==Httpc::http_recv_header(m_fd,&rsp))
		{
			if(200==rsp.retcode || 206==rsp.retcode)
			{
				if(rsp.bodylen)
					this->m_func_on_data(m_func_param,rsp.body,(int)rsp.bodylen);
				while(m_brun)
				{
					size=Httpc::http_recv_data(m_fd,buf,10240);
					if(0==size)
						break;
					if(size>0)
						this->m_func_on_data(m_func_param,buf,size);
				}
			}
			else
			{
			}
		}
		if(m_fd)
			Httpc::http_close(m_fd);
		if(m_brun)
			Sleep(2000);
	}

	delete[] buf;
	return 0;
}
