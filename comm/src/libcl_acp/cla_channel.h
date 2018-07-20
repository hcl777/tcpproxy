#pragma once
#include "cla_protoCnn.h"
#include "cla_connector.h"
#include "cla_channelinfo.h"

class cla_channel : public cla_channeli,public cla_timerHandler
{
public:
	cla_channel(cla_connector* ctr,int idx);
	virtual ~cla_channel(void);
	virtual void reset();

	typedef list<cla_sendpacket_t> sendlist;
	typedef sendlist::iterator senditer;
	typedef map<unsigned int,cla_recvpacket_t> recvmap;
	typedef recvmap::iterator recviter;
public:
	//app call
	virtual int attach(cla_addr &to,cla_addr &proxy);
	virtual int connect(cla_addr &to,cla_addr &proxy);
	virtual int disconnect();
	virtual int send(cla_memblock *b,bool more=false);  //-1:false; 0:send ok; 1:put int sendlist
	virtual void update_recv_win_num(int cache_block_num); //cache_block_num表示sockpool还剩多少数据未被读

	virtual int			set_bandwidth(int size); //设置参考带宽
	virtual int			get_sendspeed(cla_sendspeed_t& s);
	virtual int			get_recvspeed(cla_recvspeed_t& s);
	virtual int			get_sndls_size() {return m_sndls.size();}

	//connector call
	virtual int on_connected();
	virtual int on_recv_data(cla_memblock* b);
	virtual int on_recv_data_ack(cla_memblock* b);

	virtual void	on_timer(int e);
	void			timer_send_data();
	void			timer_send_ack();
private:
	void set_addr(cla_addr& to,cla_addr& proxy);
	void on_disconnected();

private:
	cla_connector*		m_ctr;
	cla_sendinfo_t		m_snd;
	cla_recvinfo_t		m_rcv;
	sendlist			m_sndls;
	recvmap				m_rcvmap;
	bool				m_is_reg_sndtimer; //是否注册了发送定时器
	char				m_buf_sndh[1024];
};


