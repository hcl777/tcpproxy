#pragma once
#include "cl_basetypes.h"
#include "cl_thread.h"
/*
˵��:�����¼������Ϣ���汾�ز����ϱ�������.
�ϱ���¼Ϊ: ������|�汾��|������|����(125�ֽ�)
*/
class cl_error : public cl_thread
{
public:
	cl_error(void);
	virtual ~cl_error(void);

public:
	void set_error(int err); //�����open,����ʱ��
	int get_error()const {return m_last_err;}

	int open_log(const string& appname,const string& appver,
		const string& logpath,const string& report_url);
	void close();

public:
	int m_last_err;
	bool m_bopen;
	string m_appname,m_appver;
	string m_url; //��¼�ϱ�·��.
	string m_localpath; //���û���.
	string m_reportpath; //�ϱ�����ɾ��.
};

