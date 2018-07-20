#include "setting.h"
#include "cl_inifile.h"
#include "cl_util.h"

#define DEFAULT_PORT 9980
#define DEFAULT_DBADDR  "cly:111@127.0.0.1:3306/cllic"
#define DEFAULT_NTP_SERVER "us.ntp.org.cn"

setting::setting(void)
{
	m_port = DEFAULT_PORT;
	m_dbaddr = DEFAULT_DBADDR;
	m_ntp_server = DEFAULT_NTP_SERVER;
	inipath = cl_util::get_module_dir()+"cl_licsvr.ini";
}


setting::~setting(void)
{
}

int setting::load()
{

	string str;
	char buf[1024];
	cl_inifile ini;
	if(0==ini.open(inipath.c_str()))
	{
		//**************************************************
		//license
		m_port = ini.read_int("public","port",DEFAULT_PORT);
		ini.write_int("public","port",m_port);
		
		m_dbaddr = ini.read_string("public","dbaddr",DEFAULT_DBADDR,buf,1024);
		ini.write_string("public","dbaddr",m_dbaddr.c_str());

		m_ntp_server = ini.read_string("public","ntp_server",DEFAULT_NTP_SERVER,buf,1024);
		ini.write_string("public","ntp_server",m_ntp_server.c_str());

		//************************************************
		ini.close();
	}
	return 0;
}
string setting::get_ntp_server1()
{
	string str;
	char buf[1024];
	GetPrivateProfileStringA("public","ntp_server","",buf,1024,inipath.c_str());
	str = buf;
	if(str.empty()) str = DEFAULT_NTP_SERVER;
	return str;
}
