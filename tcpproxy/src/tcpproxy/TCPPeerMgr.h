#pragma once
#include "TCPAcceptor.h"
#include "TCPPeer.h"
#include "Handler.h"

class TCPPeerMgr : public TCPChannelFactory
	,public TCPPeerListener
	,public TimerHandler
{
public:
	TCPPeerMgr(void);
	virtual ~TCPPeerMgr(void);
public:
	int init();
	void fini();
	virtual bool attach_tcp_socket(SOCKET fd,sockaddr_in& addr);
	virtual void on(TCPPeerListener::Disconnected,TCPPeer* peer);
	virtual void on_timer(int e);

private:
	void clear_all_peer();
	void handle_pending();
private:
	bool m_binit;
	TCPAcceptor m_apt;
	unsigned short m_server_port;
	string	m_server_ip;
	list<TCPPeer*> m_peers;
	list<TCPPeer*> m_pendings;

};
typedef Singleton<TCPPeerMgr> TCPPeerMgrSngl;