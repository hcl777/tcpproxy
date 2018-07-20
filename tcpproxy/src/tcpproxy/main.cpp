
#include <stdio.h>
#include "Util.h"
#include "Setting.h"
#include "TCPPeerMgr.h"
#include "SelectReactor.h"
#include "Timer.h"
#include "SchedulerBase.h"
#include "MemBlock.h"

int init();
void fini();
int main(int argc,char** argv)
{
	Util::debug_memleak();
	void* sngl = Util::process_single_lockname_create();
	if(NULL==sngl)
	{
		printf("*** exe is runing !!!");
		return 0;
	}
	Util::chdir(Util::get_module_dir().c_str());
	
	init();

	while('q'!=getchar())
		Sleep(1000);

	fini();

	Util::process_single_lockname_close(sngl);
	return 0;

}


int init()
{
	Util::socket_init();
	IOReactorSngl::instance(new SelectReactor());
	TimerSngl::instance();
	MemBlockPoolSngl::instance(new MemBlockPoolBase());
	MemBlockPoolSngl::instance()->init();
	SettingSngl::instance()->init();
	TCPPeerMgrSngl::instance()->init();
	
	SchedulerBaseSngl::instance()->run();
	return 0;
}
void fini()
{
	SchedulerBaseSngl::instance()->end();

	TCPPeerMgrSngl::instance()->fini();
	SettingSngl::instance()->fini();
	MemBlockPoolSngl::instance()->fini();

	TCPPeerMgrSngl::destroy();
	SettingSngl::destroy();
	MemBlockPoolSngl::destroy();
	TimerSngl::destroy();
	IOReactorSngl::destroy();
	SchedulerBaseSngl::destroy();

	Util::socket_fini();

}

