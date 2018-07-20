
#include <stdio.h>

#include "https.h"
#include "cl_util.h"
#include "cl_net.h"



int main(int argc,char** argv)
{
	cl_util::debug_memleak();
	cl_util::chdir(cl_util::get_module_dir().c_str());
	PSL_HANDLE pslh = cl_util::process_single_lockname_create();
	if(0==pslh)
	{
		printf("*** %s is runing ***\n",cl_util::get_module_name().c_str());
		return -1;
	}
	cl_net::socket_init();

	https hs;
	if(0!=hs.init())
	{
		printf("#*** https init faild *** \n");
		goto end;
	}

#ifdef _DEBUG
	while('q'!=getchar())
		Sleep(1000);
#else
	while(1)
		Sleep(10000);
#endif
	hs.fini();

end:
	cl_net::socket_fini();
	cl_util::process_single_lockname_close(pslh);
	return 0;
}


