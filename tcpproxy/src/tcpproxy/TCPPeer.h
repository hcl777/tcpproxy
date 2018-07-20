#pragma once
#include "TCPChannel.h"
#include "File64.h"

class TCPPeer;
class TCPPeerListener
{
public:
	virtual ~TCPPeerListener(void){}

	
	template<int I>
	struct S{enum{T=I};};
	
	typedef S<3> Disconnected;

	virtual void on(Disconnected,TCPPeer* peer){}
};


class TCPPeer : public ChannelListener
	, public Speaker<TCPPeerListener>
{
public:
	TCPPeer(void);
	~TCPPeer(void);

public:
	bool attach(SOCKET fd,sockaddr_in& addr,const char* svr_ip,unsigned short svr_port,const char* filepath);
	int disconnect();

private:
	int send(MemBlock* b);

public:
	virtual void on(Connected,Channel* ch);
	virtual void on(Disconnected,Channel* ch);
	virtual void on(Readable,Channel* ch,const int& wait);
	//virtual void on(Writable,Channel* ch);

private:
	TCPChannel m_ch;
	int m_state;
	TCPPeer *m_peer; //һ˫PEER��Ӧ
	File64 *m_file;//ֻ��������src����
	string m_type; //"src:": ����, "des:": Ŀ�귽

};
