#pragma once
#include "cla_mempool.h"
#include "cl_incnet.h"

class cla_acceptor;
class cla_acceptorHandler
{
	friend class cla_acceptor;
public:
	cla_acceptorHandler(void)
		:m_fd(INVALID_SOCKET) 
	{}
	virtual ~cla_acceptorHandler(void){}
	
	//����1��ʾ block �Ӻ���.
	virtual int on_acceptor_recv(cla_memblock *block,sockaddr_in& addr) = 0;
protected:
	int m_fd;
};


class cla_acceptor
{
public:
	cla_acceptor(void);
	~cla_acceptor(void);
public:
	int open(unsigned short port,cla_acceptorHandler* cnn);
	int close();
	int handle_select_read(unsigned long delay_usec=0);
	int get_fd() const {return m_fd;}
protected:
	int handle_input();
private:
	int m_fd;
	fd_set m_rfd;
	cla_acceptorHandler* m_cnn;
	cla_acceptorHandler* m_stun;

	//Ƶ��ʹ�õ���ʱ����
	int _tmp_n;
	cla_memblock *_tmp_block;
	sockaddr_in _tmp_addr;
	socklen_t _tmp_addr_len;

	timeval _timeout;
	int _n;
};

