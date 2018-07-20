#include "https.h"
#include "cl_licsvr.h"
#include "setting.h"
#include "cl_util.h"

https::https(void)
{
}


https::~https(void)
{
}
int https::init()
{
	if(m_svr.is_open())
		return 1;
	settingSngl::instance()->load();
	if(0!=cl_licsvrSngl::instance()->init())
	{
		fini();
		return -1;
	}
	if(0!=m_svr.open(settingSngl::instance()->get_port(),NULL,on_request,this,true,20,CLLIC_SVR_VERSION))
	{
		fini();
		return -1;
	}

	return 0;
}
void https::fini()
{
	m_svr.stop();
	cl_licsvrSngl::instance()->fini();

	cl_licsvrSngl::destroy();
	settingSngl::destroy();
}
int https::on_request(cl_HttpRequest_t* req)
{
	https* ph = (https*)req->fun_param;
#define EIF(func) else if(strstr(req->cgi,"/lic/"#func".php")) ph->func(req)

	if(0==strcmp(req->cgi,"/") || strstr(req->cgi,"/index."))
		ph->index(req);
	EIF(licd);
	else
	{
		string path = cl_util::get_module_dir()+ &req->cgi[1];
		cl_httprsp::response_file(req->fd,path);
	}

	return 0;
}
void https::index(cl_HttpRequest_t* req)
{
	char buf[4096];
#define CAT_TITLE(s) sprintf(buf+strlen(buf),"<br><strong><font>%s</font></strong><br>",s)
#define CAT(cgi) sprintf(buf+strlen(buf),"<a target=\"_blank\" href=\"%s\">%s</a><br>",cgi,cgi)
	
	sprintf(buf,"<h3>HTTP API(cl_licsvr):</h3>");
	CAT("/version");
	CAT("/lic/licd.php?peer_name=&ver=&n=&e=");
	cl_httprsp::response_message(req->fd,buf);
}
void https::licd(cl_HttpRequest_t* req)
{
	char buf[4096];
	int size=4096;
	string params, peer_name,ver,ip;
	unsigned int n,e;
	
	params = req->params;
	peer_name = cl_util::url_get_parameter(params,"peer_name");
	ver = cl_util::url_get_parameter(params,"ver");
	n = atoi(cl_util::url_get_parameter(params,"n").c_str());
	e = atoi(cl_util::url_get_parameter(params,"e").c_str());
	n += e;
	ip = cl_net::ip_ntoa(req->addr.s_addr);

	if(peer_name.empty() || 0==n || 0==e)
	{
		cl_httprsp::response_error(req->fd);
		return;
	}

	if(0!=cl_licsvrSngl::instance()->get_lic(peer_name,ver,ip,n,e,buf,&size))
	{
		cl_httprsp::response_error(req->fd);
		return;
	}
	cl_httprsp::response_message(req->fd,buf,size);
}
