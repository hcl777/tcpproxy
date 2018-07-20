
#include <stdio.h>
#include "cl_util.h"
#include "cl_net.h"
#include "cla_proxyServer.h"
#include "clacp.h"

void print_help();

int main(int argc,char** argv)
{
	cl_util::debug_memleak();
	cl_net::socket_init();

	int i = 0;
	unsigned short port = 12068;

	if(cl_util::string_array_find(argc,argv,"-h")>0||cl_util::string_array_find(argc,argv,"-v")>0)
	{
		print_help();
		return 0;
	}
	if((i=cl_util::string_array_find(argc,argv,"-t"))>0)
	{
		bool loop = false;
		if(cl_util::string_array_find(argc,argv,"--loop")>0)
			loop = true;
		if(i+1<argc)
		{
			cla_test_proxyServer(argv[i+1],loop);
		}
		return 0;
	}

	if((i=cl_util::string_array_find(argc,argv,"-p"))>0)
	{
		if(i+1<argc)
			port = atoi(argv[i+1]);
	}

	//*****************************
	cla_proxyServer svr;
	if(0!=svr.open(port))
		return -1;

	svr.loop();

	svr.close();
	cl_net::socket_fini();
	return 0;
}
void print_help()
{
	printf("[VERSION]: cla_proxyServer_20170601 \n");
	printf("[PARAMS]: %s [-t ip:port] [-p port(default:12068)] \n",cl_util::get_module_name().c_str());
}



