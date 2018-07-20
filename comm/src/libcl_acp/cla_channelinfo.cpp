#include "cla_channelinfo.h"


void cla_recvcycle::reset()
{
	cycle_seq		= 0;
	begin_tick		= 0;
	end_tick		= 0;
	num				= 0;
	sizeB			= 0;

	last_cycle_seq	= 0;
	last_num		= 0;
	last_speedB		= 0;
}
void cla_recvcycle::on_recv(ULONGLONG utick,unsigned char seq,unsigned int size)
{
	//注意 seq=0 无效,空闲
	//这种统计法保证了速度精确，但包序到达顺序不一致时，丢率预估有错
	if(seq!=cycle_seq)
	{
		//统计速度然后初始化
		unsigned int t = (unsigned int)((end_tick-begin_tick)/1000);
		//离上一次接收间隔不到2秒,这样上一次的数据才考虑要
		if(cycle_seq>0 && sizeB>0 && t>1 && end_tick+2000000>utick)
		{
			last_cycle_seq = cycle_seq;
			last_num = num;
			last_speedB = (unsigned int)(sizeB*(1000/(double)t));
		}
		else
		{
			last_cycle_seq = 0;
			last_num = 0;
			last_speedB = 0;
		}
		begin_tick = end_tick = utick;
		cycle_seq = seq;
		num = 1; //统计接收数据时包含
		sizeB = 0; //第1个包不作速度统计
	}
	else if(0!=seq)
	{
		end_tick = utick;
		num++;
		sizeB += size;
	}
}