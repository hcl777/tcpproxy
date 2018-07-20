#pragma once
#include "cl_basetypes.h"
#include "cl_synchro.h"
#include "cl_singleton.h"

#define CLA_MPTHREAD_CORE 1
#define CLA_MPTHREAD_APP 2

//ע��mtu���ܴ���1500
#define CLA_MEMBLOCK_SIZE 1500
#define CLA_MEMPOOL_MIN_SIZE 10000000

class cla_mempool;
class cla_memblock
{
	friend class cla_mempool;
private:
	cla_memblock(int size)
		:bufsize(size)
		,datasize(0)
		,datapos(0)
	{
		buffer = new unsigned char[size];
	}
	~cla_memblock(void)
	{
		delete[] buffer;
	}
public:
	unsigned char *buffer;
	int bufsize;
	int datasize;
	int datapos;
public:
	static cla_memblock* alloc(int threadtoken);
	void free(int threadtoken);
};
//ʹ��3���б�1���߳�1ʹ�ã�1���߳�2ʹ�ã�1�����������������������̱߳��ƽ�⣬���������ִ��ƽ�����ʱʹ����
class cla_mempool
{
public:
	cla_mempool(void);
	~cla_mempool(void);
	typedef cl_CriticalSection Mutex;

public:
	int init();
	void fini();
	cla_memblock* get_block(int threadtoken);
	void put_block(cla_memblock* block,int threadtoken);

private:
	list<cla_memblock*> m_ls[3];
	Mutex m_mt;
	bool m_binit;
	int m_bufsize;
	unsigned int m_block_num;
	unsigned int m_block_min_num;
};

typedef cl_singleton<cla_mempool> cla_mempoolSngl;


