#pragma once
#include "cla_mempool.h"
#include "clacp.h"
#include "cl_synchro.h"
#include "cla_channel.h"
#include "cla_timer.h"
#include "cla_connector.h"
#include "cl_thread.h"

#define CLA_SOCK_IDEL_TICK 60000


typedef cl_SimpleMutex				SMutex;
typedef cl_TLock<cl_SimpleMutex>	SLock;

//用户sock 状态
enum SOCKSTATE
{ 
	SOCK_IDLE=0,
	SOCK_WAIT_CONNECTING,
	SOCK_CONNECTING,
	SOCK_WAIT_ACCEPTING,
	SOCK_ACCEPTING,
	SOCK_CONNECTED,
	SOCK_DISCONNECTING
};
typedef struct tag_cla_sockhandle
{
	//注意: 只有上层改变
	SMutex				mt;
	int					sock_state;			//空闲，正在连接，正在接受，连接完成
	char				close_state;		//应用层和底层关闭
	cla_addr			to;
	cla_addr			proxy;
	cla_channeli*		ch;
	//unsigned int		max_sendbufsize;
	list<cla_memblock*> sendlist;
	list<cla_memblock*> recvlist;
	DWORD				last_active_tick;
	
	tag_cla_sockhandle(void)
		:sock_state(SOCK_IDLE)
		,close_state(0)
		,last_active_tick((DWORD)-CLA_SOCK_IDEL_TICK)
	{
	}
}cla_sockhandle_t;

class cla_sockpool : public cla_timerHandler
	,public cla_channelListener
	,public cla_cnnFactory
	,public cl_thread
{
public:
	cla_sockpool(void);
	~cla_sockpool(void);

public:
	int			init(unsigned short port,const char* stunsvr);
	void		fini();
	void		set_mtu(int mtu) { m_ctr.set_mtu(mtu);}

	CLA_SOCKET	accept(cla_addr* from);
	CLA_SOCKET	connect(const cla_addr* to,const cla_addr* proxy);
	int			closesocket(CLA_SOCKET fd);
	int			select(cla_fdset* rset,cla_fdset* wset);
	int			send(CLA_SOCKET fd,const char* buf,int len);//返回:-1 关闭
	int			recv(CLA_SOCKET fd,char* buf,int len); //返回:-1 关闭

	bool		is_connected(CLA_SOCKET fd) const { return (SOCK_CONNECTED==m_socks[fd].sock_state); }
	bool		is_write(CLA_SOCKET fd) const {return ( (SOCK_CONNECTED==m_socks[fd].sock_state && (m_socks[fd].sendlist.size()+m_socks[fd].ch->get_sndls_size())<m_socks[fd].ch->m_max_sndls_size) || SOCK_DISCONNECTING==m_socks[fd].sock_state); }
	bool		is_read(CLA_SOCKET fd) const {return (!m_socks[fd].recvlist.empty() || SOCK_DISCONNECTING==m_socks[fd].sock_state);}

	int			set_sendbuf(CLA_SOCKET fd,int size) { if(size<10240)size=10240; m_socks[fd].ch->m_max_sndls_size = size/CLA_MEMBLOCK_SIZE;return 0;}
	int			set_recvbuf(CLA_SOCKET fd,int size) { if(size<20000) size=20000;m_socks[fd].ch->m_max_rcvls_size = size/CLA_MEMBLOCK_SIZE;return 0;}
	int			set_bandwidth(CLA_SOCKET fd,int size) {if(size>0 && size<20000) size=20000;return m_socks[fd].ch->set_bandwidth(size);}
	int			get_sendspeed(CLA_SOCKET fd,cla_sendspeed_t& s){return m_socks[fd].ch->get_sendspeed(s);}
	int			get_recvspeed(CLA_SOCKET fd,cla_recvspeed_t& s){return m_socks[fd].ch->get_recvspeed(s);}

private:
	void			release_fd(CLA_SOCKET fd);
	static void		clear_memblock_list(list<cla_memblock*>& ls,int ithreadtoken);
	int				find_idel_socket();
	void			_handle_send(CLA_SOCKET fd);
public:
	virtual void			on_timer(int e);
	virtual cla_channeli*	cla_attach_channel(cla_addr &to,cla_addr &proxy);
	virtual int				work(int e);

	virtual void on(Connected,cla_channeli* ch);
	virtual void on(Disconnected,cla_channeli* ch);
	virtual void on(Data,cla_channeli* ch,cla_memblock *b);
	virtual void on(Writable,cla_channeli* ch);
	
private:
	SMutex				m_mt;
	bool				m_brun;
	cla_sockhandle_t*	m_socks;
	int					m_socknum; //记录当前连接数
	int					m_maxsock; //记录当前最大的fd
	cla_connector		m_ctr;
	
	list<CLA_SOCKET>	m_ls_accepting; //底层通知上层的消息队列

	//上层通知底层的消息队列
	list<CLA_SOCKET>	m_ls_connecting;
	list<CLA_SOCKET>	m_ls_disconnecting;
	list<CLA_SOCKET>	m_ls_sending;

};

typedef cla_singleton<cla_sockpool> cla_sockpoolSngl;
