#include "cla_protoCnn.h"


int operator << (cl_ptlstream& ps, const cla_ptl_cnn_proxy_t& inf)
{
	ps << inf.connid;
	ps << inf.src;
	ps << inf.proxy;
	return ps.ok();
}
int operator >> (cl_ptlstream& ps, cla_ptl_cnn_proxy_t& inf)
{
	ps >> inf.connid;
	ps >> inf.src;
	ps >> inf.proxy;
	return ps.ok();
}

int operator << (cl_ptlstream& ps, const cla_ptl_cnn_data_t& inf)
{
	ps << inf.seq;
	ps << inf.cycle_seq;
	return ps.ok();
}
int operator >> (cl_ptlstream& ps, cla_ptl_cnn_data_t& inf)
{
	ps >> inf.seq;
	ps >> inf.cycle_seq;
	return ps.ok();
}

int operator << (cl_ptlstream& ps, const cla_ptl_cnn_data_ack_t& inf)
{
	return ps.ok();
}
int operator >> (cl_ptlstream& ps, cla_ptl_cnn_data_ack_t& inf)
{
	return ps.ok();
}




