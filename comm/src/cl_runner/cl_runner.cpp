#include "cl_runner.h"
#include "cl_util.h"

cl_runner::cl_runner(void)
	:m_brun(false)
{
}


cl_runner::~cl_runner(void)
{
}

int cl_runner::run()
{
	if(m_brun) return 1;
	cl_util::get_stringlist_from_file(cl_util::get_module_dir()+"cl_runner.conf",m_ls);
	if(m_ls.empty())
		return -1;
	m_brun = true;
	this->activate(m_ls.size());
	return 0;
}

void cl_runner::end()
{
	if(m_brun)
		m_brun = false;
	wait();
	m_ls.clear();
}

int cl_runner::work(int e)
{
	int i=0;
	string cmd;
	list<string>::iterator it=m_ls.begin();
	for(;i<e;++it,++i);
	cmd = *it;
	string path = cl_util::get_module_dir()+"cl_runner.log";
	while(m_brun)
	{
		cl_util::write_tlog(path.c_str(),1024,"thread_%d +++ run(%s) begin...",e,cmd.c_str());
		system(cmd.c_str());
		cl_util::write_tlog(path.c_str(),1024,"thread_%d --- run(%s) end!",e,cmd.c_str());
		Sleep(30000);
	}

	return 0;
}

#ifdef _WIN32
	//可使用 tasklist, tskill pid; 的方式实现
int cl_runner::stop()
{
	return -1;
}
#endif
#ifdef __GNUC__
int cl_get_pid_by_name(const string& name,list<string>& ls_pids)
{
	char cmd[1024];
	string tmppath = cl_util::get_module_dir()+"~system.txt";

	ls_pids.clear();
	cl_util::file_delete(tmppath);
	sprintf(cmd,"ps x|grep '%s' | awk '{print $1}' > %s",name.c_str(),tmppath.c_str());
	system(cmd); //不管执行成功与否
	cl_util::get_stringlist_from_file(tmppath,ls_pids);
	return 0;
}
int cl_kill_pids(list<string>& ls_pids)
{
	char cmd[1024];
	for(list<string>::iterator it=ls_pids.begin();it!=ls_pids.end();++it)
	{
		sprintf(cmd,"kill -9 %s",(*it).c_str());
		printf("#system( %s )\n",cmd);
		system(cmd);
	}
	return 0;
}
int cl_runner::stop()
{
	int pid = 0;
	pid = (int)getpid();
	list<string> ls_pids;
	list<string> ls;
	list<string>::iterator it;
	string str;

	//先kill cl_runner,排除掉自己的pid
	cl_get_pid_by_name(cl_util::get_module_name(),ls_pids);
	for(it=ls_pids.begin();it!=ls_pids.end();)
	{
		if(pid == cl_util::atoi((*it).c_str()))
			ls_pids.erase(it++);
		else
			++it;
	}
	printf("# ----[[[ try kill %s ]]]---:\n",cl_util::get_module_name().c_str());
	cl_kill_pids(ls_pids);

	cl_util::get_stringlist_from_file(cl_util::get_module_dir()+"cl_runner.conf",ls);
	for(it=ls.begin();it!=ls.end();it++)
	{
		str = *it;
		cl_util::str_replace(str,"\"","");
		cl_util::str_replace(str,"'","");
		printf("#----[[[ try kill %s ]]]---:\n",str.c_str());
		cl_get_pid_by_name(str,ls_pids);
		cl_kill_pids(ls_pids);
	}
	
	return 0;
}

#endif

