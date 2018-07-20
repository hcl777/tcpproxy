#pragma once
#include "cla_mempool.h"
#include "cla_channelspd.h"
#include "cla_protoCnn.h"

//���Ͷ���
typedef struct tag_cla_sendpacket
{
	unsigned int	seq;	//���к�
	short			send_count;	//���ʹ���
	short			nak_count; //Ԥ�ƶ�ʧ
	ULONGLONG		last_send_tick; //���ڼ�¼��ʼ���Ͱ�ʱ�䣬���յ�ACKʱ������ͳ�Ƹð�һ��������ʱ����
	cla_memblock	*block;
	tag_cla_sendpacket(void) : seq(0),send_count(0),nak_count(0),last_send_tick(0),block(0) {}
}cla_sendpacket_t;

//���ն���
typedef struct tag_cla_recvpacket
{
	unsigned int	seq;	//���к�
	ULONGLONG		recv_utick; //����ʱ��
	int				ack_count; //�ظ�ACK����,����յ��ظ���,���ƻ���0
	cla_memblock	*block;//����head
	tag_cla_recvpacket(void) : seq(0),recv_utick(0),ack_count(0),block(0) {}
}cla_recvpacket_t;

typedef struct tag_cla_sendinfo
{
	unsigned int	seq;//�´η����°�ʹ�õ����к�
	unsigned int	win_num; //��need_seq��ʼ���ɷ��͵ĸ���
	unsigned int	need_seq;//�����½磬�Է�δȷ�ϵ��½磬���Է���Ҫ����һ������
	unsigned int	max_send_seq;//��ʵ�Ѿ����͵���λ�ã������ϲ�෢���ݴ����
	unsigned int	ack_seq; //�Է��ظ���ack���кţ��ж�ACK�Ƿ�ʧ
	cla_channelspd	spd;	//�ٶȿ���
	
	unsigned char	not_more_data_count; //�޳������ӷ�������ʱ����ʱ�ط����ڱ��
	void reset()
	{
		seq				= 0; //��ʱ��0��ʼ���ͣ�������������Ҫ��syn��syn_ack�����н���
		win_num			= 10;
		need_seq		= 0;
		max_send_seq	= need_seq-1;
		ack_seq			= 0;
		not_more_data_count = 0;
	}
	tag_cla_sendinfo(void){ reset();}
}cla_sendinfo_t;

/*
���ն�ͳ������
�����ٶȣ�������������
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
	unsigned int			min_seq;//�����½磬δȷ�ϵ��½�
	unsigned int			win_num; //������δӦ�ò���յ�����

	cla_recvcycle			cyc;

	unsigned int			ack_seq;//����ack��seq
	cla_ptl_cnn_data_ack_t	ack;
	unsigned int			last_ack_tick;
	unsigned int			unack_recv_num; //δ�ظ�ACKǰ�յ��İ���
	unsigned int			last_ack_need_seq; //���һ�λس̵�need_seq,
	int						ack_resend_count; //����ͬһ��need_seq��ACK����

	//�ṹ����ACK�ظ�����ttl����û��ACK�ɻظ�ʱ����ظ�һ�����������ٻظ������������Ų�˳�����
	struct {
		ULONGLONG utick;
		unsigned int seq;
	} last_pack;  

	void reset()
	{
		min_seq = 0;
		win_num = 1000; //Լ1M����
		cyc.reset();
		ack_seq = 0;
		last_ack_tick = 0;
		unack_recv_num = 0;
		last_ack_need_seq = 0;
		ack_resend_count = 0;
	}
	tag_cla_recvinfo(void) {reset();}
}cla_recvinfo_t;


