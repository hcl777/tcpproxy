#include "uac_UDPSpeedCtrl.h"
#include "uac_SimpleString.h"

namespace UAC
{

UDPSpeedCtrl::UDPSpeedCtrl(void)
	: send_win(100000)
	, speed_timeout_ms(160)
	, speed_last_tick_ms(GetTickCount())
	, lose_rate_inc_max(g_udps_conf.max_lose_rate*(double)0.7/100+0.01)
	, lose_rate_sub_min(g_udps_conf.max_lose_rate/(double)100)
	, const_lose_rate(g_udps_conf.const_send_lose_rate/(double)100)
	, const_speedB(g_udps_conf.const_send_speedB)
	, max_speedB(20480000)  //���20M
	, min_speedB(20240)		//��С20K
	, speedB(80000)
	, ttlus(100000)
	, mtu(0)
	, last_recv_ack_tick(0)
	, bsend_enough_data(true)

	, last_recv_speedB(0)
	, last_recv_speed_i(0)

	, lose_num(0)

{
	csc.begin_ms = speed_last_tick_ms;
	csc.begin_speedB = csc.end_speedB = speedB;
	csc.begin_validspeedB = speedB;
	last_csc = csc;
}
UDPSpeedCtrl::~UDPSpeedCtrl(void)
{
	
}

void UDPSpeedCtrl::get_speed_info(int send_cycle_times,int recv_cycle_times,double& lose_rate,unsigned int& sendspeedB,unsigned int& validspeedB)
{
	//һ�㷢�����ڴ����Ƚ������ڴ�����1����Ϊ���һ���ڵķ����ٶȽ��ն˻�ûͳ����ɣ����Է��Ͷ�һ���ڣ����յ����ö�Ӧ���͵�ǰN-1������
	sendspeedB = send_speed.speed.get_rate(send_cycle_times);
	validspeedB = recv_speed.get_rate(recv_cycle_times);
	if(sendspeedB>validspeedB)
		lose_rate = (sendspeedB-validspeedB)/(double)sendspeedB;
	else
		lose_rate = 0;
	//�����ǰ�ٶȵ�����������nak����ʧ��������ֻȡ1/3
	if(0==csc.lose_num)
		lose_rate /= 3;
	//else
	//	lose_rate = (lose_rate + csc.lose_num*mtu / sendspeedB) / 2;
}

void UDPSpeedCtrl::check_change_speed(unsigned int curr_tick)
{
	//���ٷ���
	if(const_speedB)
	{
		speedB = const_speedB;
		return;
	}
	//������͹����ݲ�ִ��
	if(last_recv_ack_tick+(2*ttlus/1000)+200<curr_tick)
		return;

	double last_lose_rate = 0;
	unsigned int last_sendspeedB = 0;
	unsigned int last_validspeedB = 0;
	//if(lose_num<1)
	if(csc.level == LEVEL_INC_FIRST)
	{
		//ÿ��speed_timeout_ms��������20%
		if(recv_speed.get_amount()<2)
		{
			last_lose_rate = 0;
			last_sendspeedB = last_validspeedB = speedB;
		}
		else
			get_speed_info(5,4,last_lose_rate,last_sendspeedB,last_validspeedB);
			
		csc.end_ms = curr_tick;
		csc.end_sendspeedB = last_sendspeedB;
		csc.end_validspeedB = last_validspeedB;
		csc.end_lose_rate = last_lose_rate;
		last_csc = csc;
		last_csc.analyze();

		csc.reset();
		csc.level = LEVEL_INC_FIRST;
		csc.begin_ms = curr_tick;
		csc.begin_speedB = speedB;
		csc.begin_sendspeedB = last_sendspeedB;
		csc.begin_validspeedB = last_validspeedB;
		csc.begin_lose_rate = last_lose_rate;

		//***********************************************
		if(last_lose_rate>0.10)
		{
			csc.level = LEVEL_INC_FAST;
			//
			speedB = (unsigned int)(last_csc.end_sendspeedB*1.05); //���ʵ�ʽ����ٶ�
			UACLOG("# first inc done!!! \n");
		}
		else
		{
			speedB = (unsigned int)(last_csc.end_sendspeedB*1.3); //���ʵ�ʽ����ٶ�
			//printf("# +++++++++++++++ speedB : %d KB   %d  %d \n",speedB/1024,recv_speed.get_amount(),last_csc.end_sendspeedB);
		}
		if(speedB<min_speedB) speedB = min_speedB;
		//***********************************************

		csc.end_speedB = speedB;

	}
	else
	{
		//600�������һ��
		unsigned int timout = 300;
		if(csc.level==LEVEL_INC_FAST ||csc.level==LEVEL_SUB_FAST)
			timout = 400;
		if(csc.begin_ms + timout>curr_tick)
			return;
		get_speed_info(7,6,last_lose_rate,last_sendspeedB,last_validspeedB);

		csc.end_ms = curr_tick;
		csc.end_sendspeedB = last_sendspeedB;
		csc.end_validspeedB = last_validspeedB;
		csc.end_lose_rate = last_lose_rate;
		last_csc = csc;
		last_csc.analyze();

		csc.reset();
		csc.begin_ms = curr_tick;
		csc.begin_speedB = speedB;
		csc.begin_sendspeedB = last_sendspeedB;
		csc.begin_validspeedB = last_validspeedB;
		csc.begin_lose_rate = last_lose_rate;

		//***********************************************
		if(const_lose_rate>0)
		{
			speedB = (unsigned int)(last_csc.end_validspeedB * (1+const_lose_rate));
			last_csc.level=LEVEL_CONST_LOSE_RATE;
			//UACLOG("# const_lose_rate=%.2f last_csc.end_validspeedB = %d \n",const_lose_rate,last_csc.end_validspeedB);
		}
		else
		{
			//�㷨����������󶪰��ʲ�����3%
			//���������յ�����������ʵ�����ٶȱ��н����ٶȲ���1.02����Ϊ�޶���
			if(last_csc.lose_num<1 && last_csc.end_sendspeedB/(double)last_csc.end_validspeedB < 1.02)
			{
				//�޶�������10%
				if(last_csc.level==LEVEL_INC_FAST)
					speedB = (unsigned int)(last_csc.end_sendspeedB*1.07); //���ʵ�ʽ����ٶ�
				else
					speedB = (unsigned int)(last_csc.end_sendspeedB*1.12); //���ʵ�ʽ����ٶ�
				if(speedB<min_speedB) speedB = min_speedB;
				csc.level = LEVEL_INC_FAST;
			}
			else
			{
				//�������ʴ������
				if(last_csc.end_lose_rate>lose_rate_sub_min) //lose_rate_sub_minΪ������󶪰���
				{
					//speedB = (unsigned int)(last_csc.end_validspeedB * (1+lose_rate_sub_min*0.7)); //���ʵ�ʽ����ٶ�
					if(last_csc.level==LEVEL_INC_FAST)
						speedB = (unsigned int)(last_csc.end_sendspeedB*0.95); //���ʵ�ʽ����ٶ�
					else
						speedB = (unsigned int)(last_csc.end_sendspeedB * 0.9); //
					if(speedB<min_speedB) speedB = min_speedB;
					csc.level = LEVEL_SUB_FAST;
				}
				else
				{
					//�ж��ϴ������ٻ��Ǽ�������ʵ�����ٶȶԱ�Ϊ׼�������ϴ�ʵʩ��������Ϊ׼
					if(last_csc.send_flag>0)
					{
						if(last_csc.valid_flag>0)
						{
							//�����ٶ�����������
							inc_slow_speed();
						}
						else
						{
							//�����ٶȲ�������½�������
							sub_slow_speed();
						}
					}
					else if(last_csc.send_flag==0)
					{
						if(last_csc.valid_flag>=0)
						{
							//����
							inc_slow_speed();
						}
						else
						{
							//����
							sub_slow_speed();
						}
					}
					else
					{
						//���˷����ٶ�
						if(last_csc.valid_flag>0)
						{
							//����
							inc_slow_speed();
						}
						else if(last_csc.valid_flag==0)
						{
							//�����ٶ�����ʧ
							sub_slow_speed();
						}
						else
						{
							if(last_csc.lose_rate_flag<=0)
							{
								//�����ٶ�����ʧ,����������������¼���
								inc_slow_speed();
							}
						}
					}
				}
			}
		}
		//***********************************************
		csc.end_speedB = speedB;

		// +/- b/e:(end_lose_rat,begin_change_rate,change_speedKB,end_speedKB) - s,r(end_sendspeed<B,end_validspeed<B)
		if(3==g_udps_conf.debug_msg_type)
		{
			static string sflag[6]={"+++","++","--","+","-","=="};
			UACLOG(" %s(%s) b/e:( %d%% ,[ %d ], %d ) - s,r( %d , %d ) \n"
				,sflag[last_csc.level].c_str(),last_csc.end_sendspeedB>last_csc.begin_sendspeedB?"+":(last_csc.end_sendspeedB<last_csc.begin_sendspeedB?"-":"=")
				,(int)(last_csc.end_lose_rate*100),(int)(last_csc.end_speedB-last_csc.begin_speedB)>>10,last_csc.end_speedB>>10
				,last_csc.end_sendspeedB>>10,last_csc.end_validspeedB>>10
				);
		}
	}
}


}

