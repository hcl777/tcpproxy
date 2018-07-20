#include "uac_statistics.h"
#include "uac_UDPConfig.h"

namespace UAC
{
statistics::statistics(void)
	:m_last_tick(0)
{
}


statistics::~statistics(void)
{
}

int statistics::init()
{
	TimerSngl::instance()->register_timer(static_cast<TimerHandler*>(this),1,1000);
	return 0;
}
void statistics::fini()
{
	TimerSngl::instance()->unregister_all(static_cast<TimerHandler*>(this));
}
void statistics::on_timer(int e)
{
	switch(e)
	{
	case 1:
		{
			m_last_tick = GetTickCount();
			m_sendspeed.on_second();
			m_recvspeed.on_second();
			//UACLOG("#UAC ----recv_speed=%d KB  send speed=%d KB\n",(int)(m_recvspeed.get_speed(2)>>10),(int)(m_sendspeed.get_speed(2)>>10));
		}
		break;
	default:
		break;
	}
}
unsigned int statistics::get_max_limit(int second_limit,int last)
{
	if(0==second_limit)
		return 0x7fffffff;
	DWORD tick = _timer_distance(GetTickCount(),m_last_tick);
	if(tick>1000) tick = 1000;
	int n = (int)(second_limit * (tick/(double)1000))+1500;
	if(n<last) return 0;
	return n-last;
}
unsigned int statistics::get_max_sendsize()
{
	return get_max_limit(g_uac_conf.limit_sendspeed,(int)m_sendspeed.get_last());
}
unsigned int statistics::get_max_recvsize()
{
	return get_max_limit(g_uac_conf.limit_recvspeed,(int)m_recvspeed.get_last());
}

}
