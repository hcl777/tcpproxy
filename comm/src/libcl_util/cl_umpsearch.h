#pragma once

#include "cl_thread.h"


//udp multicast protocol search
//�����ԣ��ಥʵ����Ҳֻ����ͬһ�������ڿ����������޷��������η��Ͷಥ��

class cl_umpsearch : public cl_thread
{
public:
	cl_umpsearch(void);
	virtual ~cl_umpsearch(void);

public:
	int run(const char* multiaddr);
	void end();
	virtual int work(int e);

	static int find(const char* multiaddr);
private:
	bool				m_brun;
	char				m_multiip[64];
	unsigned short		m_port;
};

int cl_umpsarech_main(int argc,char** argv);

