#pragma once
#include "cla_mempool.h"
#include "cla_channelspd.h"
#include "cla_protoCnn.h"

//发送队列
typedef struct tag_cla_sendpacket
{
	unsigned int	seq;	//序列号
	short			send_count;	//发送次数
	short			nak_count; //预计丢失
	ULONGLONG		last_send_tick; //用于记录开始发送包时间，当收到ACK时，可以统计该包一个来回用时多少
	cla_memblock	*block;
	tag_cla_sendpacket(void) : seq(0),send_count(0),nak_count(0),last_send_tick(0),block(0) {}
}cla_sendpacket_t;

//接收队列
typedef struct tag_cla_recvpacket
{
	unsigned int	seq;	//序列号
	ULONGLONG		recv_utick; //接收时间
	int				ack_count; //回复ACK次数,如果收到重复包,将计划置0
	cla_memblock	*block;//不带head
	tag_cla_recvpacket(void) : seq(0),recv_utick(0),ack_count(0),block(0) {}
}cla_recvpacket_t;

typedef struct tag_cla_sendinfo
{
	unsigned int	seq;//下次发送新包使用的序列号
	unsigned int	win_num; //从need_seq开始，可发送的个数
	unsigned int	need_seq;//窗口下界，对方未确认的下界，即对方想要的下一个数据
	unsigned int	max_send_seq;//真实已经发送到的位置，可能上层多发，暂存队列
	unsigned int	ack_seq; //对方回复的ack序列号，判断ACK是否丢失
	cla_channelspd	spd;	//速度控制
	
	unsigned char	not_more_data_count; //无持续增加发送数据时，超时重发周期变短
	void reset()
	{
		seq				= 0; //暂时从0开始发送，如果随机，则需要在syn和syn_ack握手中交换
		win_num			= 10;
		need_seq		= 0;
		max_send_seq	= need_seq-1;
		ack_seq			= 0;
		not_more_data_count = 0;
	}
	tag_cla_sendinfo(void){ reset();}
}cla_sendinfo_t;

/*
接收端统计周期
计算速度，包数，重收数
*/
class cla_recvcycle
{
public:
	unsigned char	cycle_seq;
	ULONGLONG		begin_tick;
	ULONGLONG		end_tick;
	unsigned short	num;
	unsigned int	sizeB;

	unsigned char	last_cycle_seq;
	unsigned short	last_num;
	unsigned int	last_speedB;

	cla_recvcycle(void){reset();}
	void reset();
	void on_recv(ULONGLONG utick,unsigned char seq,unsigned int size);
};
typedef struct tag_cla_recvinfo
{
	unsigned int			min_seq;//窗口下界，未确认的下界
	unsigned int			win_num; //减掉被未应用层接收的数量

	cla_recvcycle			cyc;

	unsigned int			ack_seq;//发送ack的seq
	cla_ptl_cnn_data_ack_t	ack;
	unsigned int			last_ack_tick;
	unsigned int			unack_recv_num; //未回复ACK前收到的包数
	unsigned int			last_ack_need_seq; //最后一次回程的need_seq,
	int						ack_resend_count; //发送同一个need_seq的ACK次数

	//结构用于ACK回复计算ttl，当没有ACK可回复时，则回复一个，有则不能再回复这个，否则序号不顺序递增
	struct {
		ULONGLONG utick;
		unsigned int seq;
	} last_pack;  

	void reset()
	{
		min_seq = 0;
		win_num = 1000; //约1M左右
		cyc.reset();
		ack_seq = 0;
		last_ack_tick = 0;
		unack_recv_num = 0;
		last_ack_need_seq = 0;
		ack_resend_count = 0;
	}
	tag_cla_recvinfo(void) {reset();}
}cla_recvinfo_t;


