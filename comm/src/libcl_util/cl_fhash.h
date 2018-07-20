#pragma once
#include "cl_RDBFile64.h"
#include "cl_string.h"
#include "cl_thread.h"
#include "cl_feList.h"

//************************** ��32λ����ת��Ϊ5�������ַ� ****************************
//����64�������ַ���32λ����ֻȡ��30λת��Ϊ5�������ַ���ÿ���ַ�����6λ���������2λ
void cl_fhash_symbol_check_init();
unsigned int cl_fhash_symbol_to_u32(const char strhash[5]);
void cl_fhash_symbol_to_string(unsigned int n,char outstr[5]);
//===================================================================================

//��ASCII ת��Ϊ16���ƿ����ַ���outs �������2*size
void cl_hx2str(unsigned char digest[], int size, char outs[]);

//���������ļ���SHA1
int cl_fhash_sha1_file(const char* path,char strhash[41]);

//���������ļ���CRC32
int cl_fhash_crc32_file(const char* path, unsigned int& nhash);


//**************************** �ļ��ֿ�hash crc32�б� ****************************
/*
�ļ���hash; 
sha1: 100m Լ1.9��. 1��=86400��= 5T = 500���ļ�.
crc32: 100m Լ0.8��. 1��=86400��= 10T = 1000���ļ�����
*/
//CRC32 �����ļ��ӿ�
int cl_fhash_fileblock(const char* path,int block_size,int index,unsigned int& nhash);

//CRC32 ���������ļ��������ӿ�
//����100��,���ɸ�ʽΪ: size+subsize+a:shash,(a:��ʾc3c32�㷨) subsize������nfactor��������.
int cl_fhash_file_sbhash(const char* path,int nfactor,char strhash[1024]);


//���ļ���CRC32 HASH �л�ȡ��С
long long cl_fhash_getfsize_from_sbhash(const char* strhash);
//�ӳ�HASH����Ϊ��HASH MD5
string cl_fhash_sbhash_to_mainhash(const string& subhash);

//���峤hash�Ͷ�hash���ļ���
bool cl_get_fhash_from_hashfile(const string& path,string& hash,string& subhash);
bool cl_put_fhash_to_hashfile(const string& path, const string& hash, const string& subhash);
bool cl_get_buffer_from_hashfile(const string& path, string& buffer);
//===================================================================================


//********************************** ���������ļ�����MD5 ****************************
//���skipsize����blocksize����md5
//hash = fhsb(blocksize)s(skipsize)f(size)i(blocks).md5
int cl_fhash_sample(const char* path, char hash[128], int blocksize, int skipsize);
//===================================================================================

class cl_fhash_sbhash : public cl_thread
{
private:
	typedef struct tag_block
	{
		char*	buf;
		int		size;
	}block_t;
public:
	cl_fhash_sbhash(const char* filepath, int nfactor); //�ֿ������nfactor��������
	virtual ~cl_fhash_sbhash();

	int get_percentage() const { return (int)((m_read_size+1)*1000/(m_fsize+1)); }
	int get_result(char strhash[1024]) { wait(); strcpy(strhash, m_strhash); return m_result; }
	virtual int work(int e);

private:
	static const int BUFSIZE;
	cl_ERDBFile64	m_file;
	int				m_result; // -1=����ʧ�ܣ�0=����ɹ�
	char			m_strhash[1024];
	char*			m_sbp;
	int				m_nfactor;
	size64_t		m_fsize,m_read_size;
	cl_feList<block_t*>	m_fe;
public:
	unsigned int m_readtick, m_hashtick;
};

//CRC32 ���������ļ��������ӿ� ,���ö��߳���
int cl_fhash_file_sbhash_T(const char* path, int nfactor, char strhash[1024]);
//test:
void cl_fhash_test();

