#pragma once
#include "cl_incnet.h"
#include "cl_basetypes.h"
#include "cl_net.h"

#define CL_HTTP_MAX_HEADLEN 2047
typedef struct tagcl_HttpRequest
{
	int fd;
	char header[CL_HTTP_MAX_HEADLEN+1];
	char cgi[CL_HTTP_MAX_HEADLEN+1];
	char params[CL_HTTP_MAX_HEADLEN+1];
	char method[8];  //GET,POST
	unsigned long long Content_Length;
	char body[CL_HTTP_MAX_HEADLEN+1];
	unsigned int bodylen;

	struct in_addr addr;
	void* fun_param; //应用程序参数

	tagcl_HttpRequest(void)
	{
		fd = 0;
		fun_param = NULL;
		reset();
	}
	void reset()
	{
		header[0] = '\0';
		cgi[0] = '\0';
		params[0] = '\0';
		method[0] = '\0';
		Content_Length = 0;
		body[0] = '\0';
		bodylen = 0;
	}
}cl_HttpRequest_t;

typedef int (*FUN_HANDLE_HTTP_REQ_PTR)(cl_HttpRequest_t*);

typedef struct tag_cl_httppubconf
{
	string ver; // /version返回
}cl_httppubconf_t;

class cl_httprsp
{
public:
	cl_httprsp(FUN_HANDLE_HTTP_REQ_PTR fun,void* fun_param,cl_httppubconf_t* c);
	~cl_httprsp(void);

public:
	int handle_req(SOCKET sock,sockaddr_in &addr);

private:
	cl_HttpRequest_t*			m_req;
	FUN_HANDLE_HTTP_REQ_PTR		m_fun_handle_http_req;
	cl_httppubconf_t			*m_pubconf;

public:
	int recv_head(cl_HttpRequest_t* req,int timeoutms);
	int handle_head(cl_HttpRequest_t* req,FUN_HANDLE_HTTP_REQ_PTR fun);

	static int get_header_field(const string& header,const string& session, string& text);
	static int get_header_range(const string& header,ULONGLONG& ibegin,ULONGLONG& iend);
	static bool is_keeplive(const string& header);

	static void response_error(int fd,const char* msg=NULL,int code=404,int timeoutms=10000);
	static void response_message(int fd,const char* msg,int len=-1,int code=200,int timeoutms=10000);
	static void response_message(int fd,const string& msg,int code=200,int timeoutms=10000);
	static void response_file(int fd,const string& path);
	static void response_rdbfile(cl_HttpRequest_t* req,const string& path);
};
extern bool g_https_exiting;
