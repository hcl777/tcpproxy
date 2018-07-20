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
	//ע�� seq=0 ��Ч,����
	//����ͳ�Ʒ���֤���ٶȾ�ȷ�������򵽴�˳��һ��ʱ������Ԥ���д�
	if(seq!=cycle_seq)
	{
		//ͳ���ٶ�Ȼ���ʼ��
		unsigned int t = (unsigned int)((end_tick-begin_tick)/1000);
		//����һ�ν��ռ������2��,������һ�ε����ݲſ���Ҫ
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
		num = 1; //ͳ�ƽ�������ʱ����
		sizeB = 0; //��1���������ٶ�ͳ��
	}
	else if(0!=seq)
	{
		end_tick = utick;
		num++;
		sizeB += size;
	}
}