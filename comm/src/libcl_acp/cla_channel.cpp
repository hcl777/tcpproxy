#include "cla_channel.h"
#include "cl_net.h"

//回复ACK定时器间隔 20毫秒
#define CLA_ACK_TIMER_TICK 20

cla_channel::cla_channel(cla_connector* ctr,int idx)
:cla_channeli(idx)
,m_ctr(ctr)
,m_is_reg_sndtimer(false)
{
}

cla_channel::~cla_channel(void)
{
}
void cla_channel::reset()
{
	m_snd.reset();
	m_rcv.reset();
	
	for(senditer it=m_sndls.begin();it!=m_sndls.end(); ++it)
		(*it).block->free(CLA_MPTHREAD_CORE);
	m_sndls.clear();
	for(recviter it=m_rcvmap.begin();it!=m_rcvmap.end();++it)
		it->second.block->free(CLA_MPTHREAD_CORE);
	m_rcvmap.clear();

	cla_timerSngl::instance()->unregister_all(static_cast<cla_timerHandler*>(this));
	m_is_reg_sndtimer = false;
	cla_channeli::reset();
}
void cla_channel::set_addr(cla_addr &to,cla_addr &proxy)
{
	memset(&m_addr,0,sizeof(m_addr));
	m_addr.sin_family = AF_INET;
	if(proxy.ip>0)
	{
		m_proxyhead_len = CLA_PTL_PROXY_HEAD_LENGTH;	
		m_addr.sin_port = proxy.port;
		m_addr.sin_addr.s_addr = proxy.ip;
		to.ntype = 0; //
	}
	else
	{
		m_proxyhead_len = 0;	
		m_addr.sin_port = to.port;
		m_addr.sin_addr.s_addr = to.ip;
	}
	m_ndes = to;
}
int cla_channel::on_connected()
{
	DEBUGMSG("# cla_channel::on_connected() \n");

	//初始化发送头，未墙入数据头部
	cl_ptlstream ps(m_buf_sndh,1024,0);
	if(m_proxyhead_len)
	{
		ps << CLA_PTL_PROXY_SEND;
		ps << m_ndes;
	}
	ps << CLA_PTL_CNN_DATA;
	ps << m_s;

	m_state = CLA_CONNECTED;
	cla_timerSngl::instance()->register_timer(static_cast<cla_timerHandler*>(this),2,CLA_ACK_TIMER_TICK);
	call(cla_channelListener::Connected(),static_cast<cla_channeli*>(this));
	return 0;
}
void cla_channel::on_disconnected()
{
	DEBUGMSG("# cla_channel::on_disconnected() \n");
	reset();
	call(cla_channelListener::Disconnected(),static_cast<cla_channeli*>(this));
}

//-------------------------------------------------------------------
//app call
int cla_channel::attach(cla_addr &to,cla_addr &proxy)
{
	DEBUGMSG("# cla_channel::attach(to[%s],proxy[%s]) \n",cl_net::ip_ntoas(to.ip,to.port).c_str(),cl_net::ip_ntoas(proxy.ip,proxy.port).c_str());
	assert(CLA_DISCONNECTED==m_state);
	set_addr(to,proxy);
	m_state = CLA_CONNECTING;
	return 0;
}
int cla_channel::connect(cla_addr &to,cla_addr &proxy)
{
	DEBUGMSG("# cla_channel::connect(to[%s],proxy[%s]) \n",cl_net::ip_ntoas(to.ip,to.port).c_str(),cl_net::ip_ntoas(proxy.ip,proxy.port).c_str());
	assert(CLA_DISCONNECTED==m_state);
	set_addr(to,proxy);
	if(0!=m_ctr->connect(static_cast<cla_channeli*>(this),to.ntype))
	{
		assert(0);
		return -1;
	}
	m_state = CLA_CONNECTING;
	return 0;
}
int cla_channel::disconnect()
{
	if(CLA_CONNECTED == m_state)
	{
		//全部未发的数据发一遍,但并不保证数据完全到达
		
	}
	if(CLA_DISCONNECTED!=m_state)
	{
		m_ctr->disconnect(this);
		on_disconnected();
	}
	return 0;
}
int cla_channel::send(cla_memblock *b,bool more/*=false*/)
{
	if(CLA_CONNECTED!=m_state)
	{
		b->free(CLA_MPTHREAD_CORE);
		return -1;
	}
	cla_sendpacket_t sp;
	sp.seq = m_snd.seq++;
	sp.block = b;
	unsigned int lseq = cl_bstream::htol32(sp.seq);//使用小序
	memcpy(b->buffer,m_buf_sndh,m_proxyhead_len + CLA_PTL_CNN_HEAD_LENGTH);
	memcpy(b->buffer+m_proxyhead_len + CLA_PTL_CNN_HEAD_LENGTH,&lseq,4);
	m_sndls.push_back(sp);
	if(!m_is_reg_sndtimer)
	{
		cla_timerSngl::instance()->register_timer(static_cast<cla_timerHandler*>(this),1,1);
		m_is_reg_sndtimer = true;
	}
	return (m_sndls.size()<m_max_sndls_size?0:1);
}
void cla_channel::update_recv_win_num(int cache_block_num)
{
	m_rcv.win_num = m_max_rcvls_size - cache_block_num;
}

int	cla_channel::set_bandwidth(int size)
{
	return 0;
}
int cla_channel::get_sendspeed(cla_sendspeed_t& s)
{
	return 0;
}
int	cla_channel::get_recvspeed(cla_recvspeed_t& s)
{
	return 0;
}

//connector call
int cla_channel::on_recv_data(cla_memblock* b)
{
	//要处理b的释放
	//b->free(CLA_MPTHREAD_CORE);
	return 1;
}
int cla_channel::on_recv_data_ack(cla_memblock* b)
{
	return 0;
}

void cla_channel::on_timer(int e)
{
	switch(e)
	{
	case 1:
		timer_send_data();
		break;
	case 2:
		timer_send_ack();
		break;
	default:
		break;
	}
}
void cla_channel::timer_send_data()
{
}
void cla_channel::timer_send_ack()
{
}

