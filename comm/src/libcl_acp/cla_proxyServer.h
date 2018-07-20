#pragma once
#include "cl_basetypes.h"
//#include "cl_incnet.h"

class cla_proxyServer
{
public:
	cla_proxyServer(void);
	virtual ~cla_proxyServer(void);

public:
	int open(unsigned short port);
	void close();
	int loop();

private:
	bool m_brun;
	int m_fd;
};

void cla_test_proxyServer(const char* ipport,bool loop);

