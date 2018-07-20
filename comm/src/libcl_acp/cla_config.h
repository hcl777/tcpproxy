#pragma once

typedef void (*CLA_CALLBACK_ONNATOK)(int ); //(int nattype)
typedef void (*CLA_CALLBACK_ONIPPORTCHANGED)(unsigned int ,unsigned short ); //(unsigned int ip,unsigned short port)

typedef struct tag_cla_config
{
	unsigned int	packet_size;
	unsigned int	max_recv_win_num; //���ɽ��հ���
	unsigned char	max_resend_num;
	unsigned short	max_lose_rate;  //����ʵʱ����ط��ʣ������ڴ�ֵ�ſ���������
	unsigned int	const_send_speedB; //�����0�����ٷ���
	unsigned int	const_recv_speedB; //�����0�����ٷ���
	unsigned short	const_recv_lose_rate; //�����0���������ʷ��ͣ���������

	unsigned int	udp_keeplive_timer_msec; //Ĭ��5000����
	unsigned int	udp_conn_timeout_msec; //15000����
	unsigned char	debug_msg_type;
	unsigned int	udp_testspeed_num; //���ٰ���

	unsigned int	limit_sendspeed_i; //ÿ������������ٷ���
	unsigned int	limit_sendspeed; //����������������ٶ�
	unsigned int	limit_recvspeed; //�����������������ٶ�

	//udp nattype
	int				nattype;
	//callback
	CLA_CALLBACK_ONNATOK callback_onnatok;
	CLA_CALLBACK_ONIPPORTCHANGED callback_onipportchanged;

	tag_cla_config(void)
		:packet_size(0)  //�˴���ֵ����IPͷ,ipͷ20�ֽڣ�������ѡ�У�,UDPͷ8�ֽڣ�UDPSͷ26�ֽڣ��ܴ�С����1500�ֽ�,
		,max_recv_win_num(1000) //1��packԼ1K����.
		,max_resend_num(40)
		,max_lose_rate(20)
		,const_send_speedB(0)
		,const_recv_speedB(0)
		,const_recv_lose_rate(0)

		,udp_keeplive_timer_msec(5000)
		,udp_conn_timeout_msec(12000)
		,debug_msg_type(0)
		,udp_testspeed_num(10)

		,limit_sendspeed_i(0)
		,limit_sendspeed(0)
		,limit_recvspeed(0)

		,nattype(6)
		,callback_onnatok(0)
		,callback_onipportchanged(0)
	{}

}cla_config_t;
extern cla_config_t g_cla_conf;

