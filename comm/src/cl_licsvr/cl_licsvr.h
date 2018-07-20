#pragma once
#include "cl_basetypes.h"
#include "cl_singleton.h"
#include "cl_mysqlConn.h"
#include "cl_synchro.h"
#include "cl_thread.h"

#define CLL_ERR_SUCCEED 0
#define CLL_ERR_DB_WRONG	1

class cl_licsvr : public cl_thread
{
public:
	cl_licsvr(void);
	virtual ~cl_licsvr(void);

	typedef cl_SimpleMutex Mutex;
	typedef cl_TLock<Mutex> Lock;
public:
	int init();
	void fini();
	virtual int work(int e);
	int get_lic(const string& peer_name,const string& ver,const string& ip,unsigned int n,unsigned int e,char* buf,int* size);

private:
	bool			m_binit;
	Mutex			m_mt;
	cl_mysqlConn	m_db;
	string			m_enddate;
};

typedef cl_singleton<cl_licsvr> cl_licsvrSngl;

