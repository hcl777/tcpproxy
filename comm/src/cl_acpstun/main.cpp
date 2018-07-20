
#include <stdio.h>
#include "cla_stunServer.h"
#include "cl_util.h"
#include "cl_net.h"
#include "cla_stunServer.h"
#include "cl_inifile.h"

void print_help();
int load_conf(cla_addr& stunA,cla_addr& stunB);
int main(int argc,char** argv)
{
	cl_util::debug_memleak();
	cl_net::socket_init();


	if(cl_util::string_array_find(argc,argv,"-h")>0||cl_util::string_array_find(argc,argv,"-v")>0)
	{
		print_help();
		return 0;
	}

	//*****************************
	cla_addr stunA,stunB;
	cla_stunServer svr;
	if(0!=load_conf(stunA,stunB))
	{
		printf("*** load conf fail! (see -h)\n");
		return -1;
	}
	if(0!=svr.open(stunA,stunB))
		return -1;

	svr.loop(); //Ñ­»·Ö´ÐÐ

	svr.close();
	cl_net::socket_fini();
	return 0;
}
void print_help()
{
	printf("[VERSION]: cla_proxyStun_20170601 \n");
	printf("set acpstun.ini like this format:\n"
		"[server]\n"
		"stunA=0.0.0.0:8337\n"
		"stunB=127.0.0.1:8337\n");
}
int load_conf(cla_addr& stunA,cla_addr& stunB)
{
	char buf[1024];
	string str;
	cl_inifile ini;
	if(0!=ini.open((cl_util::get_module_dir()+"acpstun.ini").c_str()))
		return -1;
	str = ini.read_string("server","stunA","",buf,1024);
	stunA.ip = htonl(cl_net::ip_atoh(cl_util::get_string_index(str,0,":").c_str()));
	stunA.port = (unsigned short)htons(cl_util::atoi(cl_util::get_string_index(str,1,":").c_str()));
	
	str = ini.read_string("server","stunB","",buf,1024);
	stunB.ip = htonl(cl_net::ip_atoh(cl_util::get_string_index(str,0,":").c_str()));
	stunB.port = (unsigned short)htons(cl_util::atoi(cl_util::get_string_index(str,1,":").c_str()));
	if(0==stunA.port || 0==stunB.ip || 0==stunB.port)
		return -1;

	printf("#setting::stunA(%s:%d) stunB(%s:%d) \n",
		cl_net::ip_ntoas(stunA.ip).c_str(),(int)ntohs(stunA.port),
		cl_net::ip_ntoas(stunB.ip).c_str(),(int)ntohs(stunB.port));
	return 0;
}

