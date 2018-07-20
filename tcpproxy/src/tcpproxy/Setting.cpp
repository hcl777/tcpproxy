#include "Setting.h"
#include "IniFile.h"
#include "Util.h"

Setting::Setting(void)
{
	m_listen_port = 7780;
	m_server_ip = "";
	m_server_port = 0;
}

Setting::~Setting(void)
{
}
int Setting::init()
{
	IniFile ini;
	char buf[1024];
	if(0==ini.open("./tcpproxy.ini"))
	{
		m_listen_port = ini.read_int("public","listen_port",7780);
		ini.write_int("public","listen_port",m_listen_port);

		m_server_ip = ini.read_string("public","server_ip","",buf,1024);
		ini.write_string("public","server_ip",m_server_ip.c_str());
		m_server_port = ini.read_int("public","server_port",0);
		ini.write_int("public","server_port",m_server_port);

		m_is_writelog = ini.read_int("public","is_writelog",1);
		ini.write_int("public","is_writelog",m_is_writelog);

		m_is_writeseq = ini.read_int("public","is_writeseq",1);
		ini.write_int("public","is_writeseq",m_is_writeseq);

		ini.close();
	}
	Util::my_create_directory("./log");
	return 0;
}
void Setting::fini()
{
}

