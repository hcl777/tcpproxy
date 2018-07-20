
#include <stdio.h>
#include "cl_util.h"
#include "cl_lic.h"
#include "cl_ntp.h"
#include "cl_net.h"

void print_help();

void lic_publish(const char* strlic);
void lic_test(const char* path);
int main(int argc,char** argv)
{
	cl_net::socket_init();
	int i = 0;
	if((i=cl_util::string_array_find(argc,argv,"-p"))>0)
	{
		if(i+1<argc)
			lic_publish(argv[i+1]);
	}
	else if((i=cl_util::string_array_find(argc,argv,"-t"))>0)
	{
		if(i+1<argc)
			lic_test(argv[i+1]);
	}
	else
		print_help();

	cl_net::socket_fini();
	return 0;
}

void print_help()
{
	printf("params -p peer_name|end_time|ntp_server --- publish a license!\n");
	printf("params -t path  --- check the license!\n");
}
void lic_publish(const char* strlic)
{
	cl_licinfo_t lic;
	string str = strlic;
	string path;
	lic.peer_name = cl_util::get_string_index(str,0,"|");
	lic.end_time = cl_util::get_string_index(str,1,"|");
	lic.ntp_server = cl_util::get_string_index(str,2,"|");
	path = cl_util::get_module_dir()+lic.peer_name + ".core";
	if(0==cllic_lic_to_file(&lic,path.c_str(),true))
		printf(" publish license(%s,%s,%s) => %s ok! \n",lic.peer_name.c_str(),lic.end_time.c_str(),lic.ntp_server.c_str(),path.c_str());
	else
		printf(" ***publish license fail! \n");
}
void lic_test(const char* path)
{
	cl_licinfo_t lic;
	if(0!=cllic_file_to_lic(path,&lic,true))
	{
		printf(" *** load license(%s) fail! \n",path);
		return;
	}
	

	printf(" license(%s,%s,%s): \n",lic.peer_name.c_str(),lic.end_time.c_str(),lic.ntp_server.c_str());
	printf(" ntp_date	: %s \n",cl_util::time_to_date_string((time_t)cl_ntp_get_sec1970(lic.ntp_server.c_str())).c_str());
	printf(" local_date	: %s \n",cl_util::time_to_date_string(time(NULL)).c_str());
}
