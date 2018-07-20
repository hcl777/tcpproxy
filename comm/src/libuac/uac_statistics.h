#pragma once
#include "uac_Speedometer.h"
#include "uac_Singleton.h"
#include "uac_Timer.h"
#include "uac_ntypes.h"

namespace UAC
{

class statistics : public TimerHandler
{
public:
	statistics(void);
	~statistics(void);
public:
	int init();
	void fini();
	virtual void on_timer(int e);

	unsigned int get_max_sendsize();
	unsigned int get_max_recvsize();
private:
	unsigned int get_max_limit(int second_limit,int last);
public:
	Speedometer<uint64> m_sendspeed; //总发送速度
	Speedometer<uint64> m_recvspeed; //总接收速度
private:
	DWORD		m_last_tick;
};
typedef Singleton<statistics> statisticsSngl;

}

