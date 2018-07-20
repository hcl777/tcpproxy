#include "cla_sockpool.h"
#include "cla_timer.h"
#include "cla_mempool.h"
#include "cla_statistics.h"


#define ERRFD_RETURN(fd,ret) if(fd<0||fd>=CLA_FD_SIZE) return ret
#define MAXSOCK_CLR(s,ms) if(s==ms) {ms--;while(ms>-1 && SOCK_IDLE==m_socks[ms].sock_state) ms--;}

cla_sockpool::cla_sockpool(void)
	:m_brun(false)
	,m_socks(NULL)
	,m_socknum(0)
	,m_maxsock(-1)
{
}

cla_sockpool::~cla_sockpool(void)
{
}

int	cla_sockpool::init(unsigned short port,const char* stunsvr)
{
	if(m_brun) return 1;
	cla_timerSngl::instance();
	cla_mempoolSngl::instance()->init();
	cla_statisticsSngl::instance()->init();

	m_socks = new cla_sockhandle_t[CLA_FD_SIZE];
	for(int i=0;i<CLA_FD_SIZE;++i)
	{
		m_socks[i].ch = new cla_channel(&m_ctr,i);
		m_socks[i].ch->set_listener(static_cast<cla_channelListener*>(this));
	}

	if(0!=m_ctr.init(port,static_cast<cla_cnnFactory*>(this),stunsvr))
	{
		fini();
		return -1;
	}
	m_brun = true;
	activate();
	return 0;
}
void cla_sockpool::fini()
{
	if(m_brun)
	{
		m_brun = false;
		wait();
		m_ctr.fini();
	}
	if(m_socks)
	{
		for(int i=0;i<CLA_FD_SIZE;++i)
		{
			//要关掉所有SOCK_ACCEPTING的连接
			if(SOCK_WAIT_ACCEPTING==m_socks[i].sock_state || SOCK_ACCEPTING==m_socks[i].sock_state || SOCK_DISCONNECTING==m_socks[i].sock_state)
				m_socks[i].ch->disconnect();
			m_socks[i].ch->set_listener(NULL);
			delete m_socks[i].ch;
		}
		delete[] m_socks;
		m_socks = NULL;
		assert(0==m_socknum);
	}
	
	cla_statisticsSngl::instance()->fini();
	cla_mempoolSngl::instance()->fini();

	cla_statisticsSngl::destroy();
	cla_mempoolSngl::destroy();
	cla_timerSngl::destroy();
}

/*
function: 
从accepting接收队列中取出最早的一个，将状态转成连接成功。并返回句柄。
return:
-1表示没有连接接入。
*/
CLA_SOCKET	cla_sockpool::accept(cla_addr* from)
{
	
	list<CLA_SOCKET> ls;
	CLA_SOCKET fd = -1;
	if(!m_ls_accepting.empty())
	{
		{
			SLock l(m_mt);
			if(!m_ls_accepting.empty())
			{
				fd = m_ls_accepting.front();
				m_ls_accepting.pop_front();
			}
			else
				return -1;
		}
		{
			SLock l(m_socks[fd].mt);
			if(SOCK_ACCEPTING==m_socks[fd].sock_state)
			{
				m_socks[fd].sock_state = SOCK_CONNECTED; //accepting 时可能已经有数据，底层只有将连接建完成才会变成accepting
				memcpy(from,&m_socks[fd].to,sizeof(cla_addr));
				return fd;
			}
			else
			{
				//底层已经断开
				return -1;
			}
		}
	}
	return -1;
}
/*

*/
CLA_SOCKET cla_sockpool::connect(const cla_addr* to,const cla_addr* proxy)
{
	CLA_SOCKET fd = -1;
	//不会死循环，
	while(1)
	{
		fd = find_idel_socket();
		if(-1==fd)
		{
			return -1;
		}
		else
		{
			SLock l(m_socks[fd].mt);
			if(SOCK_IDLE!=m_socks[fd].sock_state || CLA_DISCONNECTED!=m_socks[fd].ch->get_state())
				continue;
			m_socks[fd].sock_state = SOCK_WAIT_CONNECTING;
			m_socks[fd].to = *to;
			memset(&m_socks[fd].proxy,0,sizeof(cla_addr));
			if(proxy)
				m_socks[fd].proxy = *proxy;

			m_socks[fd].last_active_tick = GetTickCount();
			break;
		}
	}
	{
		SLock l(m_mt);
		m_socknum++;
		m_ls_connecting.push_back(fd); //传递一个连接消息
	}
	return fd;
}
int	cla_sockpool::closesocket(CLA_SOCKET fd)
{
	//底层主动断开的话，不发消息通知上层，只将状态变为disconnecting
	ERRFD_RETURN(fd,-1);

	//assert(SOCK_IDLE!=m_socks[fd].sock_state);
	SLock l(m_socks[fd].mt);
	if(SOCK_IDLE==m_socks[fd].sock_state)
		return 0;
	m_socks[fd].sock_state = SOCK_DISCONNECTING;
	m_socks[fd].close_state |= 0x01;
	if(m_socks[fd].close_state==0x03)
	{
		release_fd(fd);
	}
	else
	{
		SLock l(m_mt);
		m_ls_disconnecting.push_back(fd);
	}
	return 0;
}

/*
function:
	查询所有可读或可写的SOCK
*/
int	cla_sockpool::select(cla_fdset* rset,cla_fdset* wset)
{
	//这里总是查询所有SOCK的状态返回
	int n = 0;
	if(rset) rset->fd_count = 0;
	if(wset) wset->fd_count = 0;
	for(int i=0;i<(m_maxsock+1);++i)
	{
		if(rset)
		{
			if(!m_socks[i].recvlist.empty() || SOCK_DISCONNECTING==m_socks[i].sock_state)
			{
				rset->fd_array[rset->fd_count++] = i;
				n++;
			}
		}
		if(wset)
		{
			if((SOCK_CONNECTED==m_socks[i].sock_state && (m_socks[i].sendlist.size()+m_socks[i].ch->get_sndls_size())<m_socks[i].ch->m_max_sndls_size) || SOCK_DISCONNECTING==m_socks[i].sock_state)
			{
				wset->fd_array[wset->fd_count++] = i;
				n++;
			}
		}
	}
	return n;
}
int	cla_sockpool::send(CLA_SOCKET fd,const char* buf,int len)//返回:-1 关闭
{
	ERRFD_RETURN(fd,-1);
	SLock l(m_socks[fd].mt);
	if(SOCK_CONNECTED!=m_socks[fd].sock_state)
		return -1;

	if(m_socks[fd].sendlist.empty())
	{
		SLock ll(m_mt);
		m_ls_sending.push_back(fd); //通知底层发送数据
	}
	//分包
	int headsize = CLA_PTL_CNN_DATA_HEAD_LENGTH + m_socks[fd].ch->m_proxyhead_len;
	// 减60为减掉IP头20和UDP头8,然后再多减32字节，遇到过减45字节也发不动的情况。
	int max_pksize = m_socks[fd].ch->mtu() - headsize - 60;
	int packsize;
	cla_memblock* block;
	int sendsize = 0;
	while(len>0)
	{
		
		packsize = max_pksize;
		if(packsize>len)
			packsize = len;
		block = cla_memblock::alloc(CLA_MPTHREAD_APP);
		if(NULL==block) return sendsize;
		block->datapos = headsize;
		block->datasize = block->datapos + packsize;
		memcpy(block->buffer+block->datapos,buf+sendsize,packsize);
		sendsize += packsize;
		len -= packsize;
		m_socks[fd].sendlist.push_back(block);
	}
	return sendsize;
}
int	cla_sockpool::recv(CLA_SOCKET fd,char* buf,int len) //返回:-1 关闭
{
	//-1 表示可关闭
	ERRFD_RETURN(fd,-1);
	if(len<=0) return 0;
	SLock l(m_socks[fd].mt);
	int recvsize = 0;
	int cpsize;
	cla_memblock* block;

	//限接收速度,如果延迟接收，则睡8毫秒
	int max_recvsize = cla_statisticsSngl::instance()->get_max_recvsize();
	if(len > max_recvsize)
		len = max_recvsize; 
	if(max_recvsize==0)
		Sleep(1);

	assert(0==(m_socks[fd].close_state&0x01));
	if(m_socks[fd].close_state&0x01) return -1;
	//正常来说: 应用层如果关闭了则不会再调用recv()
	//若底层即使已经关闭，但应用层未关，此时有数据仍然照收。收到没数据时
	//再判断是否已经断开，会更理想一点。
	while(len>0 && !m_socks[fd].recvlist.empty())
	{
		block = m_socks[fd].recvlist.front();
		cpsize = block->datasize - block->datapos;
		if(cpsize>len) cpsize = len;
		memcpy(buf+recvsize,block->buffer+block->datapos,cpsize);
		block->datapos += cpsize;
		len -= cpsize;
		recvsize += cpsize;
		if(block->datasize == block->datapos)
		{
			m_socks[fd].recvlist.pop_front();
			block->free(CLA_MPTHREAD_APP);
		}
	}
	if(0==recvsize)
	{
		if(SOCK_CONNECTED!=m_socks[fd].sock_state)
			return -1;
	}
	else
	{
		m_socks[fd].ch->update_recv_win_num(m_socks[fd].recvlist.size());
		cla_statisticsSngl::instance()->m_recvspeed.add(recvsize);
	}

	return recvsize;
}


//*****************************************

void cla_sockpool::release_fd(CLA_SOCKET fd)
{
	m_socks[fd].sock_state = SOCK_IDLE;
	MAXSOCK_CLR(fd,m_maxsock);
	m_socks[fd].close_state = 0;
	m_socks[fd].last_active_tick = GetTickCount();
	clear_memblock_list(m_socks[fd].sendlist,CLA_MPTHREAD_APP);
	clear_memblock_list(m_socks[fd].recvlist,CLA_MPTHREAD_APP);
	{
		SLock ll(m_mt);
		m_socknum--;
	}
}
void cla_sockpool::clear_memblock_list(list<cla_memblock*>& ls,int ithreadtoken)
{
	for(list<cla_memblock*>::iterator it=ls.begin();it!=ls.end();++it)
	{
		(*it)->free(ithreadtoken); //上层线程调用队列UAC_THREAD_APP，底层用1
	}
	ls.clear();
}

int cla_sockpool::find_idel_socket()
{
	//非线程安全，即找到的idel节点在使用时再次确定是否为idel，否则重复找
	//每次总是从头找，使connector循环次数尽量小
	DWORD tick = GetTickCount();
	for(int i=0;i<CLA_FD_SIZE;++i)
	{
		if(SOCK_IDLE==m_socks[i].sock_state && _timer_after(tick,m_socks[i].last_active_tick + CLA_SOCK_IDEL_TICK) && CLA_DISCONNECTED==m_socks[i].ch->get_state())
		{
			if(m_maxsock<i) m_maxsock = i;
			return i;
		}
	}
	return -1;
}
void cla_sockpool::_handle_send(CLA_SOCKET fd)
{
	cla_memblock* b;
	SLock l(m_socks[fd].mt);
	//不用检查状态是否为连接状态，ch->send会检查
	while(!m_socks[fd].sendlist.empty())
	{
		b = m_socks[fd].sendlist.front();
		m_socks[fd].sendlist.pop_front();
		if(0!=m_socks[fd].ch->send(b,!m_socks[fd].sendlist.empty()))
			return;
	}
}

void cla_sockpool::on_timer(int e)
{
	//暂无用
}
cla_channeli* cla_sockpool::cla_attach_channel(cla_addr &to,cla_addr &proxy)
{
	CLA_SOCKET fd = -1;
	//不会死循环，
	while(1)
	{
		fd = find_idel_socket();
		if(-1==fd)
		{
			break;
		}
		else
		{
			SLock l(m_socks[fd].mt);
			if(SOCK_IDLE!=m_socks[fd].sock_state || CLA_DISCONNECTED!=m_socks[fd].ch->get_state())
				continue;
			m_socks[fd].sock_state = SOCK_WAIT_ACCEPTING;
			m_socks[fd].last_active_tick = GetTickCount();
			break;
		}
	}
	
	if(-1==fd)
		return NULL;
	{
		SLock l(m_socks[fd].mt);
		m_socks[fd].ch->attach(to,proxy); //一定成功
		m_socks[fd].to = to;
		m_socks[fd].proxy = proxy;
		m_socks[fd].to.ntype = 6;
		
		{
			SLock ll(m_mt);
			m_socknum++;
		}
		return m_socks[fd].ch;
	}
	
	return NULL;
}
int	cla_sockpool::work(int e)
{
	list<CLA_SOCKET> ls;
	list<CLA_SOCKET>::iterator it;
	int fd;
	ULONGLONG delay_usec = 0;
	while(m_brun)
	{
		cla_timerSngl::instance()->handle_root();
		delay_usec = cla_timerSngl::instance()->get_remain_us();
		delay_usec = delay_usec/2;
		if(delay_usec>30000) delay_usec=30000;
		
		m_ctr.handle_select_read((long)delay_usec);

		//处理上层的连接，断开，数据发送等
		//1。发送
		if(!m_ls_sending.empty())
		{
			m_mt.lock();
			m_ls_sending.swap(ls);
			m_mt.unlock();

			for(it=ls.begin();it!=ls.end();++it)
			{
				_handle_send(*it);
			}
			ls.clear();
		}

		//2.连接
		if(!m_ls_connecting.empty())
		{
			m_mt.lock();
			m_ls_connecting.swap(ls);
			m_mt.unlock();

			for(it=ls.begin();it!=ls.end();++it)
			{
				fd = *it;
				SLock l(m_socks[fd].mt);
				if(SOCK_WAIT_CONNECTING!=m_socks[fd].sock_state)
					continue;
				m_socks[fd].sock_state = SOCK_CONNECTING;
				if(0!=m_socks[fd].ch->connect(m_socks[fd].to,m_socks[fd].proxy))
				{
					assert(false);
					on(cla_channelListener::Disconnected(),m_socks[fd].ch);
				}
			}
			ls.clear();
		}

		//2.断开
		if(!m_ls_disconnecting.empty())
		{
			m_mt.lock();
			m_ls_disconnecting.swap(ls);
			m_mt.unlock();

			for(it=ls.begin();it!=ls.end();++it)
			{
				fd = *it;
				//SLock l(m_socks[fd].mt); //由于使用简单锁,所以在此不要锁.否则ondisconnect导致死锁.
				//可能上层底层都已经关闭
				//(上层关闭时底层未关时增加了引命令，未执行到此时底层有关了的情况是可能的)
				//这种情况仍然执行一次关闭(可能fd已经成为新的连接错关，但可能性极低。)
				if(m_socks[fd].close_state == 0x01)
					m_socks[fd].ch->disconnect();
			
			}
			ls.clear();
		}
	}
	return 0;
}

void cla_sockpool::on(Connected,cla_channeli* ch)
{
	int fd = ch->idx();
	SLock l(m_socks[fd].mt);
	if(SOCK_WAIT_ACCEPTING == m_socks[fd].sock_state)
	{
		m_socks[fd].sock_state = SOCK_ACCEPTING;
		SLock ll(m_mt);
		m_ls_accepting.push_back(fd);
	}
	else if(SOCK_CONNECTING == m_socks[fd].sock_state)
	{
		m_socks[fd].sock_state = SOCK_CONNECTED;
	}
	else
	{
		assert(SOCK_IDLE==m_socks[fd].sock_state);
		m_socks[fd].ch->disconnect();
	}
}
void cla_sockpool::on(Disconnected,cla_channeli* ch)
{
	int fd = ch->idx();
	SLock l(m_socks[fd].mt); //这里会导致锁的重入,所以要使用循环锁
	if(SOCK_IDLE!=m_socks[fd].sock_state)
	{
		m_socks[fd].close_state |= 0x02;
		//SOCK_WAIT_ACCEPTING,SOCK_ACCEPTING时表示上层还没接收连接,没有上层操作
		if(SOCK_WAIT_ACCEPTING==m_socks[fd].sock_state || SOCK_ACCEPTING==m_socks[fd].sock_state||m_socks[fd].close_state == 0x03)
			release_fd(fd);
		else
			m_socks[fd].sock_state = SOCK_DISCONNECTING;
	}
}
void cla_sockpool::on(Data,cla_channeli* ch,cla_memblock *b)
{
	int fd = ch->idx();
	{
		SLock l(m_socks[fd].mt);
		//SOCK_ACCEPTING 表示应用层未执行accept(),即未产生上层控制
		if((SOCK_ACCEPTING==m_socks[fd].sock_state||SOCK_CONNECTED==m_socks[fd].sock_state))
		{
			m_socks[fd].recvlist.push_back(b);
		}
		else
		{
			assert(SOCK_DISCONNECTING==m_socks[fd].sock_state);
			b->free(CLA_MPTHREAD_CORE);
			//在此不要执行disconnect,可能上层刚执行完，m_ls_disconnecting 里有此命令未执行到
		}
	}
}
void cla_sockpool::on(Writable,cla_channeli* ch)
{
	_handle_send(ch->idx());
}

