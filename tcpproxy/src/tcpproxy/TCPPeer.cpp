#include "TCPPeer.h"
#include "Setting.h"

TCPPeer::TCPPeer(void)
: Speaker<TCPPeerListener>(1)
, m_state(0)
, m_peer(NULL)
, m_type("")
, m_file(NULL)
{
	DEBUGMSG("# ++new peer() \n");
	m_ch.add_listener(this);
}

TCPPeer::~TCPPeer(void)
{
	m_ch.remove_listener(this);
	if(m_peer && m_type=="src:")
		delete m_peer;
	
	DEBUGMSG("# --delete peer() \n");
}

bool TCPPeer::attach(SOCKET fd,sockaddr_in& addr,const char* svr_ip,unsigned short svr_port,const char* filepath)
{
	assert(NULL==m_peer);
	if(0!=m_ch.attach(fd,addr))
		return false;
	m_peer = new TCPPeer();
	m_peer->m_peer = this;
	m_peer->m_type = "des:";
	m_peer->m_ch.connect(svr_ip,svr_port);
	m_state = 1;
	m_type = "src:";
	if(filepath)
	{
		m_file = new File64();
		m_file->open(filepath,F64_RDWR|F64_TRUN);
	}
	
	DEBUGMSG("# ++++++++ new tcp in ++++++++ \n");
	return true;
}


int TCPPeer::disconnect()
{
	return m_ch.disconnect();
}
int TCPPeer::send(MemBlock* b)
{
	if(m_type == "src:")
	{
		if(m_file)
		{
			if(SettingSngl::instance()->get_is_writeseq())
				m_file->write_n("\r\n/server:/\r\n",13);
			m_file->write_n(b->buf,b->datalen);
			m_file->flush();
		}
	}
	return m_ch.send(b);
}


void TCPPeer::on(Connected,Channel* ch)
{
	m_state = 1;
}
void TCPPeer::on(Disconnected,Channel* ch)
{
	if(m_file)
	{
		m_file->close();
		delete m_file;
		m_file = NULL;
	}
	m_state = 0;
	if(m_peer->m_state != 0)
		m_peer->disconnect();
	if(m_type == "src:")
		DEBUGMSG("# ------- end tcp in ------- \n");
	fire(TCPPeerListener::Disconnected(),this);
}
void TCPPeer::on(Readable,Channel* ch,const int& wait)
{
	int& iwait = (int&)wait;
	if(m_peer->m_state != 1)
	{
		iwait = 1;
		return;
	}
	int ret=0;
	MemBlock *b;
	do
	{
		b = MemBlock::allot(4096);
		ret = m_ch.recv(b->buf,4096);
		if(ret>0)
		{
			b->datalen = ret;
			//写入文件并转发
			if(m_type == "src:")
			{
				if(m_file)
				{
					if(SettingSngl::instance()->get_is_writeseq())
						m_file->write_n("\r\n/client:/\r\n",13);
					m_file->write_n(b->buf,b->datalen);
					m_file->flush();
				}
				DEBUGMSG("# c=>s : %d\n",ret);
			}
			else
				DEBUGMSG("# s=>c : %d\n",ret);
			m_peer->send(b);
		}
		else
			b->free();
	}while(ret>0);
}
//void TCPPeer::on(Writable,Channel* ch)
//{
//}

