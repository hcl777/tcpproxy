#include "cla_acceptor.h"
#include "cla_proto.h"
#include "cl_socketbase.h"
#include "cl_net.h"

cla_acceptor::cla_acceptor(void)
:m_fd(INVALID_SOCKET)
,m_cnn(NULL)
,_tmp_block(NULL)
{
	FD_ZERO(&m_rfd);
	memset(&_tmp_addr,0,sizeof(_tmp_addr));
	_tmp_addr_len = sizeof(_tmp_addr);
}

cla_acceptor::~cla_acceptor(void)
{
}
int cla_acceptor::open(unsigned short port,cla_acceptorHandler* cnn)
{
	assert(INVALID_SOCKET==m_fd);
	if(0!=cl_socketbase::open_udp_sock(m_fd,htons(port),INADDR_ANY,1024000,1024000))
		return -1;
	
	FD_ZERO(&m_rfd);
	FD_SET(m_fd,&m_rfd);
	cl_net::sock_set_nonblock(m_fd,1);

	m_cnn = cnn;
	if(m_cnn)
		m_cnn->m_fd = m_fd;
	return 0;
}
int cla_acceptor::close()
{
	cl_socketbase::close_sock(m_fd);
	if(_tmp_block)
	{
		_tmp_block->free(CLA_MPTHREAD_CORE);
		_tmp_block = NULL;
	}
	return 0;
}
int cla_acceptor::handle_input()
{
	int size = 0;
	while(1)
	{
		if(NULL==_tmp_block)
		{
			_tmp_block = cla_memblock::alloc(CLA_MPTHREAD_CORE);
			if(NULL==_tmp_block) return -1;
		}
		size = recvfrom(m_fd,(char*)_tmp_block->buffer,_tmp_block->bufsize,0,(sockaddr*)&_tmp_addr,&_tmp_addr_len);
		if(size>0)
		{
			_tmp_block->datasize = size;

			if(1==m_cnn->on_acceptor_recv(_tmp_block,_tmp_addr))
				_tmp_block = NULL; //数据放有队列后续使用
			else
				_tmp_block->datasize = 0; //数据不再使用
				
			//memset(&_tmp_addr,0,_tmp_addr_len);	
		}
		else
		{
			//在些判断错误类型,如果是缓冲区不够,尝试用更大的缓冲区收掉这个垃圾包.
#ifdef _WIN32
			//int err=WSAGetLastError();
			//perror("recvfrom() err:");
#endif
			break;
		}
	}
	return 0;
}
int cla_acceptor::handle_select_read(unsigned long delay_usec/*=0*/)
{
	_timeout.tv_sec=(long)(delay_usec/1000000);
	_timeout.tv_usec=(long)(delay_usec%1000000);
	FD_ZERO(&m_rfd);
	FD_SET(m_fd,&m_rfd);
	_n = select(m_fd+1,&m_rfd,NULL,NULL,&_timeout);
	if(_n>0)
		handle_input();
	return _n;
}


