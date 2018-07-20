#pragma once
#include "clim.h"
#include "cl_incnet.h"

class clim_client
{
public:
	clim_client(void);
	~clim_client(void);

public:
	int		open(const char* svr,CLIM_MESSAGE func,void* func_param);
	void	close();
	void	root(); //一次工作循环
	int		send(char* data,int size);

private:
	void	reset();
private:
	int				m_state;
	SOCKET			m_fd;
	CLIM_MESSAGE	m_func;
	void*			m_func_param;
};

