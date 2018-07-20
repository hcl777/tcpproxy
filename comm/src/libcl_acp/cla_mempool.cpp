#include "cla_mempool.h"


//*****************************************************
cla_memblock* cla_memblock::alloc(int threadtoken)
{
	cla_memblock* b = cla_mempoolSngl::instance()->get_block(threadtoken);
	return b;
}
void cla_memblock::free(int threadtoken)
{
	cla_mempoolSngl::instance()->put_block(this,threadtoken);
}
//*****************************************************
cla_mempool::cla_mempool(void)
:m_binit(false)
,m_bufsize(0)
,m_block_num(0)
,m_block_min_num(0)
{
}

cla_mempool::~cla_mempool(void)
{
}

int cla_mempool::init()
{
	if(m_binit)
		return -1;
	m_binit = true;

	m_bufsize = CLA_MEMBLOCK_SIZE;
	m_block_min_num = CLA_MEMPOOL_MIN_SIZE/CLA_MEMBLOCK_SIZE;

	//初始化时全部放入平衡队列
	for(unsigned int j=0;j<m_block_min_num;j++)
		m_ls[0].push_back(new cla_memblock(m_bufsize));
	m_block_num += m_ls[0].size();
	return 0;
}
void cla_mempool::fini()
{
	cla_memblock *_block;
	assert(m_block_num == m_ls[0].size() + m_ls[1].size() + m_ls[2].size());
	if(!m_binit)
		return;
	m_binit = false;
	
	for(int i=0;i<3;++i)
	{
		while((_block=m_ls[i].front()))
		{
			m_ls[i].pop_front();
			delete _block;
		}
	}
	m_block_num = 0;
}
cla_memblock* cla_mempool::get_block(int threadtoken)
{
	cla_memblock *_block;
	//0列为公共
	assert(1==threadtoken || 2==threadtoken);
	if(m_ls[threadtoken].empty())
	{
		cl_TLock<Mutex> l(m_mt);
		int n=0;
		while((_block=m_ls[0].front()))
		{
			m_ls[0].pop_front();
			m_ls[threadtoken].push_back(_block);
			n++;
			if(n>500) //每次取500,避免占用过长时间
				break;
		}
		if(n<500)
		{
			CLLOG1("# cla_mempool new block(%d) - n=%d !! \n",m_bufsize,1000-n);
			for(int i=n;i<500;++i)
			{
				m_ls[threadtoken].push_back(new cla_memblock(m_bufsize));
				m_block_num++;
			}
		}
	}
	_block = m_ls[threadtoken].front();
	m_ls[threadtoken].pop_front();
	return _block;
}
void cla_mempool::put_block(cla_memblock* block,int threadtoken)
{
	cla_memblock *_block;
	//0列为公共
	assert(1==threadtoken || 2==threadtoken);
	block->datasize = 0;
	block->datapos = 0;
	m_ls[threadtoken].push_back(block);
	if(m_ls[threadtoken].size()>(m_block_min_num/3 + 1000))
	{
		cl_TLock<Mutex> l(m_mt);
		for(int i=0;i<500;++i)
		{
			_block = m_ls[threadtoken].front();
			m_ls[threadtoken].pop_front();
			if(m_ls[0].size()<m_block_min_num+2000)
				m_ls[0].push_back(_block);
			else
			{
				delete _block;
				m_block_num--;
				CLLOG1("# cla_mempool free block(%d) !! \n",m_bufsize);
			}
		}
	}
}

