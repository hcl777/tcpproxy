#pragma once
#include "cl_cyclist.h"
#include "cl_timer.h"
#include "cl_singleton.h"
#include "cl_synchro.h"
/*************************************
˵��:
���ڴ��ʵ��n��̶���С�ڴ���Ԥ�������.
ÿ��Ŀ��С��ͬ.��ͬ����С����ͬ.
���ڴ�ز���������.ֻ�ʺϵ��߳�ʹ��.
һ��cl_memblockQueueΪһ����ͬ��С��cl_memblock..
һ��cl_memblockPool������cl_memblockQueue.
cl_memblockQueue�е��ڴ������ڽ���ɾ��,ÿ20��ִ��һ��.
**************************************/

#define CL_MBLOCK_NEW_RETURN_INT(name,size,ret) cl_memblock *name = cl_memblock::allot(size); if(NULL==name) {return ret;} 

class cl_memblock
{
	friend class cl_memblockQueue;
	friend class cl_memblockPool;
public:
	char *buf;
	//int buflen,datapos,datalen;
	int buflen,rpos,wpos;
private:
	int i;
	int ref;
private:
	cl_memblock(int i,int size)
		:buf(new char[size])
		,buflen(size)
		,rpos(0)
		,wpos(0)
		,i(i)
		,ref(0)//��ʼ0
	{
	}
	~cl_memblock(void)
	{
		delete[] buf;
	}

public:
	static cl_memblock* allot(int size);
	void refer(){ref++;}
	void release();
	int length() const { return wpos-rpos; }
	char* read_ptr() const { return buf+rpos; }
	int write(const char* b,int len,int offset)
	{
		if(offset>wpos)
			return 0;
		int tmp = wpos - offset; //ȥ���ص�����
		if(len<=tmp)
			return 0;
		len -= tmp;
		b += tmp;
		assert(wpos+len<=buflen);
		if(wpos+len>buflen) len = buflen-wpos;
		memcpy(buf+wpos,b,len);
		wpos += len;
		return len;
	}
};

class cl_memblockQueue
{
public:
	cl_memblockQueue(int i,int blocksize);
	~cl_memblockQueue(void);

	typedef list<cl_memblock*> BlockList;
	typedef BlockList::iterator BlockIter;
public:
	void put(cl_memblock* b);
	cl_memblock* get();

	void reduce(); //�ü�,���ڼ�
	int get_blocksize() const { return m_blocksize;}
private:
	BlockList m_ls;
	int m_i;
	int m_blocksize;	//block ��С
	int m_outs;			//�Ѿ�����ȥ��.
	int m_maxouts;		//������������������
};

class cl_memblockPool : public cl_timerHandler
{
	typedef cl_SimpleMutex Mutex;
	typedef cl_TLock<Mutex> Lock;
public:
	cl_memblockPool();
	~cl_memblockPool(void);
public:
	int init(int blocksize[],int n);
	void fini();

	cl_memblock* get_block(int size);
	void put_block(cl_memblock* b);

	virtual void on_timer(int e);
private:
	Mutex	m_mt;
	cl_memblockQueue **m_queue;
	int m_queue_num;
	int m_outs;
};

typedef cl_singleton<cl_memblockPool> cl_memblockPoolSngl;


