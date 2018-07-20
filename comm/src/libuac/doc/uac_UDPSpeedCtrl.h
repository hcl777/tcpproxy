#pragma once
#include <assert.h>
#include "uac_basetypes.h"
#include "uac_UDPProtocol.h"
#include "uac_Speedometer.h"

namespace UAC
{

#define SEND_CYCLE_TIMEOUT 200000

class UDPSSpeed
{
public:
	UDPSSpeed(void)
		:speed_timeout_ms(160)
		,last_tick_ms(GetTickCount())
		,data_size(0)
	{
	}
public:
	void on_data(unsigned int size)
	{
		data_size += size;
	}
	void on_tick(unsigned int tick_ms)
	{
		if(last_tick_ms+speed_timeout_ms<tick_ms)
		{
			unsigned int tmp = tick_ms-last_tick_ms;
			if(tmp>1000) tmp=1000;
			speed.step(data_size*1000/tmp);
			last_tick_ms = tick_ms;
			data_size = 0;
		}
	}
public:
	ArrRuler<unsigned int,18> speed; //speed���ٶ��Ѿ�ת����B/s
private:
	unsigned int speed_timeout_ms;
	unsigned int last_tick_ms;
	unsigned int data_size;
};

class UDPSpeedCtrl
{
public:
	enum {LEVEL_INC_FIRST=0,LEVEL_INC_FAST,LEVEL_SUB_FAST,LEVEL_INC_SLOW,LEVEL_SUB_SLOW,LEVEL_CONST_LOSE_RATE};
	//��¼һ�������ٹ���
	typedef struct tagChspeedcycle
	{
		//flag�� 1�����٣�2������
		unsigned char	level;
		unsigned int	lose_num; //��������
		unsigned int	begin_ms; //��������
		unsigned int	begin_speedB;
		unsigned int	begin_sendspeedB;
		unsigned int	begin_validspeedB; //��һ���ڵ�ƽ���ٶ�
		double			begin_lose_rate;
		//double			begin_change_rate;
		unsigned int	end_ms;
		unsigned int	end_speedB;
		unsigned int	end_sendspeedB;
		unsigned int	end_validspeedB;
		double			end_lose_rate;

		int				send_flag; //-1�����٣�0��ά�ֲ��䣬1������
		int				valid_flag;//-1�����٣�0��ά�ֲ��䣬1������
		int				lose_rate_flag;

		tagChspeedcycle(void):level(LEVEL_INC_FIRST) { reset();}
		void reset()
		{
			lose_num = 0;
			begin_ms = 0;
			begin_speedB = 0;
			begin_sendspeedB = 0;
			begin_validspeedB = 0;
			begin_lose_rate = 0;
			//begin_change_rate = 0;
			end_ms = 0;
			end_speedB = 0;
			end_sendspeedB = 0;
			end_validspeedB = 0;
			end_lose_rate = 0;
			send_flag=0;
			valid_flag=0;
			lose_rate_flag=0;
		}
		void analyze()
		{
			//����ÿ������������3%
			//����1.5%���ڱ���Ϊ�ٶȲ���
			double chd_sendrate = end_sendspeedB/(double)(begin_sendspeedB+1);
			double chd_validrate = end_validspeedB/(double)(begin_validspeedB+1);
			double chd_loserate = end_lose_rate - begin_lose_rate;
			if(chd_sendrate>1.015)
				send_flag = 1;
			else if(chd_sendrate<0.985)
				send_flag = -1;
			else
				send_flag = 0;

			if(chd_validrate>1.02)
				valid_flag = 1;
			else if(chd_validrate<0.98)
				valid_flag = -1;
			else
				valid_flag = 0;

			if(chd_loserate>0.02)
				lose_rate_flag = 1;
			else if(chd_loserate<-0.02)
				lose_rate_flag = -1;
			else
				lose_rate_flag = 0;
		}
	}Chspeedcycle_t;

	//�����ٶ�����
	typedef struct tagSendCycleTimer
	{
		unsigned int cycle_speedB;
		unsigned int cycle_amountB; //send_cycle_timer���ͳ�Ʒ���
		ULONGLONG cycle_begin_utick;
		tagSendCycleTimer(void)
			:cycle_speedB(0)
			,cycle_amountB(0)
			,cycle_begin_utick(0)
		{
		}
	}SendCycleTimer_t;

public:

	void reset_speed(int speed)
	{
		speedB = speed;
		if(speedB>max_speedB)
			speedB = max_speedB;
		else if(speedB<min_speedB)
			speedB = min_speedB;
		recv_speed.reset();
		last_csc.end_sendspeedB = last_csc.end_validspeedB = last_csc.begin_speedB = last_csc.end_speedB = speedB;
	}
	void on_second()
	{
		speedometer.on_second();
	}

	void on_sendsize(unsigned int size)
	{
		//��������ʱ����
		speedometer.add(size); //�ƻ�����(1s��
		send_speed.on_data(size); //ʵʱ�����ٶ�(160ms��
		sct.cycle_amountB += size;
	}
	void on_resendsize(unsigned int size)
	{
	}
	void on_recv_ack(){}

	void on_recv_nak(int num)
	{
		//Ŀǰֻ��nak����ʧʱ�Ż���ã�nak�����ack������ͬһ��UDP������
		if(0==lose_num)
		{
			//����������30%
			speedB = (unsigned int)(speedB*0.7);
			if(speedB<min_speedB) speedB = min_speedB;
		}
		lose_num += num;
		csc.lose_num += num;
	}

	void on_ack_recv_speed(unsigned int speedB,unsigned int speed_i)
	{
		//speedB Ϊ���ն����һ���ٶ�����(160����)���ٶ�
		//speed_i Ϊ���ն˵��ٶȶ��еĵ�ǰ�α꣬���Դ�ʶ��ͬ����
		//speed��ACK�д�����
		last_recv_ack_tick = GetTickCount();
		if(last_recv_speedB!=speedB || last_recv_speed_i!=speed_i)
		{
			recv_speed.step(speedB);
			last_recv_speedB = speedB;
			last_recv_speed_i = speed_i;
		}
	}

	//�����ٶȼ���ɷ��Ͱ���
	int get_max_send_packnum(ULONGLONG curr_utick)
	{
		if(sct.cycle_begin_utick+2*SEND_CYCLE_TIMEOUT<curr_utick)
		{
			//��ʱ���Ƿ���10��������
			//UACLOG("# SEND_CYCLE_TIMEOUT out us \n");
			sct.cycle_amountB = 0;
			sct.cycle_begin_utick = curr_utick - 10000;
		}
		ULONGLONG utick = curr_utick-sct.cycle_begin_utick;
		unsigned int max_sendB = (unsigned int)((speedB*utick/1000000) + 1024);
		if(max_sendB > sct.cycle_amountB)
			max_sendB -= sct.cycle_amountB;
		else
			max_sendB = 0;
		return max_sendB/(mtu+UDPHEAD_LENGTH);
	}

	//curr_tick:����
	void on_tick(unsigned int curr_tick,ULONGLONG curr_utick)
	{
		send_speed.on_tick(curr_tick);
		if(speed_last_tick_ms + speed_timeout_ms < curr_tick)
		{
			speed_last_tick_ms = curr_tick;
			check_change_speed(curr_tick);
		}

		//������������
		if(sct.cycle_speedB != speedB || sct.cycle_begin_utick+SEND_CYCLE_TIMEOUT<curr_utick)
		{
			sct.cycle_speedB = speedB;
			sct.cycle_amountB = 0;
			sct.cycle_begin_utick = curr_utick;
		}
	}
	
	void sub_speed(unsigned int size)
	{
		if(speedB>size)
			speedB -= size;
		if(speedB<min_speedB)
			speedB = min_speedB;
	}
	void inc_slow_speed()
	{
		if(last_csc.end_lose_rate<lose_rate_inc_max)
		{
			speedB = (unsigned int)(last_csc.end_sendspeedB*1.04); 
			csc.level = LEVEL_INC_SLOW;
		}
	}
	void sub_slow_speed()
	{
		if(last_csc.end_lose_rate>0.05)
		{
			speedB = (unsigned int)(last_csc.end_sendspeedB*0.95); 
			if(speedB<min_speedB) speedB = min_speedB;
			csc.level = LEVEL_SUB_SLOW;
		}
	}

	void get_speed_info(int send_cycle_times,int recv_cycle_times,double& lose_rate,unsigned int& sendspeedB,unsigned int& validspeedB);
private:
	void check_change_speed(unsigned int curr_tick);
	//void level_inc_slow2(double lose_rate,unsigned int sendspeedB,unsigned int validspeedB,unsigned int curr_tick);
	//void level_sub_slow2(double lose_rate,unsigned int sendspeedB,unsigned int validspeedB,unsigned int curr_tick);
	//void check_change_speed2(unsigned int curr_tick);


//***************************************************************************************************************
public:
	UDPSpeedCtrl(void);
	~UDPSpeedCtrl(void);
public:
	unsigned int send_win; //���㷨����send_win����
	unsigned int speed_timeout_ms;
	unsigned int speed_last_tick_ms;

	double lose_rate_inc_max;  //�����ٵ���󶪰��ʣ�һ��Ϊ��󶪰��ʵ�70%
	double lose_rate_sub_min;  //�ɼ��ٵ���С�����ʣ�������󶪰���
	double			const_lose_rate;
	unsigned int	const_speedB;
	//�ٶ�Ϊÿ���ٶ�
	unsigned int max_speedB;  
	unsigned int min_speedB;
	unsigned int speedB;
	unsigned int ttlus;
	unsigned int mtu;
	unsigned int last_recv_ack_tick;
	bool bsend_enough_data;

	
	Speedometer<unsigned int,10> speedometer;
private:
	UDPSSpeed send_speed;
	SendCycleTimer_t sct;

	ArrRuler<unsigned int,18> recv_speed;
	unsigned int last_recv_speedB;
	unsigned int last_recv_speed_i;

	unsigned int lose_num;

	Chspeedcycle_t csc;
	Chspeedcycle_t last_csc;
//**********************************************************************************************************************
};

}


