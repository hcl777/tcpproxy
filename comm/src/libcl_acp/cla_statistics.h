#pragma once
#include "cla_timer.h"
#include "cl_speedometer.h"
#include "cl_ntypes.h"
#include "cla_singleton.h"

class cla_statistics: public cla_timerHandler
{
public:
	cla_statistics(void);
	~cla_statistics(void);
public:
	int init();
	void fini();
	virtual void on_timer(int e);

	unsigned int get_max_sendsize();
	unsigned int get_max_recvsize();
private:
	unsigned int get_max_limit(int second_limit,int last);
public:
	cl_speedometer<uint64> m_sendspeed; //总发送速度
	cl_speedometer<uint64> m_recvspeed; //总接收速度
private:
	DWORD		m_last_tick;
};

typedef cla_singleton<cla_statistics> cla_statisticsSngl;
