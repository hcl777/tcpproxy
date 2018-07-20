#include "cla_connector.h"
#include "clacp.h"
#include "cl_net.h"
#include "cla_log.h"
#include "cla_config.h"

//建立连接超时
#define CLA_CNN_UNOK_TIMEOUT 12000
//最后接收数据超时
#define CLA_CNN_LASTRECV_TIMEOUT 50000
//保活超时
#define CLA_CNN_KEEPALIVE_TIMEOUT 6000


sockaddr_in& cla_to_sockaddr(sockaddr_in& addr,const cla_addr& a)
{
	addr.sin_addr.s_addr = a.ip;
	addr.sin_port = a.port;
	return addr;
}

cla_connector::cla_connector()
:m_binit(false)
,m_cnnf(NULL)
,m_socknum(0)
,m_max_i(-1)
,m_tick(GetTickCount())
,m_mtu(cl_net::get_mtu())
{

	memset(&_tmp_addr,0,sizeof(_tmp_addr));
	_tmp_addr.sin_family = AF_INET;

	_tmp_ts_sbuf = new char[m_mtu];
	memset(_tmp_ts_sbuf,0xc5,m_mtu);
	
}

cla_connector::~cla_connector(void)
{
	delete[] _tmp_ts_sbuf;
}
int cla_connector::init(unsigned short port,cla_cnnFactory* cnnf,const char* stunsvr)
{
	if(m_binit)
		return 1;
	if(0!=m_apt.open(port,static_cast<cla_acceptorHandler*>(this)))
		return -1;
	m_fd = m_apt.get_fd();
	m_stunc.init(m_fd,stunsvr);

	m_udps = new UDPSession_t*[CLA_FD_SIZE];
	for(int i=0;i<CLA_FD_SIZE;++i)
		m_udps[i] = new UDPSession_t();
	cla_timerSngl::instance()->register_timer(this,1,200);
	m_cnnf = cnnf;
	m_max_i = -1;
	m_binit = true;
	return 0;
}
void cla_connector::fini()
{
	if(!m_binit)
		return;
	m_apt.close();
	m_stunc.fini();
	m_fd = INVALID_SOCKET;
	cla_timerSngl::instance()->unregister_all(this);
	assert(0==m_socknum);
	for(int i=0;i<CLA_FD_SIZE;++i)
		delete m_udps[i];
	delete[] m_udps;
	m_binit = false;
}
void cla_connector::on_timer(int e)
{
	m_tick = GetTickCount();
	switch(e)
	{
	case 1:
		on_timer_send();
		break;
	default:
		assert(0);
		break;
	}
}

//channel call
/************************
************************/
int cla_connector::connect(cla_channeli* ch,int nattype)
{
	int i = register_channel(ch,nattype);
	if(-1==i) return -1;
	if(ch->m_proxyhead_len>0)
	{
		m_udps[i]->des_nattype2 = 0;
		m_udps[i]->state = ECLA_CORE_MYADDRING; //不管类型,都先新获取一次商端口
		ptl_send_T1(CLA_PTL_CNN_MYADDR,i,m_udps[i]->handle->m_addr);
	}
	else
	{
		m_udps[i]->state = ECLA_CORE_CONNECTING;
		if(m_udps[i]->des_nattype2>1)
			ptl_send_proxy_T1(CLA_PTL_CNN_HOLE,i,m_udps[i]->handle->m_ndes,m_stunc.stunA());
			//ptl_send_hole(CLA_PTL_CNN_HOLE,i,m_udps[i]->handle->m_ndes);
		ptl_send_cnn_syn(i,CLA_PTL_CNN_SYN);
	}
	return 0;
}
int cla_connector::disconnect(cla_channeli* ch)
{
	//连接发两个close
	if(ch->m_s.did >= 0)
	{
		//有des_sessionid表示连接成功 disconn
		ptl_send_cnn_cmd(ch->__idx,CLA_PTL_CNN_CLS);
		ptl_send_cnn_cmd(ch->__idx,CLA_PTL_CNN_CLS);
	}
	unregister_channel(ch);
	return 0;
}

//*****************************
//acceptor call
//返回1表示 block 延后处理.
int cla_connector::on_acceptor_recv(cla_memblock *block,sockaddr_in& addr)
{
	cla_addr _tmp_from,_tmp_from_proxy;
	_tmp_rps.attach((char*)block->buffer,block->datasize,block->datasize);
	_tmp_from.ip = addr.sin_addr.s_addr;
	_tmp_from.port = addr.sin_port;
	_tmp_from_proxy.ip = 0;
	_tmp_from_proxy.port = 0;
	return on_recv(_tmp_rps,_tmp_from,_tmp_from_proxy,block);
}

//---------------------------------------------------------------------

int cla_connector::register_channel(cla_channeli *ch,int nattype)
{
	int i=ch->__idx;
	assert(!m_udps[i]->is_used);
	if(m_udps[i]->is_used)
		return -1;
	m_socknum++;
	if(m_max_i<i) m_max_i = i;
	m_udps[i]->is_used = true;
	m_udps[i]->des_nattype = nattype;
	m_udps[i]->des_nattype2 = nattype;
	m_udps[i]->last_send_tick = m_tick;
	m_udps[i]->last_recv_tick = m_tick;
	m_udps[i]->begin_tick = m_tick;
	
	m_udps[i]->handle = ch;
	m_udps[i]->handle->m_s.did = -1;
	m_udps[i]->handle->m_fd = m_fd;
	return i;
}
void cla_connector::unregister_channel(cla_channeli *ch)
{
	int i = ch->__idx;
	assert(m_udps[i]->is_used);
	if(!m_udps[i]->is_used)
		return;

	//发起方连接失败的上报记录
	if(ECLA_CORE_CONNECTING == m_udps[i]->state)
	{
		cla_log::on_connect_fail();
	}

	m_udps[i]->reset();
	m_udps[i]->begin_tick = m_tick;
	ch->m_s.did = -1;
	ch->m_fd = INVALID_SOCKET;

	m_socknum--;
	if(i==m_max_i) m_max_i--;
	while(m_max_i>=0 && !m_udps[m_max_i]->is_used)
		m_max_i--;
}

void cla_connector::ptl_send_channel_buf(int i,char* buf,int len)
{
	//如果要中转,假设buf已经预留前面proxyhead, len已含proxyhead大小.
	if(m_udps[i]->handle->m_proxyhead_len>0)
	{
		_tmp_sps.attach(buf,m_udps[i]->handle->m_proxyhead_len,0);
		_tmp_sps << CLA_PTL_PROXY_SEND;
		_tmp_sps << m_udps[i]->handle->m_ndes;
	}
	::sendto(m_fd,buf,len,0,(const sockaddr*)&m_udps[i]->handle->m_addr,sizeof(sockaddr_in));
}

void cla_connector::ptl_send_nat(int i,cla_addr& ndes)
{
	//send nat 不会有中转的情况
	_tmp_addr.sin_port = ndes.port;
	_tmp_addr.sin_addr.s_addr = ndes.ip;
	_tmp_sps.attach(_tmp_sbuf,500,0);
	_tmp_sps << CLA_PTL_CNN_NAT;
	_tmp_sps << i;
	::sendto(m_fd,_tmp_sbuf,_tmp_sps.length(),0,(const sockaddr*)&_tmp_addr,sizeof(sockaddr_in));
}

void cla_connector::ptl_send_cnnproxy(int i)
{
	//经stun 走proxy中转
	cla_ptl_cnn_proxy_t _tmp_cnn_proxy;
	_tmp_cnn_proxy.connid = i;
	_tmp_cnn_proxy.proxy.ip = m_udps[i]->handle->m_addr.sin_addr.s_addr;
	_tmp_cnn_proxy.proxy.port = m_udps[i]->handle->m_addr.sin_port;
	_tmp_cnn_proxy.src = m_udps[i]->handle->m_nmy;

	ptl_send_proxy_T1(CLA_PTL_CNN_PROXY,_tmp_cnn_proxy,m_udps[i]->handle->m_ndes,m_stunc.stunA());
}

void cla_connector::ptl_send_cnn_cmd(int i,cla_cmd_t cmd)
{
	_tmp_sps.attach(_tmp_sbuf+m_udps[i]->handle->m_proxyhead_len,500,0);
	_tmp_sps << cmd;
	_tmp_sps << m_udps[i]->handle->m_s;
	ptl_send_channel_buf(i,_tmp_sbuf,_tmp_sps.length()+m_udps[i]->handle->m_proxyhead_len);
	m_udps[i]->last_send_tick = m_tick;
}
void cla_connector::ptl_send_cnn_syn(int i,cla_cmd_t cmd)
{
	_tmp_sps.attach(_tmp_sbuf+m_udps[i]->handle->m_proxyhead_len,500,0);
	_tmp_sps << cmd;
	_tmp_sps << m_udps[i]->handle->m_s;
	_tmp_sps << m_mtu;
	_tmp_sps << m_udps[i]->handle->m_ndes;
	ptl_send_channel_buf(i,_tmp_sbuf,_tmp_sps.length()+m_udps[i]->handle->m_proxyhead_len);
	m_udps[i]->last_send_tick = m_tick;
}
void cla_connector::ptl_send_cnn_cls(cla_session_t& s,cla_addr& ndes,cla_addr& nproxy)
{
	if(nproxy.ip>0)
		ptl_send_proxy_T1(CLA_PTL_CNN_CLS,s,ndes,cla_to_sockaddr(_tmp_addr,nproxy));
	else
		ptl_send_T1(CLA_PTL_CNN_CLS,s,cla_to_sockaddr(_tmp_addr,ndes));
}
void cla_connector::on_timer_send()
{
	for(int i=0;i<=m_max_i;++i)
	{
		if(!m_udps[i]->is_used)
			continue;

		//1.检测超时连接
		if( (ECLA_CORE_CONNECTED!=m_udps[i]->state 
			&& ECLA_CORE_ACCEPTED!=m_udps[i]->state
			&& _timer_after(m_tick,m_udps[i]->begin_tick+CLA_CNN_UNOK_TIMEOUT)) 
			|| _timer_after(m_tick,m_udps[i]->last_recv_tick+CLA_CNN_LASTRECV_TIMEOUT) )
		{
			m_udps[i]->handle->disconnect();
			continue;
		}
		
		//2.10秒保活一次
		if(ECLA_CORE_CONNECTED==m_udps[i]->state || ECLA_CORE_ACCEPTED==m_udps[i]->state)
		{
			if(_timer_after(m_tick,m_udps[i]->last_send_tick+CLA_CNN_KEEPALIVE_TIMEOUT))
				ptl_send_cnn_cmd(i,CLA_PTL_CNN_LIVE);
		}
		else if(_timer_after(m_tick,m_udps[i]->last_send_tick+1000))
		{
			if(ECLA_CORE_MYADDRING==m_udps[i]->state)
				ptl_send_T1(CLA_PTL_CNN_MYADDR,i,m_udps[i]->handle->m_addr);
			else if(ECLA_CORE_CNNPROXYING==m_udps[i]->state)
				ptl_send_cnnproxy(i);
			else if(ECLA_CORE_CONNECTING==m_udps[i]->state)
			{
				//因为对方类型是4的情况，需要收到对方的NAT包,所以不管收不收到hole_ack.都照重发hole.
				if(m_udps[i]->des_nattype2>1 || 4==m_udps[i]->des_nattype)
					ptl_send_proxy_T1(CLA_PTL_CNN_HOLE,i,m_udps[i]->handle->m_ndes,m_stunc.stunA());
				ptl_send_cnn_syn(i,CLA_PTL_CNN_SYN);
			}
			else if(ECLA_CORE_ACCEPTING==m_udps[i]->state)
				ptl_send_cnn_syn(i,CLA_PTL_CNN_ACK);
		}
	}
}
bool cla_connector::check_session_ok(cla_session_t& s,cla_addr& ndes)
{
	int i = s.did;
	if(i<0||i>=CLA_FD_SIZE||!m_udps[i]->is_used)
		return false;
	if(m_udps[i]->handle->m_ndes != ndes)
		return false;
	if(-1!=m_udps[i]->handle->m_s.did && m_udps[i]->handle->m_s.did != s.sid)
		return false;
	return true;
}
void cla_connector::check_update_port(int i,unsigned short nport)
{
	if(m_udps[i]->handle && m_udps[i]->handle->m_ndes.port!=nport)
	{
		printf("#******** cla_connector desport changed (%d,%d)\n",ntohs(m_udps[i]->handle->m_ndes.port),ntohs(nport));
		m_udps[i]->handle->m_ndes.port = nport;
	}
}
int cla_connector::on_recv(cl_ptlstream& ps,cla_addr& from,cla_addr& proxy,cla_memblock* b)
{
	cla_cmd_t cmd;
	cla_session_t s;
	int i = 0;
	if(0!=ps>>cmd)
		return -1;
	
	//switch语句通过建索引表的方式跳转
	switch(cmd)
	{
	case CLA_PTL_STUN_CMD:
		m_stunc.on_data(ps.read_ptr(),ps.length(),from);
		break;
	case CLA_PTL_PROXY_RECV:
		{
			//DEBUGMSG("# proxy from [ %s ] \n",cl_net::ip_ntoas(from.ip,from.port).c_str());
			proxy = from;
			if(0!=ps>>from)
				return -1;
			return on_recv(ps,from,proxy,b);
		}
		break;
	case CLA_PTL_CNN_DATA:
		{
			if(0!=ps>>s) return -1;
			if(check_session_ok(s,from))
			{
				i = s.did;
				if(ECLA_CORE_ACCEPTING==m_udps[i]->state)
				{
					m_udps[i]->state = ECLA_CORE_ACCEPTED;
					m_udps[i]->handle->on_connected();
				}
				if(ECLA_CORE_CONNECTED==m_udps[i]->state || ECLA_CORE_ACCEPTED==m_udps[i]->state)
				{
					m_udps[i]->last_recv_tick = m_tick;
					m_udps[i]->handle->on_recv_data(b); //里面释放
					return 1;
				}
			}
			s.did = s.sid;
			s.sid = -1;
			ptl_send_cnn_cls(s,from,proxy);
			return 0;
		}
		break;
	case CLA_PTL_CNN_DATA_ACK:
		{
			if(0!=ps>>s) return -1;
			if(check_session_ok(s,from))
			{
				i = s.did;
				if(ECLA_CORE_CONNECTED==m_udps[i]->state || ECLA_CORE_ACCEPTED==m_udps[i]->state)
				{
					m_udps[i]->last_recv_tick = m_tick;
					m_udps[i]->handle->on_recv_data(b); //由外部释放
				}
			}
			return 0;
		}
		break;
	case CLA_PTL_CNN_HOLE:
		{
			if(0!=ps>>i) return -1;
			//如果自己是nat4，其实可以不回复，避免stun接收过多中转包
			DEBUGMSG("# on_recv( hole i=%d ) \n",i);
			if(4!=g_cla_conf.nattype)
				ptl_send_proxy_T1(CLA_PTL_CNN_HOLE_ACK,i,from,cla_to_sockaddr(_tmp_addr,proxy));
			ptl_send_nat(i,from);
		}
		break;
	case CLA_PTL_CNN_HOLE_ACK:
		{
			if(0!=ps>>i) return -1;
			if(i>m_max_i || !m_udps[i]->is_used || from.ip!=m_udps[i]->handle->m_ndes.ip) return -1;
			m_udps[i]->des_nattype2 = 0;
			DEBUGMSG("# on_recv( hole_ack i=%d ) \n",i);
		}
		break;
	case CLA_PTL_CNN_NAT:
		{
			//1.更新nattype2,
			//2.可能端口改变,更新端口,
			//3.继续发送syn
			if(0!=ps>>i) return -1;
			if(i>m_max_i || !m_udps[i]->is_used || from.ip!=m_udps[i]->handle->m_ndes.ip) return -1;
			m_udps[i]->des_nattype2 = 0;
			DEBUGMSG("# on_recv( nat i=%d ) \n",i);
			if(ECLA_CORE_CONNECTING==m_udps[i]->state)
			{
				m_udps[i]->handle->m_ndes.port = from.port;
				ptl_send_cnn_syn(i,CLA_PTL_CNN_SYN);
			}
		}
		break;
	case CLA_PTL_CNN_MYADDR_ACK:
		{
			cla_addr cla;
			ps >> i;
			ps >> cla;
			if(0!=ps.ok()) return -1;
			if(i>m_max_i || !m_udps[i]->is_used) return -1;
			DEBUGMSG("# cnn_myaddr_ack( i=%d addr=%s ) \n",i,cl_net::ip_ntoas(cla.ip,cla.port).c_str());
			if(ECLA_CORE_MYADDRING==m_udps[i]->state)
			{
				m_udps[i]->handle->m_nmy = cla;
				m_udps[i]->state = ECLA_CORE_CNNPROXYING;
				ptl_send_cnnproxy(i);
			}
		}
		break;
	case CLA_PTL_CNN_PROXY:
		{
			//从stun转发来.从proxy回复
			cla_ptl_cnn_proxy_t cnn_proxy;
			if(0!=ps>>cnn_proxy) return -1;
			DEBUGMSG("# cnn_proxy( i=%d addr=%s,proxy=%s ) \n",cnn_proxy.connid,cl_net::ip_ntoas(cnn_proxy.src.ip,cnn_proxy.src.port).c_str(),
				cl_net::ip_ntoas(cnn_proxy.proxy.ip,cnn_proxy.proxy.port).c_str());
			ptl_send_proxy_T1(CLA_PTL_CNN_PROXY_ACK,cnn_proxy.connid,cnn_proxy.src,cla_to_sockaddr(_tmp_addr,cnn_proxy.proxy));
		}
		break;
	case CLA_PTL_CNN_PROXY_ACK:
		{
			if(0!=ps>>i) return -1;
			if(i>m_max_i || !m_udps[i]->is_used || from.ip!=m_udps[i]->handle->m_ndes.ip) return -1;
			DEBUGMSG("# cnn_proxy_ack( i=%d addr=%s,proxy=%s ) \n",i,cl_net::ip_ntoas(from.ip,from.port).c_str(),
				cl_net::ip_ntoas(proxy.ip,proxy.port).c_str());
			if(ECLA_CORE_CNNPROXYING==m_udps[i]->state)
			{
				assert(proxy.ip == m_udps[i]->handle->m_addr.sin_addr.s_addr);
				m_udps[i]->handle->m_ndes.port = from.port;
				m_udps[i]->state = ECLA_CORE_CONNECTING;
				ptl_send_cnn_syn(i,CLA_PTL_CNN_SYN);
			}
		}
		break;
	default:
		{
			if(0!=ps>>s) return -1;

			DEBUGMSG("# on_recv( cmd=%d , %d -> %d) \n",(int)cmd,s.sid,s.did);
			if(CLA_PTL_CNN_SYN==cmd)
			{
				//先检查是否已经存在连接,有则只回复ACK,没有则创建新连接.
				uint32 mtu;
				cla_addr my;
				ps >> mtu;
				if(0!=ps>>my) return -1;
				if(0==mtu) mtu=1500;
				//查找是否已经有对方的sid 和 from
				DWORD tick = GetTickCount();
				for(i=0;i<=m_max_i;++i)
				{
					if(m_udps[i]->is_used && m_udps[i]->handle->m_ndes == from && m_udps[i]->handle->m_s.did == s.sid)
					{
						if(ECLA_CORE_ACCEPTING==m_udps[i]->state)
							ptl_send_cnn_syn(i,CLA_PTL_CNN_ACK);
						else if(_timer_after(tick,m_udps[i]->begin_tick + 2*CLA_CNN_UNOK_TIMEOUT))
						{
							//在此判断此连接对方已经中断，并且重启发起的新连接，未调试
							DEBUGMSG("#*** recv CLA_PTL_CNN_SYN: old cnn and disconnect (sid=%d,did=%d) \n",m_udps[i]->handle->m_s.sid,m_udps[i]->handle->m_s.did);
							m_udps[i]->handle->disconnect();
							break;
						}
						return 0;
					}
				}
				//接收新连接
				cla_channeli *ch = m_cnnf->cla_attach_channel(from,proxy);
				if(NULL==ch)
				{
					s.did = s.sid;
					s.sid = -1;
					ptl_send_cnn_cls(s,from,proxy);
					return 0;
				}
				i = register_channel(ch,1);
				ch->m_mtu = mtu<m_mtu?mtu:m_mtu; //取小的
				ch->m_s.did = s.sid;
				m_udps[i]->state = ECLA_CORE_ACCEPTING;
				ptl_send_cnn_syn(i,CLA_PTL_CNN_ACK);
				return 0;
			}
			else if(CLA_PTL_CNN_ACK==cmd)
			{
				//
				uint32 mtu;
				cla_addr my;
				ps >> mtu;
				if(0!=ps>>my) return -1;
				if(0==mtu) 
					mtu=1500;
				if(!check_session_ok(s,from))
					return 0;
				i = s.did;
				m_udps[i]->handle->m_mtu = mtu<m_mtu?mtu:m_mtu; //取小的
				if(ECLA_CORE_CONNECTING==m_udps[i]->state)
				{
					assert(m_udps[i]->handle->m_s.did == -1);
					m_udps[i]->handle->m_s.did = s.sid;
					m_udps[i]->state = ECLA_CORE_CONNECTED;
					ptl_send_cnn_syn(i,CLA_PTL_CNN_ACK);
					m_udps[i]->handle->on_connected();
				}
				else if(ECLA_CORE_ACCEPTING==m_udps[i]->state)
				{
					m_udps[i]->state = ECLA_CORE_ACCEPTED;
					m_udps[i]->handle->on_connected();
				}	
			}
			else if(CLA_PTL_CNN_LIVE==cmd)
			{
				if(check_session_ok(s,from))
					m_udps[s.did]->last_recv_tick = m_tick;
			}
			else if(CLA_PTL_CNN_CLS==cmd)
			{
				if(check_session_ok(s,from))
					m_udps[s.did]->handle->disconnect();
			}
		}
		break;
	}
	return 0;
}


