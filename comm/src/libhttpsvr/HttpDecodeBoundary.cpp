#include "HttpDecodeBoundary.h"
#include "cl_httprsp.h"
#include "cl_util.h"

HttpDecodeBoundary::HttpDecodeBoundary(const char* boundary)
{
	//�ֶθ�ʽΪ"--�ָ���",���һ���ָ�����"--�ָ���--"
	if(strlen(boundary)>0)
	{
		m_boundary = "\r\n--";
		m_boundary += boundary;
		m_mem.tryputsize(4096);
		m_mem.buf[0] = '\r';
		m_mem.buf[1] = '\n';
		m_mem.datasize = 2;
	}
	else
		m_boundary = "";
}

HttpDecodeBoundary::~HttpDecodeBoundary(void)
{
}

const char* str_memery_find(const char* srcbuf,int size,const char* desstr)
{
	const char *p=srcbuf;
	int i=0,j=0;
	int len = (int)strlen(desstr);
	assert(len>0);
	for(i=0;i<=size-len;++i)
	{
		for(j=0;j<len;++j)
		{
			if( *(p+i+j) != desstr[j] )
			{
				break;
			}
		}
		if(j==len)
			return p+i;
	}
	return NULL;
}

int HttpDecodeBoundary::put_in(const char* buf,int size)
{
	if(size<=0)
		return 0;
	if(!m_mem.tryputsize(size)) return -1;
	memcpy(m_mem.buf+m_mem.datasize,buf,size);
	m_mem.datasize += size;
	return 0;
}
int HttpDecodeBoundary::get_out(char *buf,int size)
{
	int len = 0;
	if(m_boundary.empty())
	{
		//ֱ�ӿ���ȫ������
		len = size> m_mem.datasize? m_mem.datasize:size;
		if(len>0) memcpy(buf,m_mem.buf,len);
		m_mem.datasize -= len;
		if(len && m_mem.datasize) memmove(m_mem.buf,m_mem.buf+len,m_mem.datasize);
		return len;
	}
	//������һ��boundary,��������֮ǰ������
	int copysize = 0;
	int cplen = 0;
	const char *ptr;
	while(copysize<size && m_mem.datasize>0)
	{
		m_mem.buf[m_mem.datasize] = '\0';
		//ptr = strstr(m_mem.buf,m_boundary.c_str());
		ptr = str_memery_find(m_mem.buf,m_mem.datasize,m_boundary.c_str());
		if(ptr)
		{
			//ǰ�������ݵĻ��Ϳ���
			len = (int)(ptr-m_mem.buf);
			if(len>0)
			{
				cplen = len;
				if(cplen+copysize>size) cplen = size-copysize;
				memcpy(buf+copysize,m_mem.buf,cplen);
				copysize += cplen;
				m_mem.datasize-=cplen;
				if(m_mem.datasize) memmove(m_mem.buf,m_mem.buf+cplen,m_mem.datasize);
				len -= cplen;
			}
			if(0==len)
			{
				//����˫�س���
				m_mem.buf[m_mem.datasize] = '\0';
				ptr = strstr(m_mem.buf,"\r\n\r\n");
				if(!ptr)
					break; //�Ҳ���������
				else
				{
					//�ҵ����ƶ�
					ptr += 4;
					len = (int)(ptr-m_mem.buf);
					m_mem.datasize -= len;
					if(m_mem.datasize) memmove(m_mem.buf,m_mem.buf+len,m_mem.datasize);
				}
			}
		}
		else
		{
			//δ�ҵ���һ���ָ���־���Ϳ���������־����
			cplen = size-copysize;
			if(cplen + (int)m_boundary.length() > m_mem.datasize)
				cplen = m_mem.datasize - (int)m_boundary.length();
			if(cplen<=0)
				break;
			memcpy(buf+copysize,m_mem.buf,cplen);
			copysize += cplen;
			m_mem.datasize-=cplen;
			if(m_mem.datasize) memmove(m_mem.buf,m_mem.buf+cplen,m_mem.datasize);
			if(m_mem.datasize<(int)m_boundary.length())
				break;
		}
	}
	return copysize;
}
string HttpDecodeBoundary::get_boundary_from_head(const char* httpheader)
{
	//get boundary
	string str,boundary;
	cl_httprsp::get_header_field(httpheader,"Content-Type",str);
	int pos = (int)str.find("boundary=");
	if(pos<=0)
		return "";
	boundary = str.substr(pos+9);
	pos = (int)boundary.find(";");
	if(pos>0)
		boundary.erase(pos);
	cl_util::string_trim(boundary);
	return boundary;
}

