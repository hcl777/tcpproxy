
#include "cl_mysqlConn.h"
#ifdef CL_MYSQL
#include <stdio.h>
#include <string.h>
#include "cl_util.h"
#include "mysql.h"

/*
mysqlclient.lib�ı��뷽����
window���룺
1����װcmake���ߣ�
2������mysql-connector-c Դ���ѹ��
3������VC���̣� 
    ��cmake��ѡ��Դ����·����ѡ�����ɹ���·��
    ��configure��ѡ���Ѿ���װ��VSƽ̨���ٵ�generate����VC���̣�
    ������ɺ���ڹ���Ŀ¼�´���VC���̡�
4������debug����release�汾��
   �ɵ���ѡ��mysqlclient���룬�����ɹ�������mysqlclient.lib��libmysql\Debug\Ŀ¼���档
   ��ȥʹ�ü��ɡ�
*/

#ifdef _WIN32
#ifdef _DEBUG
#pragma comment(lib,"D:/source/mysql/lib/debug/mysqlclient.lib")
#else
#pragma comment(lib,"D:/source/mysql/lib/release/mysqlclient.lib")
#endif
#endif

//******************************************
//û���������ݿ�ʱ��ת����gdk��ʽ���ַ�������
//MYSQL g_gbk_mysql;
//g_gbk_mysql.charset = &my_charset_gbk_bin;

//**********************************************************************

//MYSQL_RES
cl_mysqlQuery::cl_mysqlQuery(void* pmysql)
	:m_pmysql(pmysql)
	,m_pres(NULL)
{
}
cl_mysqlQuery::~cl_mysqlQuery(void)
{
	free_result();
}
void cl_mysqlQuery::free_result()
{
	if(m_pres)
	{
		mysql_free_result((MYSQL_RES*)m_pres);
		m_pres = NULL;
	}
}
int cl_mysqlQuery::query(const char* sql,int* affected_rows/*=NULL*/)
{
	free_result();
	MYSQL* pmysql = (MYSQL*) m_pmysql;
	if(NULL==pmysql)
		return -1;

	if(0!=mysql_real_query(pmysql,sql,strlen(sql)))
	{
		printf("***mysql_real_query(%s) ERR: [%d] [%s] \n",sql,mysql_errno(pmysql),mysql_error(pmysql));
		return -2;
	}
	if(affected_rows)
		*affected_rows = (int)mysql_affected_rows(pmysql);

	//������û�н�����أ�����ִ����䶼�����Ҳ�������������mysql_errno(pmysql)==0;
	m_pres = (MYSQL_RES*)mysql_store_result(pmysql);

	if(0!=mysql_errno(pmysql))
	{
		m_pres = NULL;
		printf("***mysql_store_result(%s) ERR: [%d] [%s] \n",sql,mysql_errno(pmysql),mysql_error(pmysql));
		return -3;
	}
	return 0;
}

int cl_mysqlQuery::get_num_rows()
{
	if(NULL==m_pres)
		return 0;
	return (int)mysql_num_rows((MYSQL_RES*)m_pres);
}

int cl_mysqlQuery::get_num_fields()
{
	if(NULL==m_pres)
		return 0;
	return mysql_num_fields((MYSQL_RES*)m_pres);
}

int cl_mysqlQuery::next_row(char*** row)
{
	if(NULL==m_pres)
		return -1;
	MYSQL_ROW  myrow = mysql_fetch_row((MYSQL_RES*)m_pres);
	if(myrow)
	{
		*row = myrow;
		return 0;
	}
	return -1;
}
char* cl_mysqlQuery::escape_string(char *to,int tolen,const char *from)
{
	return cl_mysqlQuery::escape_string(to,tolen,from,m_pmysql);
}
char* cl_mysqlQuery::escape_string(char *to,int tolen,const char *from,void *db)
{
	if(0 == to || 0 == from)
		return NULL;
	int reallen = strlen(from); 
	if(reallen == 0 || tolen < (2*reallen+1))
		return NULL;

	if(db)
	{
		mysql_real_escape_string((MYSQL*)db,to,from,reallen);
	}
	else
	{
		mysql_escape_string(to,from,reallen);
	}
	return to;
}



//**********************************************************************
cl_mysqlConn::cl_mysqlConn(void)
: query(NULL)
, m_pmysql(NULL)
{
	//connect();
	m_dbcode[0]='\0';
}

cl_mysqlConn::~cl_mysqlConn(void)
{
	disconnect();
}
int cl_mysqlConn::connect(const char* dbaddr,const char* dbcode/*="utf8"*/)
{
	//user:pass@ip:port/dbname
	string ip,user,passwd,dbname;
	int port;
	string addr,str,str2;
	
	strcpy(m_dbaddr,dbaddr);
	if(dbcode)
		strcpy(m_dbcode,dbcode); //utf8 or gbk
	else
		m_dbcode[0] = '\0';
	addr = m_dbaddr;
	if(addr.empty())
		return -1;

	dbname = cl_util::get_string_index(addr,1,"/");
	str = cl_util::get_string_index(addr,0,"@");
	user = cl_util::get_string_index(str,0,":");
	passwd = cl_util::get_string_index(str,1,":");
	str = cl_util::get_string_index(addr,1,"@");
	ip = cl_util::get_string_index(str,0,":");
	port = cl_util::atoi(cl_util::get_string_index(str,1,":").c_str());

	disconnect();
	if(NULL==(m_pmysql=mysql_init(NULL)))
	{
		printf("mysql init() error\n");
		return -1;
	}

	if(NULL==mysql_real_connect((MYSQL*)m_pmysql,ip.c_str(),user.c_str(),passwd.c_str(),dbname.c_str(),port,NULL,0))
	{
		printf("mysql_real_connect error: [%d] %s\n",mysql_errno((MYSQL*)m_pmysql), mysql_error((MYSQL*)m_pmysql));
		disconnect();
		return -2;
	}
	//����д�����
	if(strlen(m_dbcode))
	{
		char names[128];
		sprintf(names,"set names %s",m_dbcode); //utf8 or gbk
		mysql_real_query((MYSQL*)m_pmysql,names,strlen(names));
	}
	query.set_mysql(m_pmysql);
	printf("#mysql connect(%s) ok! \n",m_dbaddr);
	return 0;
}


int cl_mysqlConn::disconnect()
{
	if(m_pmysql)
	{
		mysql_close((MYSQL*)m_pmysql);
		m_pmysql = NULL;
		query.set_mysql(NULL);
		mysql_library_end(); //���ô˽����������ڴ�й©
		printf("# ** dbdisconnect() ...\n");
	}
	return 0;
}
int cl_mysqlConn::ping()
{
	if(m_pmysql)
	{
		if(0!=mysql_ping((MYSQL*)m_pmysql))
			return connect(m_dbaddr);
		else
		{
			printf("#--db ping ok!--\n");
		}
	}
	else
	{
		connect(m_dbaddr);
	}
	return -1;
}

#endif

