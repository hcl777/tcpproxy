#pragma once
#include "cl_thread.h"

typedef int (*CLFUNC_ON_HTTP_DATA)(void* param,const char* buf,int size);
class cl_sourceHttp : public cl_thread
{
public:
	cl_sourceHttp(void);
	virtual ~cl_sourceHttp(void);

public:
	int open(const char* url,CLFUNC_ON_HTTP_DATA on_data,void* on_data_param);
	int close();
	virtual int work(int e);
	bool is_open() const {return m_brun;}
private:
	bool m_brun;
	CLFUNC_ON_HTTP_DATA m_func_on_data;
	void* m_func_param;
	char m_url[1024];
	int m_fd;
};

