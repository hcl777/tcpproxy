
#include <stdio.h>
#include "cl_util.h"
#include "cl_net.h"
#include "clu.h"


int main(int argc,char** argv)
{


	cl_util::debug_memleak();
	cl_util::chdir(cl_util::get_module_dir().c_str());

	cl_net::socket_init();

	clu_init(7891,NULL);

	while('q'!=getchar())
		Sleep(1000);

	clu_fini();

	cl_net::socket_fini();
	return 0;
}

