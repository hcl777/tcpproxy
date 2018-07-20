
#include <stdio.h>
#include "cl_runner.h"
#include "cl_util.h"

void print_help();
int main(int argc,char** argv)
{
	if(cl_util::string_array_find(argc,argv,"-h")>0)
	{
		print_help();
		return 0;
	}
	else if(cl_util::string_array_find(argc,argv,"stop")>0)
	{
		cl_runner::stop();
		return 0;
	}
	else
	{
		cl_util::debug_memleak();
		PSL_HANDLE	pslh = cl_util::process_single_lockname_create();
		if(pslh==0)
		{
			printf("# *** %s is runing...!\n",cl_util::get_module_name().c_str());
			return 0;
		}

		printf("cl_runner start...\n");
		cl_runner run;
		if(0!=run.run())
		{
			return 0;
		}
		while(1)
			Sleep(1000);
		run.end();

		cl_util::process_single_lockname_close(pslh);
		return 0;
	}
}
void print_help()
{
	printf("%-12s = %s\n","[version]","cl_runner_20170803");
	printf("[params]: [stop]; \n");
	printf("[cl_runner.conf]:\n"
		"/opt/app1/text1.exe 123\n"
		"/opt/app2/text2.exe\n");
}

