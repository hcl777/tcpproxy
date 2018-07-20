#include "cl_httprsp.h"
#include "cl_util.h"
#include "cl_net.h"
#include "cl_urlcode.h"
#include "HttpResponseHeader.h"
#include "HttpContentType.h"
#include "cl_RDBEifFile64.h"

bool g_https_exiting=false;

cl_httprsp::cl_httprsp(FUN_HANDLE_HTTP_REQ_PTR fun,void* fun_param,cl_httppubconf_t* c)
:m_fun_handle_http_req(fun)
{
	m_req = new cl_HttpRequest_t();
	m_req->fun_param = fun_param;
	m_pubconf = c;
}

cl_httprsp::~cl_httprsp(void)
{
	delete m_req;
}
int cl_httprsp::handle_req(SOCKET sock,sockaddr_in &addr)
{
	cl_HttpRequest_t* req = m_req;
	req->fd = (int)sock;
	req->addr.s_addr = addr.sin_addr.s_addr;
	int timeoutms = 10000;
	do
	{
		//ֻ֧��Get ����ͷ����֧��ͷ�����ݣ�Ҳ�����յ�"\r\n\r\n"����Ϊ��һ����������
		req->reset();

		if(0!=recv_head(req,timeoutms))
			break;

		//���body���ݱȽ��٣���������
		if(req->Content_Length>req->bodylen && req->Content_Length<=CL_HTTP_MAX_HEADLEN)
		{
			if(0!=cl_net::sock_select_recv_n(req->fd,req->body+req->bodylen,(int)(req->Content_Length-req->bodylen),timeoutms))
				break;
			req->bodylen = (int)req->Content_Length;
		}
		req->body[req->bodylen] = '\0';
		
		if(0!=handle_head(req,m_fun_handle_http_req)) //����0�ű�ʾ��ȷ���������ſ��ܼ���ѭ������(���keepalive)
			break;
	}while(is_keeplive(req->header));
	return 0;
}
int cl_httprsp::recv_head(cl_HttpRequest_t* req,int timeoutms)
{
	int n = 0;
	int readsize = 0;
	DWORD begintick = GetTickCount();
	char *p = NULL;
	char* buf = req->header;
	while(readsize<CL_HTTP_MAX_HEADLEN)
	{
		if(1==cl_net::sock_select_readable(req->fd,timeoutms))
		{
			n = recv(req->fd,buf+readsize,CL_HTTP_MAX_HEADLEN-readsize,0);
			if(n<=0)
			{
				break;
			}
			readsize+=n;
			buf[readsize] = '\0';
			p = strstr(buf,"\r\n\r\n");
			if(p)
			{
				req->bodylen = readsize - (int)(p-buf+4);
				assert(req->bodylen>=0);
				if(req->bodylen>0)
				{
					memcpy(req->body,p+4,req->bodylen);
				}
				p[4] = '\0';
				return 0;
			}
		}

		//��ʱ 10��
		if(_timer_after(GetTickCount(),(begintick + timeoutms)))
			break;
	}
	return -1;
}
int cl_httprsp::handle_head(cl_HttpRequest_t* req,FUN_HANDLE_HTTP_REQ_PTR fun)
{
	//DEBUGMSG("request------- \n %s \n",header);
	string text = "";
	if(0==get_header_field(req->header,"Content-Length",text))
	{
		req->Content_Length = cl_util::atoll(text.c_str());
	}
	char buf[CL_HTTP_MAX_HEADLEN+1];
	char* ptr = strchr(req->header,'\r');
	if(NULL==ptr) return -1;
	memcpy(buf,req->header,(int)(ptr-req->header));
	buf[(int)(ptr-req->header)] = '\0';
	string src = buf;
	string str;
	str = cl_util::get_string_index(src,0," ");
	if(str.empty()||str.length()>7) return -1;
	strcpy(req->method,str.c_str());
	str = cl_util::get_string_index(src,1," ");
	if(str.empty()||str.length()>CL_HTTP_MAX_HEADLEN) return -1;
	str = cl_urldecode(str);
	int pos = (int)str.find("?");
	if(pos>0)
	{
		strcpy(req->cgi,str.substr(0,pos).c_str());
		strcpy(req->params,str.substr(pos).c_str());
	}
	else if(pos==0)
	{
		return -1;
	}
	else
	{
		strcpy(req->cgi,str.c_str());
		req->params[0]='\0';
	}

	if(strstr(req->cgi,"/version") && !m_pubconf->ver.empty())
	{
		response_message(req->fd,m_pubconf->ver);
		return 0;
	}
	else if(fun)
	{
		return fun(req);
	}
	else
	{
		response_error((int)req->fd);
		return -1;
	}
}

int cl_httprsp::get_header_field(const string& header,const string& session, string& text)
{
	//ȡ��ĳ����ֵ,session ����":"��
	if(header.empty()) 
		return -1;
	int nPos = -1;
	nPos = (int)header.find(session,0);
	if(nPos != -1)
	{
		nPos += (int)session.length();
		nPos += 1; //��1����:��
		int nCr = (int)header.find("\r\n",nPos);
		text = header.substr(nPos,nCr - nPos);
		cl_util::string_trim(text);
		return 0;
	}
	else
	{
		return -1;
	}
}
int cl_httprsp::get_header_range(const string& header,ULONGLONG& ibegin,ULONGLONG& iend)
{
	string str;
	if(0==get_header_field(header,"Range",str))
	{
		str = cl_util::get_string_index(str,1,"=");
		ibegin = cl_util::atoll(cl_util::get_string_index(str,0,"-").c_str());
		iend = cl_util::atoll(cl_util::get_string_index(str,1,"-").c_str());
		return 0;
	}
	else
	{
		return -1;
	}
}
bool cl_httprsp::is_keeplive(const string& header)
{
	//�Ƿ�keep-alive
	string str;
	if(0==get_header_field(header,"Connection",str))
	{
		if(0==stricmp(str.c_str(),"keep-alive"))
		{
			DEBUGMSG("# cl_httprsp::req \"keep-alive\" \n");
			return true;
		}
	}
	return false;
}

void cl_httprsp::response_error(int fd,const char* msg/*=NULL*/,int code/*=404*/,int timeoutms/*=10000*/)
{
	printf("response %d",code);

	HttpResponseHeader responseHdr;

	string str = "error";
	if(msg)
		str = msg;
    
    SYSTEMTIME st;
	GetLocalTime(&st);

	responseHdr.AddStatusCode(code);
    responseHdr.AddDate(st);
    responseHdr.AddServer("sphttpsvr");
    responseHdr.AddMyAllowFields();
	responseHdr.AddContentLength((int)str.length());
    responseHdr.AddContentType("text/html");

	unsigned int tick = GetTickCount();
	int distance = 0;
    if(responseHdr.Send(fd,timeoutms))
	{
		distance = _timer_distance(GetTickCount(),tick);
		if(timeoutms>= distance)
			cl_net::sock_select_send_n(fd,str.c_str(),(int)str.length(),timeoutms-distance);
	}
}
void cl_httprsp::response_message(int fd,const char* msg,int len/*=-1*/,int code/*=200*/,int timeoutms/*=10000*/)
{
	HttpResponseHeader responseHdr;
	if(len<0)
	{
		if(msg)
			len = (int)strlen(msg);
		else
			len = 0;
	}
	if(0==len) code = 204;
    
    SYSTEMTIME st;
	GetLocalTime(&st);

	responseHdr.AddStatusCode(code);
    responseHdr.AddDate(st);
    responseHdr.AddServer("sphttpsvr");
    responseHdr.AddMyAllowFields();
	responseHdr.AddContentLength(len);
    responseHdr.AddContentType("text/html");

	unsigned int tick = GetTickCount();
	int distance = 0;
    if(responseHdr.Send(fd,timeoutms))
	{
		if(len>0)
		{
			distance = _timer_distance(GetTickCount(),tick);
			if(timeoutms>= distance)
				cl_net::sock_select_send_n(fd,msg,len,timeoutms-distance);
		}
	}
}
void cl_httprsp::response_message(int fd,const string& msg,int code/*=200*/,int timeoutms/*=10000*/)
{
	response_message(fd,msg.c_str(),(int)msg.length(),code,timeoutms);
}

void cl_httprsp::response_file(int fd,const string& path)
{
	cl_file64 file;
	if(0!=file.open(path.c_str(),F64_READ))
	{
		printf("#***httpsvr no file (%s) ;\n",path.c_str());
		response_error(fd);
		return;
	}
	size64_t size = file.seek(0,SEEK_END);
	if(size<=0)
	{
		response_error(fd);
		return;
	}
	file.seek(0,SEEK_SET);

	HttpResponseHeader responseHdr;
	SYSTEMTIME tm;
	GetLocalTime(&tm);

	responseHdr.AddStatusCode(200);
	responseHdr.AddString("Access-Control-Allow-Origin: *\r\n");
    responseHdr.AddDate(tm);
    responseHdr.AddServer("sphttpsvr");
    responseHdr.AddMyAllowFields();
	responseHdr.AddContentLength((long long)size);
	responseHdr.AddContentType(http_content_type[cl_util::get_filename_extension(path)]);
	responseHdr.Send(fd,10000);

	char *buf=new char[4096];
	size64_t rdsize = 0;
	//
	int n = 0;
	while(rdsize<size)
	{
		n = file.read(buf,4096);
		if(n<=0)
			break;
		if(0!=cl_net::sock_send_n(fd,buf,n))
			break;
		rdsize += n;
	}
	delete[] buf;
	file.close();
	cl_net::sock_select_readable(fd,10000);//����10���ӣ����Է�����ر�ʱ����ɶ���
}

void cl_httprsp::response_rdbfile(cl_HttpRequest_t* req,const string& path)
{
	cl_ERDBFile64 file;
	int fd = req->fd;
	size64_t size = rdbeif_get_mainfile_size(path.c_str());

	if(0==size || 0!=file.open(path.c_str(),F64_READ,RDBF_AUTO))
	{
		response_error(fd);
		DEBUGMSG("#*** cl_httprsp::response_rdbfile(%s) file open fail! \n",path.c_str());
		return;
	}

	//content_type
	string content_type = http_content_type[cl_util::get_filename_extension(req->cgi)];
	//range
	bool brange = false;
	size64_t ibegin = 0,iend = size-1;
	size64_t rsp_size = 0,pos=0;
	string str;
	if(0==get_header_field(req->header,"Range",str))
	{
		brange = true;
		str = cl_util::get_string_index(str,1,"=");
		ibegin = cl_util::atoll(cl_util::get_string_index(str,0,"-").c_str());
		iend = cl_util::atoll(cl_util::get_string_index(str,1,"-").c_str());
	}
	if(0==iend)
		iend = size-1;
	rsp_size = iend+1-ibegin;

	//response header
	HttpResponseHeader responseHdr;
	SYSTEMTIME tm;
	GetLocalTime(&tm);

	if (brange)
		responseHdr.AddStatusCode(206);
	else
		responseHdr.AddStatusCode(200);
	responseHdr.AddString("Access-Control-Allow-Origin: *\r\n");
    responseHdr.AddDate(tm);
	responseHdr.AddString("Accept-Ranges: bytes\r\n");
    responseHdr.AddServer("fileserver");
	responseHdr.AddContentLength((long long)rsp_size);
	if(brange)
	{
		char s[256];
		sprintf(s,"Content-Range: bytes %lld-%lld/%lld\r\n", ibegin, iend, size);
		responseHdr.AddString(s);
	}
	responseHdr.AddContentType(content_type.c_str());
	if(is_keeplive(req->header))
		responseHdr.AddString("Keep-Alive: timeout=15, max=100\r\nConnection: Keep-Alive\r\n");
	else
		responseHdr.AddString("Connection: close\r\n");
	responseHdr.Send(fd,10000);

	//send data:
	char *buf=new char[4096];
	int n = 0;
	pos = ibegin;
	DEBUGMSG("# cl_httprsp::response_rdbfile::(beg=%lld,end=%lld,size=%lld) \n"
		,ibegin,iend,size);
	file.seek(pos,SEEK_SET);
	while(pos <= iend && !g_https_exiting)
	{
		n = 4096;
		if((size64_t)n>(iend-pos+1))
			n = (int)(iend-pos+1);
		n = file.read(buf,n);
		if(n<=0)
			break;
		if(0!=cl_net::sock_send_n(fd,buf,n))
			break;
		pos += n;
	}
	delete[] buf;
	
	file.close();
	Sleep(100);
	DEBUGMSG("# cl_httprsp::response_rdbfile:: send end(pos=%lld, iend=%lld) \n",
		pos,iend);
	cl_net::sock_select_readable(fd,1000);
}

