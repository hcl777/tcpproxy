

#include <stdio.h>
#include "cl_util.h"
#include "cl_md5.h"
#include "cl_md5_2.h"
#include "cl_sha1.h"
#include "cl_crc32.h"
#include "cl_RDBFile64.h"

void print_help();
int cl_filesum_sha1(const char* path,bool brdb);
int cl_filesum_md5(const char* path,bool brdb);
int cl_filesum_crc32(const char* path,bool brdb);
int main(int argc,char** argv)
{
	bool bmd5=false;
	bool bsha1=false;
	bool bcrc32=false;
	bool btime=false;
	bool brdb=false;
	if(argc<2 || cl_util::string_array_find(argc,argv,"-h")>0)
	{
		print_help();
		return 0;
	}
	
	if(cl_util::string_array_find(argc,argv,"-md5")>0)
		bmd5 = true;
	if(cl_util::string_array_find(argc,argv,"-sha1")>0)
		bsha1 = true;
	if(cl_util::string_array_find(argc,argv,"-crc32")>0)
		bcrc32 = true;
	if(cl_util::string_array_find(argc,argv,"-t")>0)
		btime = true;
	if(cl_util::string_array_find(argc,argv,"-rdb")>0)
		brdb = true;

	if(bmd5)
	{
		DWORD tick = GetTickCount();
		cl_filesum_md5(argv[1],brdb);
		if(btime)
			printf("second=%.2f \n",_timer_distance(GetTickCount(),tick)/(double)1000);
	}
	if(bsha1)
	{
		DWORD tick = GetTickCount();
		cl_filesum_sha1(argv[1],brdb);
		if(btime)
			printf("second=%.2f \n",_timer_distance(GetTickCount(),tick)/(double)1000);
	}
	if(bcrc32)
	{
		DWORD tick = GetTickCount();
		cl_filesum_crc32(argv[1],brdb);
		if(btime)
			printf("second=%.2f \n",_timer_distance(GetTickCount(),tick)/(double)1000);
	}
	return 0;
}
void print_help()
{
	printf("version: 20161220 \n");
	printf("usage: %s path [-md5/-sha1] [-t] [-rdb]\n",cl_util::get_module_name().c_str());
	printf("-t: print seconds \n");
	printf("-rdb: try check rdb \n");
}

int cl_filesum_sha1(const char* path,bool brdb)
{
	char strhash[41];
	int i=0;
	cl_ERDBFile64 file;
	size64_t size,readSize;
	unsigned char *buf;
	const int BUFSIZE = 64<<10; //16384
	int ftype = brdb?RDBF_AUTO:RDBF_BASE;
	readSize = 0;
	if(0!=file.open(path,F64_READ,ftype))
	{
		printf("*** %s file not found! \n",path);
		return -1;
	}
	size = file.seek(0,SEEK_END);
	file.seek(0,SEEK_SET);
	buf = new unsigned char[BUFSIZE];
	//********************
	SHA1_CONTEXT ctx;
	sha1_init (&ctx);
	while (readSize<size) 
	{
		if(0>=(i=file.read((char*)buf,BUFSIZE)))
		{
			perror("#:read() failed:");
			break;
		}

		sha1_write(&ctx, buf, i);
		readSize+=i;
	}

	sha1_final(&ctx);
	//********************
	delete[] buf;
	file.close();

	for(i=0;i<20;++i)
		sprintf(strhash+2*i,"%02x",ctx.buf[i]);
	strhash[40] = '\0';
	printf("sha1:%s\n",strhash);
	return 0;
}
int cl_filesum_md5(const char* path,bool brdb)
{
	char strhash[41];
	int i=0;
	cl_ERDBFile64 file;
	size64_t size,readSize;
	unsigned char *buf;
	const int BUFSIZE = 64<<10; //16384
	int ftype = brdb?RDBF_AUTO:RDBF_BASE;
	readSize = 0;
	if(0!=file.open(path,F64_READ,ftype))
	{
		printf("*** %s file not found! \n",path);
		return -1;
	}
	size = file.seek(0,SEEK_END);
	file.seek(0,SEEK_SET);
	buf = new unsigned char[BUFSIZE];
	//********************
	//cl_md5 md5
	unsigned char md5buf[16];
	CL_MD5_CTX md5;
	cl_md5_init(&md5);
	while (readSize<size) 
	{
		if(0>=(i=file.read((char*)buf,BUFSIZE)))
		{
			perror("#:read() failed:");
			break;
		}
		cl_md5_update(&md5,buf,i);
		//md5.update(buf, i);
		readSize+=i;
	}

	//md5.finalize();
	 cl_md5_final(&md5,md5buf);
	//********************
	delete[] buf;
	file.close();

	//md5.hexdigest(strhash,33);
	for(i=0;i<16;i++)
		sprintf(strhash+2*i,"%02x",md5buf[i]);
	strhash[32] = '\0';
	printf("md5:%s\n",strhash);
	return 0;
}
int cl_filesum_crc32(const char* path,bool brdb)
{
	int i=0;
	cl_ERDBFile64 file;
	size64_t size,readSize;
	unsigned char *buf;
	const int BUFSIZE = 64<<10; //16384
	int ftype = brdb?RDBF_AUTO:RDBF_BASE;
	readSize = 0;
	if(0!=file.open(path,F64_READ,ftype))
	{
		printf("*** %s file not found! \n",path);
		return -1;
	}
	size = file.seek(0,SEEK_END);
	file.seek(0,SEEK_SET);
	buf = new unsigned char[BUFSIZE];
	//********************
	unsigned int crc32 = CL_CRC32_FIRST;
	while (readSize<size) 
	{
		if(0>=(i=file.read((char*)buf,BUFSIZE)))
		{
			perror("#:read() failed:");
			break;
		}
		crc32 = cl_crc32_write(crc32,buf,i);
		readSize+=i;
	}

	//********************
	delete[] buf;
	file.close();

	printf("crc32:%d\n",crc32);
	return 0;
}
