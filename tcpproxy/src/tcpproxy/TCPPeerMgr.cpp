#include "TCPPeerMgr.h"
#include "IOReactor.h"
#include "Setting.h"
#include "Timer.h"
#include "Util.h"

TCPPeerMgr::TCPPeerMgr(void)
:m_binit(false)
{
}

TCPPeerMgr::~TCPPeerMgr(void)
{
}

int TCPPeerMgr::init()
{
	if(m_binit) return 1;
	m_binit = true;
	m_apt.open(SettingSngl::instance()->get_listen_port(),NULL,this,IOReactorSngl::instance());
	TimerSngl::instance()->register_timer(this,1,5000);
	return 0;
}
void TCPPeerMgr::fini()
{
	if(!m_binit)
		return;
	m_binit = false;
	m_apt.close();
	TimerSngl::instance()->unregister_all(this);
	clear_all_peer();
	handle_pending();
}
bool TCPPeerMgr::attach_tcp_socket(SOCKET fd,sockaddr_in& addr)
{
	static int ifile = 0;
	char path[1024];
	sprintf(path,"./log/tcp_%s.dat",Util::time_to_datetime_string2(time(NULL)).c_str());
	TCPPeer* peer = new TCPPeer();
	if(!peer->attach(fd,addr,SettingSngl::instance()->get_server_ip().c_str()
		,SettingSngl::instance()->get_server_port(),
		SettingSngl::instance()->get_is_writelog()?path:NULL))
	{
		delete peer;
		return false;
	}
	m_peers.push_back(peer);
	peer->add_listener(this);
	return true;
}
void TCPPeerMgr::on(TCPPeerListener::Disconnected,TCPPeer* peer)
{
	for(list<TCPPeer*>::iterator it=m_peers.begin();it!=m_peers.end();++it)
	{
		if((*it)==peer)
		{
			peer->remove_listener(this);
			m_pendings.push_back(peer);
			m_peers.erase(it);
			break;
		}
	}
}
void TCPPeerMgr::on_timer(int e)
{
	switch(e)
	{
	case 1:
		{
			handle_pending();
		}
		break;
	default:
		break;
	}
}
void TCPPeerMgr::clear_all_peer()
{
	TCPPeer *peer;
	while(NULL !=(peer=m_peers.front()))
		peer->disconnect();
}
void TCPPeerMgr::handle_pending()
{
	for(list<TCPPeer*>::iterator it=m_pendings.begin();it!=m_pendings.end();++it)
		delete *it;
	m_pendings.clear();
}

