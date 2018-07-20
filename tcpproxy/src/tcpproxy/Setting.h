#pragma once
#include "Singleton.h"
#include "basetypes.h"
class Setting
{
public:
	Setting(void);
	~Setting(void);

	int init();
	void fini();

protected:
	GETSET(unsigned short,m_listen_port,_listen_port);
	GETSET(string,m_server_ip,_server_ip);
	GETSET(unsigned short,m_server_port,_server_port);
	GETSET(unsigned short,m_is_writelog,_is_writelog);
	GETSET(unsigned short,m_is_writeseq,_is_writeseq);
};

typedef Singleton<Setting> SettingSngl;
