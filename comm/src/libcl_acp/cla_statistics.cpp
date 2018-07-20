#include "cla_statistics.h"
#include "cla_config.h"

cla_statistics::cla_statistics(void)
	:m_last_tick(0)
{
}


cla_statistics::~cla_statistics(void)
{
}
int cla_statistics::init()
{
	cla_timerSngl::instance()->register_timer(static_cast<cla_timerHandler*>(this),1,1000);
	return 0;
}
void cla_statistics::fini()
{
	cla_timerSngl::instance()->unregister_all(static_cast<cla_timerHandler*>(this));
}
void cla_statistics::on_timer(int e)
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
unsigned int cla_statistics::get_max_limit(int second_limit,int last)
{
	if(0==second_limit)
		return 0x7fffffff;
	DWORD tick = _timer_distance(GetTickCount(),m_last_tick);
	if(tick>1000) tick = 1000;
	int n = (int)(second_limit * (tick/(double)1000))+1500;
	if(n<last) return 0;
	return n-last;
}
unsigned int cla_statistics::get_max_sendsize()
{
	return get_max_limit(g_cla_conf.limit_sendspeed,(int)m_sendspeed.get_last());
}
unsigned int cla_statistics::get_max_recvsize()
{
	return get_max_limit(g_cla_conf.limit_recvspeed,(int)m_recvspeed.get_last());
}
