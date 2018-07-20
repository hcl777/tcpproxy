#pragma once

#include "cl_basetypes.h"
#include "cl_singleton.h"

#define CLLIC_SVR_VERSION "cllic_svr_20171113"
class setting
{
public:
	setting(void);
	~setting(void);

	int load();
	string get_ntp_server1();
protected:
	GETSET(unsigned short,m_port,_port);
	GETSET(string,m_dbaddr,_dbaddr);
	GETSET(string,m_ntp_server,_ntp_server);

	string inipath;
};

typedef cl_singleton<setting> settingSngl;
