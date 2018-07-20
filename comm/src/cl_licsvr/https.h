#pragma once
#include "cl_httpserver.h"

class https
{
public:
	https(void);
	~https(void);

	int init();
	void fini();
	
	static int on_request(cl_HttpRequest_t* req);

private:
	void index(cl_HttpRequest_t* req);
	void licd(cl_HttpRequest_t* req);
private:
	cl_httpserver m_svr;
};

