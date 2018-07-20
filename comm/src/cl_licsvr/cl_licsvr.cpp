#include "cl_licsvr.h"
#include "setting.h"
#include "cl_util.h"
#include "cl_lic.h"

cl_licsvr::cl_licsvr(void)
	:m_binit(false)
{
	m_enddate = cl_util::time_to_date_string(time(NULL)+3600*24*180);
}


cl_licsvr::~cl_licsvr(void)
{
}
int cl_licsvr::init()
{
	if(m_binit) return 1;
	if(0!=m_db.connect(settingSngl::instance()->get_dbaddr().c_str()))
		return -1;
	m_binit = true;
	activate();
	return 0;
}
void cl_licsvr::fini()
{
	if(!m_binit)
		return;
	m_binit = false;
	wait();
	m_db.disconnect();
}
int cl_licsvr::work(int e)
{
	int n = 0;
	while(m_binit)
	{
		Sleep(1000);
		if(++n>30)
		{
			n = 0;
			m_enddate = cl_util::time_to_date_string(time(NULL)+3600*24*360);
			Lock l(m_mt);
			m_db.ping();
		}
	}
	return 0;
}
int cl_licsvr::get_lic(const string& peer_name,const string& ver,const string& ip,unsigned int n,unsigned int e,char* buf,int* size)
{
	Lock l(m_mt);
	if(peer_name.empty()) return -1;
	
	int ret;
	char sql[1024];
	int affected_rows=0;
	char** row;
	bool binsert = false;
	cl_licinfo_t lic;

	lic.peer_name = peer_name;
	lic.ntp_server = settingSngl::instance()->get_ntp_server1();
	

	//查询
	sprintf(sql,"select id,end_date from lic_peer where peer_name='%s';",peer_name.c_str());
	ret = m_db.query.query(sql,&affected_rows);
	if(0!=ret) return CLL_ERR_DB_WRONG;
	if(m_db.query.get_num_rows()<=0)
	{
		//插入
		char sql2[1024];
		sprintf(sql2,"insert into lic_peer(peer_name,ver,ip,end_date,alive_time,create_time)"
			" values('%s','%s','%s','%s',now(),now())",peer_name.c_str(),ver.c_str()
			,ip.c_str(),m_enddate.c_str());
		ret = m_db.query.query(sql2,&affected_rows);
		binsert = true;
		ret = m_db.query.query(sql,&affected_rows);
	}
	if(0==ret && m_db.query.get_num_rows()>0)
	{
		m_db.query.next_row(&row);
		lic.end_time = row[1];
		if(!binsert)
		{
			//更新
			sprintf(sql,"update lic_peer set ver='%s',ip='%s',alive_time=now() where id=%d;",ver.c_str(),ip.c_str(),atoi(row[0]));
			ret = m_db.query.query(sql,&affected_rows);
		}
	}
	else
	{
		return CLL_ERR_DB_WRONG;
	}

	DEBUGMSG("peer license(%s,%s)\n",lic.peer_name.c_str(),lic.end_time.c_str());
	//生成rsabuf
	return cllic_lic_to_rsabuf(&lic,n,e,buf,size);
}


