#pragma once
#include <string.h>

class cl_CharBuffer
{
public:
	cl_CharBuffer(void)
		:m_buffer(new char[1])
		,m_size(0)
	{
		m_buffer[m_size] = '\0';
	}
	cl_CharBuffer(char* buf,int len)
		:m_buffer(0)
		,m_size(0)
	{
		copy(buf,len);
	}
	cl_CharBuffer(char* str)
		:m_buffer(0)
		,m_size(0)
	{
		copy(str,str==NULL?0:(int)strlen(str));
	}
	~cl_CharBuffer(void)
	{
		delete[] m_buffer;
	}
	
public:
	const char* buffer() const {return m_buffer;}
	int size() const {return m_size;}
	void copy(const char* buf,int len)
	{
		delete[] m_buffer;
		if(len<0) len = 0;
		m_size = len;
		m_buffer = new char[m_size+1];
		if(m_size>0)
			memcpy(m_buffer,buf,m_size);
		m_buffer[m_size] = '\0';
	}
	cl_CharBuffer& operator=(const cl_CharBuffer& a)
	{
		copy(a.buffer(),a.size());
		return *this;
	}
	cl_CharBuffer& operator=(const char* str)
	{
		copy(str,str==NULL?0:(int)strlen(str));
		return *this;
	}
	bool operator==(const cl_CharBuffer& a)
	{
		if(a.m_size != m_size)
			return false;
		if(0==m_size || 0==memcmp(m_buffer,a.m_buffer,m_size))
			return true;
		return false;
	}

private:
	char* m_buffer;
	int m_size;
};
