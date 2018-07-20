#pragma once

typedef void (*CLA_CALLBACK_ONNATOK)(int ); //(int nattype)
typedef void (*CLA_CALLBACK_ONIPPORTCHANGED)(unsigned int ,unsigned short ); //(unsigned int ip,unsigned short port)

typedef struct tag_cla_config
{
	unsigned int	packet_size;
	unsigned int	max_recv_win_num; //最多可接收包数
	unsigned char	max_resend_num;
	unsigned short	max_lose_rate;  //允许实时最大重发率（即低于此值才可能增窗）
	unsigned int	const_send_speedB; //如果非0，定速发送
	unsigned int	const_recv_speedB; //如果非0，定速发送
	unsigned short	const_recv_lose_rate; //如果非0，定丢包率发送，定速优先

	unsigned int	udp_keeplive_timer_msec; //默认5000毫秒
	unsigned int	udp_conn_timeout_msec; //15000毫秒
	unsigned char	debug_msg_type;
	unsigned int	udp_testspeed_num; //测速包数

	unsigned int	limit_sendspeed_i; //每个连接最大限速发送
	unsigned int	limit_sendspeed; //所有连接最大发送总速度
	unsigned int	limit_recvspeed; //所有连接最大接收总速度

	//udp nattype
	int				nattype;
	//callback
	CLA_CALLBACK_ONNATOK callback_onnatok;
	CLA_CALLBACK_ONIPPORTCHANGED callback_onipportchanged;

	tag_cla_config(void)
		:packet_size(0)  //此处赋值包含IP头,ip头20字节（无特殊选行）,UDP头8字节，UDPS头26字节，总大小不超1500字节,
		,max_recv_win_num(1000) //1个pack约1K数据.
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

