#include "cla_proto.h"
#include <string.h>

int operator << (cl_ptlstream& ps, const sockaddr_in& addr)
{
	ps << (unsigned int)addr.sin_addr.s_addr;
	ps << (unsigned short)addr.sin_port;
	return ps.ok();
}
int operator >> (cl_ptlstream& ps, sockaddr_in& addr)
{
	cla_addr ca;
	ps >> ca;
	addr.sin_addr.s_addr = ca.ip;
	addr.sin_port = ca.port;
	return ps.ok();
}

int operator << (cl_ptlstream& ps, const cla_addr& inf)
{
	ps << inf.ip;
	ps << inf.port;
	return ps.ok();
}

int operator >> (cl_ptlstream& ps, cla_addr& inf)
{
	ps >> inf.ip;
	ps >> inf.port;
	return ps.ok();
}

int operator << (cl_ptlstream& ps, const cla_session_t& inf)
{
	ps << inf.sid;
	ps << inf.did;
	return ps.ok();
}
int operator >> (cl_ptlstream& ps, cla_session_t& inf)
{
	ps >> inf.sid;
	ps >> inf.did;
	return ps.ok();
}


////发送proxy_send 包,size已含proxy head
//int cla_ptl_send_proxy(int fd,uchar cmd,cl_ptlstream& ps,char* buf,int size,sockaddr_in& to,const cla_addr& naddr)
//{
//	if(0==to.sin_addr.s_addr)
//		return -1;
//	ps.attach(buf,size);
//	ps << cmd;
//	ps << naddr;
//	return ::sendto(fd,buf,size,0,(const sockaddr*)&to,sizeof(to));
//}

void cla_ptl_proxy_send2recv(char* buf,sockaddr_in& addr_io)
{
	//当BUF为小字节序处理
	//保存的数据为nip
	uint32 des_nip,src_nip;
	uint16 des_nport,src_nport;

	memcpy(&des_nip,buf+1,4);
	memcpy(&des_nport,buf+5,2);
	src_nip = cl_bstream::htol32(addr_io.sin_addr.s_addr);
	src_nport = cl_bstream::htol16(addr_io.sin_port);

	memcpy(buf+1,&src_nip,4);
	memcpy(buf+5,&src_nport,2);
	addr_io.sin_addr.s_addr = cl_bstream::ltoh32(des_nip);
	addr_io.sin_port = cl_bstream::ltoh16(des_nport);

	buf[0] = CLA_PTL_PROXY_RECV;
}

