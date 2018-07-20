
#include "cl_RDBEifFile64.h"
#include "cl_util.h"
#include "cl_sha1.h"
#include "cl_fhash.h"

void show_help();
int string_array_find(int argc,char** argv,const char* str);
int rdb_sha1(const char* path);
int rdb_sha3(const char* path, bool bthread);
int rdb_un_encrypt(const string& from_path,const string& to_path);
int rdb_encrypt_dir(const string& dir);
int rdb_encrypt(const string& from_path,const string& to_path);
int rdb_show_info(const char* path);
int rdbs_un_encrypt(const string& path,const string& to_path);
int rdbs_encrypt(const string& path,const string& to_path);
int rdbeif_show_info(const char* path);
int rdbeif_add_file_dir(const string& dir,bool bcover);
int rdbeif_encrypt_and_add_file_by_txt(const string& txtpath,const string& outdir,int filetype,bool bcover);
int rdbeif_add_file(const char* cvi_path,const char* src_paths,bool bcover);
int rdbeif_release_all_file(const char* cvi_path,const char* dir);
int rdbeif_release_file_by_name(const char* cvi_path,const char* eifname,const char* eifdir);
int rdbeif_uneifdir(const string& dir);
int rdbeif_uneiftxt_dir(const string& txtpath);
int rdbeif_eifclean(const char* cvi_path);
int rdbeif_eifclean_by_txt(const string& txtpath);

int testread(const char* path);

void crc32_folder(const char* dir);


int main(int argc,char** argv)
{
	//printf("civtool start...\n");
	cl_util::debug_memleak();
	if(/*argc<2 || */(argc>1 && (0==strcmp(argv[1],"-h")||0==strcmp(argv[1],"--help"))))
	{
		show_help();
		printf("---end.\n");
		return 0;
	}

	bool bcover = false;
	int i = 0;
	if(-1!=(i=string_array_find(argc,argv,"--cover")))
	{
		bcover = true;
	}
	
	if (-1 != (i = string_array_find(argc, argv, "--fhs")))
	{
		if (i + 1 < argc)
		{
			char hash[1024];
			int blocksize = 0;
			int skipsize = -1;
			if (i + 2 < argc) blocksize = atoi(argv[i + 2]);
			if (i + 3 < argc) skipsize = atoi(argv[i + 3]);
			if (blocksize <= 0) blocksize = 2048;
			if (skipsize < 0) skipsize = 62 << 10;

			DWORD tick = GetTickCount();
			if (0 == cl_fhash_sample(argv[i + 1], hash, blocksize, skipsize))
			{
				printf("hash: %s\n", hash);
			}
			int sec = (int)(GetTickCount() - tick + 500) / 1000;
			printf("secs: %d\n", sec);
		}
	}

	if(-1!=(i=string_array_find(argc,argv,"-i")))
	{
		if(i+1<argc)
		{
			// -i cvi-path
			rdb_show_info(argv[i+1]);
		}
	}

	if (-1 != (i = string_array_find(argc, argv, "-ie")))
	{
		if (i + 1<argc)
		{
			// -ei cvi-path
			rdbeif_show_info(argv[i + 1]);
		}
	}
	
	if(-1!=(i=string_array_find(argc,argv,"--sha1")))
	{
		if(i+1<argc)
		{
			// -sha1 cvi-path
			rdb_sha1(argv[i+1]);
		}
	}
	if(-1!=(i=string_array_find(argc,argv,"--sha3")))
	{
		if(i+1<argc)
		{
			// -sha3 cvi-path
			bool bthread = false;
			if (i + 2 < argc && 0 == strcmp(argv[i + 2], "1"))
				bthread = true;
			rdb_sha3(argv[i+1], bthread);
		}
	}

	if (-1 != (i = string_array_find(argc, argv, "--crc32_folder")))
	{
		if (i + 1<argc)
		{
			// 
			crc32_folder(argv[i + 1]);
		}
	}
	if(-1!=(i=string_array_find(argc,argv,"--cvidir")))
	{
		if(i+1<argc)
		{
			rdb_encrypt_dir(argv[i+1]);
		}
	}
	if(-1!=(i=string_array_find(argc,argv,"--cvi")))
	{
		if(i+2<argc)
		{
			rdb_encrypt(argv[i+1],argv[i+2]);
		}
	}
	if(-1!=(i=string_array_find(argc,argv,"--uncvi")))
	{
		if(i+2<argc)
		{
			rdb_un_encrypt(argv[i+1],argv[i+2]);
		}
	}
	
	if(-1!=(i=string_array_find(argc,argv,"--scvi")))
	{
		if(i+2<argc)
		{
			rdbs_encrypt(argv[i+1],argv[i+2]);
		}
	}
	if(-1!=(i=string_array_find(argc,argv,"--unscvi")))
	{
		if(i+2<argc)
		{
			rdbs_un_encrypt(argv[i+1],argv[i+2]);
		}
	}
	if(-1!=(i=string_array_find(argc,argv,"--eifdir")))
	{
		if(i+1<argc)
		{
			rdbeif_add_file_dir(argv[i+1],bcover);
		}
	}
	if(-1!=(i=string_array_find(argc,argv,"--eiftxt")))
	{
		if(i+3<argc)
		{
			rdbeif_encrypt_and_add_file_by_txt(argv[i+1],argv[i+2],atoi(argv[i+3]),bcover);
		}
	}
	if(-1!=(i=string_array_find(argc,argv,"--eif")))
	{
		if(i+2<argc)
		{
			rdbeif_add_file(argv[i+1],argv[i+2],bcover);
		}
	}
	if(-1!=(i=string_array_find(argc,argv,"--uneif")))
	{
		if(i+2<argc)
		{
			rdbeif_release_all_file(argv[i+1],argv[i+2]);
		}
	}
	if(-1!=(i=string_array_find(argc,argv,"--uneifi")))
	{
		if(i+3<argc)
		{
			rdbeif_release_file_by_name(argv[i+1],argv[i+2],argv[i+3]);
		}
	}
	if(-1!=(i=string_array_find(argc,argv,"--uneifdir")))
	{
		if(i+1<argc)
		{
			rdbeif_uneifdir(argv[i+1]);
		}
	}
	if(-1!=(i=string_array_find(argc,argv,"--uneiftxt_dir")))
	{
		if(i+1<argc)
		{
			rdbeif_uneiftxt_dir(argv[i+1]);
		}
	}
	
	if(-1!=(i=string_array_find(argc,argv,"--eifclean")))
	{
		if(i+1<argc)
		{
			rdbeif_eifclean(argv[i+1]);
		}
	}
	if(-1!=(i=string_array_find(argc,argv,"--eifcleantxt")))
	{
		if(i+1<argc)
		{
			rdbeif_eifclean_by_txt(argv[i+1]);
		}
	}
	
	//testread
	if(-1!=(i=string_array_find(argc,argv,"--testread")))
	{
		if(i+1<argc)
		{
			testread(argv[i+1]);
		}
	}

	if(1==argc)
	{
		printf("#------ try ./uneiftxt_dir.txt \n");
		rdbeif_uneiftxt_dir("./uneiftxt_dir.txt");
	}

//#ifdef _WIN32
//	printf("\nenter any key to exit...");
//	getchar();
//#endif
	printf("---end.\n");
	return 0;
}

int string_array_find(int argc,char** argv,const char* str)
{
	for(int i=0;i<argc;++i)
	{
		if(0==strcmp(argv[i],str))
			return i;
	}
	return -1;
}

void show_help()
{
	printf("cvitool version: 1.06(20180629) \n");
	printf("cvitool: cnvalid option: -h or --help \n");
	printf("usage: cvitool [-i xx][-ie xx][--fhs xx][--sha1 xx] [--sha3 xx][--crc32_folder xx] [--cvidir xx?ex=.ic2&delsrc=1][--cvi xx xx]\n"
		"[--uncvi xx xx][--eifdir xx][--eif xx xx][--uneif xx xx][--uneifi xx xx xx] \n\n");
	printf("%-10s :show cvi-file information. (cvi-path) \n", "-i");
	printf("%-10s :show cvi-file eif information. (cvi-path) \n","-ie");
	printf("%-10s :sample hash-md5(filepath [blocksize] [skipsize]) \n", "--fhs");
	printf("%-10s :count cvi-file sha1 string. (cvi-path) \n","--sha1");
	printf("%-10s :count cvi-file sha3 string. (cvi-path) \n","--sha3");
	printf("%-10s :count crc32 folder \n", "--crc32_folder");
	printf("%-10s :encrypt normal-file to cvi-file. (normal-path,cvi-path) \n","--cvi");
	printf("%-10s :unencrypt cvi-file to normal-file.(cvi-path,normal-path)  \n","--uncvi");
	printf("%-10s :add an extended-information-file into cvi-file.(cvi-path,eif-path)\n","--eif");
	printf("%-10s :release all extended-information-file.(cvi-path,eif-dir)  \n","--uneif");
	printf("%-10s :release extended-information-file by name.(cvi-path,eif-name,eif-dir)  \n","--uneifi");
	printf("%-10s :clean all eif.(cvi-path)  \n","--eifclean");
	printf("%-10s :clean all eif by txtpath(file list).(txtpath)  \n","--eifcleantxt");
	printf("\n");
#ifdef _WIN32
	getchar();
#endif
	
}
int rdb_sha1(const char* path)
{
	char strHash[48];
	int ret = -1;
	DWORD begin_tick = GetTickCount();
	if(0==Sha1_BuildFile(path,strHash,NULL,0,false))
	{
		printf("%s",strHash);
		ret = 0;
	}

	int sec = (int)(GetTickCount() - begin_tick) / 1000;
	size64_t size = cl_ERDBFile64::get_filesize(path);
	if (sec > 0)
	{
		printf("\n(sec=%d S ; size=%lld B; speed= %d MB/s;)\n",sec, size, (int)((size / sec) >> 20));
	}
	return ret;
}
#define CLY_FBLOCK_SIZE 102400
int rdb_sha3(const char* spath,bool bthread)
{
	string path = spath;
	string hash,subhash;
	char buf[2048];
	int ret = 0;

	DWORD begin_tick = GetTickCount();
	if (!bthread)
		ret = cl_fhash_file_sbhash(path.c_str(), CLY_FBLOCK_SIZE, buf);
	else
		ret = cl_fhash_file_sbhash_T(path.c_str(), CLY_FBLOCK_SIZE, buf);
	if(0==ret)
	{
		subhash = buf;
		hash = cl_fhash_sbhash_to_mainhash(subhash);
		cl_put_fhash_to_hashfile(path+".sha3",hash,subhash);
		printf("hash   :	%s\nsubhash:	%s\n", hash.c_str(), subhash.c_str());
	}
	printf("ret=%d\n", ret);

	int sec = (int)(GetTickCount() - begin_tick) / 1000;
	size64_t size = cl_ERDBFile64::get_filesize(spath);
	if (sec > 0)
	{
		printf("\n(sec=%d S ; size=%lld B; speed= %d MB/s;)\n", sec, size, (int)((size / sec) >> 20));
	}
	return 0;
}
int ic2_un_encrypt(const string& from_path,const string& to_path)
{
	cl_util::create_directory_by_filepath(to_path);
	
	cl_ERDBFile64 from_file;
	cl_file64 to_file;
	string to_path_tmp;
	size64_t size,write_size;
	int len;
	char *buf = new char[10240];

	to_path_tmp = to_path;
	to_path_tmp += ".tmp!";
	unlink(to_path_tmp.c_str());
	from_file.open(from_path.c_str(),F64_READ);
	to_file.open(to_path_tmp.c_str(),F64_RDWR|F64_TRUN);
	if(!from_file.is_open() || !to_file.is_open())
		return 0;

	write_size = 0;
	size = from_file.get_file_size();
	while(write_size<size)
	{
		len = from_file.read(buf,10240);
		if(len<=0)
			break;
		if(len!=to_file.write(buf,len))
			break;
		write_size += len;
	}

	delete[] buf;
	from_file.close();
	to_file.close();
	assert(write_size==size);
	if(write_size==size)
	{
		if(0==rename(to_path_tmp.c_str(),to_path.c_str()))
		{
			printf("# RDB-file to normal-file susseed! [%s] \n",to_path.c_str());
			return 0;
		}
	}
	return -1;
}
int rdb_un_encrypt(const string& from_path,const string& to_path)
{
	int ftype = cl_ERDBFile64::get_filetype(from_path.c_str());
	if(RDBF_BASE == ftype)
	{
		printf("#***is normal file---%s\n",from_path.c_str());
		return 2;
	}
	if(cl_ERDBFile64::get_filesize(to_path.c_str())>0)
	{
		printf("#*** normal file exist: %s \n",to_path.c_str());
		return 1;
	}
	if(RDBF_RDBS==ftype)
		return rdbs_un_encrypt(from_path,to_path);
	return ic2_un_encrypt(from_path,to_path);
	
}
int rdb_encrypt_dir(const string& cgi)
{
	list<string> ls_path;
	list<int> ls_ino;
	int size=0;
	int n=0;
	string from_path,to_path;
	string dir = cl_util::url_cgi_get_path(cgi);
	cl_util::get_folder_files(dir,ls_path,ls_ino,"",0,false);
	size = ls_path.size();
	string filedir,name,ext;
	bool bdelsrc = false;
	string exname = cl_util::url_get_parameter(cgi,"ex");
	if(exname.empty())
		exname = ".cvi";
	if(exname.at(0) != '.')
	{
		string exname2=".";
		exname2 += exname;
		exname = exname2;
	}
	if(cl_util::url_get_parameter(cgi,"delsrc")=="1")
		bdelsrc = true;

	for(list<string>::iterator it=ls_path.begin();it!=ls_path.end();++it)
	{
		from_path = *it;
		//to_path = from_path + "-.cvi";
		cl_util::filepath_split(from_path,filedir,name,ext);
		to_path = filedir+name + exname;
		if(0==rdb_encrypt(from_path,to_path))
		{
			if(bdelsrc)
				cl_util::file_delete(from_path);
		}
		n++;
		printf("# cvidir(%d)... %.2f%% \n",size,n*100/(double)size);
	}

	printf("# cvidir(%d)... 100%% \n",size);
	return 0;
}

int ic2_encrypt(const string& from_path,const string& to_path)
{
	cl_util::create_directory_by_filepath(to_path);

	string to_path_tmp;
	size64_t block_size = RDB_BLOCK_SIZE;
	size64_t from_size=0,to_size=0;
	list<int> from_index_ls,tmp_ls;
	list<int>::iterator it;
	cl_ERDBFile64 from_file,to_file;

	from_size = cl_ERDBFile64::get_filesize(from_path.c_str());
	if(from_size<=0)
		return -1;
	int blocks = (int)((from_size-1)/block_size + 1);

	int i=0;
	for(i=0;i<blocks;++i)
		tmp_ls.push_back(i);

	//乱序：
	while(!tmp_ls.empty())
	{
		srand((unsigned int)time(NULL));
		int n = rand()%tmp_ls.size();
		for(it=tmp_ls.begin(),i=0;it!=tmp_ls.end()&&i<n;++it,++i)
		{
		}
		assert(i==n);
		from_index_ls.push_back(*it);
		tmp_ls.erase(it);
	}
	//printf("# blocks : %d,%d (bs=%lld) \n",blocks,from_index_ls.size(),block_size);
	
	to_path_tmp = to_path;
	to_path_tmp += ".tmp!";
	unlink(to_path_tmp.c_str());
	from_file.open(from_path.c_str(),F64_READ/*,RDBF_AUTO*/);
	to_file.open(to_path_tmp.c_str(),F64_RDWR|F64_TRUN);
	if(!from_file.is_open() || !to_file.is_open())
		return -1;

	size64_t pos;
	char *buf = new char[300000];
	int len=0,read_len;
	int real_block_size=0;
	int index = 0;
	for(it=from_index_ls.begin(),i=0;it!=from_index_ls.end();++it,++i)
	{
		printf("\r %.2f",i*100/(double)from_index_ls.size());

		index = *it;
		if(index != blocks-1)
			real_block_size = (int)block_size;
		else
			real_block_size = (int)(from_size - block_size * index);//(int)(from_size%block_size); 刚好整块的时候会错误
		//printf(" %8d: %d (bs:%d) \n",i,index,real_block_size);
		pos = index * block_size;
		while(real_block_size>0)
		{
			len = real_block_size>300000?300000:real_block_size;
			from_file.seek(pos,SEEK_SET);
			read_len = from_file.read(buf,len);
			assert(read_len==len);
			if(read_len<=0)
			{
				printf("read file wrong!\n");
				break;
			}
			to_file.seek(pos,SEEK_SET);
			len = to_file.write(buf,read_len);
			assert(len == read_len);
			if(len!=read_len)
			{
				printf("#*** write wrong: len=%d,read_len=%d,pos=%lld,from_size=%lld,index=%d,blocks=%d \n",
					len,read_len,pos,from_size,index,blocks);
				break;
			}
			pos += read_len;
			to_size += read_len;
			real_block_size -= read_len;

		}

	}
	delete[] buf;
	assert(to_size == from_size);
	from_file.close();
	to_file.close();
	if(to_size==from_size)
	{
		if(0==rename(to_path_tmp.c_str(),to_path.c_str()))
		{
			printf("# normal-file to RDB-file susseed! [%s] \n",to_path.c_str());
			return 0;
		}
		else
		{
			printf("# *** renme fail (%s => %s) \n",to_path_tmp.c_str(),to_path.c_str());
		}
	}
	else
	{
		printf("# *** wrong write size : (%lld / %lld ) \n",to_size,from_size);
	}
	return -1;
}
int rdb_encrypt(const string& from_path,const string& to_path)
{
	if(cl_ERDBFile64::get_filesize(to_path.c_str())>0)
	{
		printf("#*** RDB file exist: %s \n",to_path.c_str());
		return 1;
	}
	if(RDBF_BASE != cl_ERDBFile64::get_filetype(from_path.c_str()))
	{
		if(0==cl_util::file_rename(from_path,to_path))
		{
			printf("#*is RDB file,move ok!---%s\n",from_path.c_str());
			return 2;
		}
		else
		{
			printf("#***is RDB file,move file fail!***---%s\n",from_path.c_str());
			return -1;
		}
	}
	return ic2_encrypt(from_path,to_path);
}
int rdb_show_info(const char* path)
{
	cl_RDBFile64::CheckInfo_t inf;
	int ret = cl_RDBFile64::get_checkinfo(path,inf);
	printf("ret = %d \n", ret);
	if (0 == ret)
	{
		printf("head_size = %d ; block_size = %d ; \n"
			"block_num = %d ; max_index=%d ; last_index=%d \n\n",
			inf.head_size, inf.block_size, inf.block_num,inf.max_index,inf.last_index);

		printf("seekend_endpos		= %lld \n",inf.seekend_endpos );
		printf("calculate_endpos	= %lld \n\n", inf.calculate_endpos);

		printf("record_file_size	= %lld \n", inf.record_file_size);
		printf("calculate_file_size	= %lld \n\n", inf.calculate_file_size);

		printf("record_write_size    = %lld \n", inf.record_write_size);
		printf("calculate_write_size = %lld \n\n", inf.calculate_write_size);
	}
	return 0;
}
int rdbs_un_encrypt(const string& path,const string& to_path)
{
	int ret = cl_RDBFile64Simple::rdbs_to_normal_file(path.c_str(),to_path.c_str());
	if(0==ret)
		printf("# rdbs un_encrypt ok (%s) \n",path.c_str());
	else if(1==ret)
		printf("# +++ already normal (%s) \n",path.c_str());
	else
		printf("# **** rdbs un_encrypt fail (%s) \n",path.c_str());
	return 0;
}
int rdbs_encrypt(const string& from_path,const string& to_path)
{
	if(cl_ERDBFile64::get_filesize(to_path.c_str())>0)
	{
		printf("#*** RDBs file exist: %s \n",to_path.c_str());
		return 1;
	}
	if(RDBF_BASE != cl_ERDBFile64::get_filetype(from_path.c_str()))
	{
		if(0==cl_util::file_rename(from_path,to_path))
		{
			printf("#*is RDBS file,move ok!---%s\n",from_path.c_str());
			return 2;
		}
		else
		{
			printf("#***is RDB file,move file fail!***---%s\n",from_path.c_str());
			return -1;
		}
	}

	int ret = cl_RDBFile64Simple::normal_to_rdbs_file(from_path.c_str(),to_path.c_str());
	if(0==ret)
		printf("# rdbs encrypt ok (%s) \n",from_path.c_str());
	else if(1==ret)
		printf("# +++ already encrypt (%s) \n",from_path.c_str());
	else
		printf("# **** rdbs encrypt fail (%s) \n",from_path.c_str());
	return 0;
}
//int rdbeif_show_info(const char* path)
//{
//	RDBEIFInfo_t ri;
//	memset(&ri,0,sizeof(ri));
//	ri.try_get_file_num = 10;
//	ri.files = new RDBEIFNode_t[10];
//	cl_ERDBFile64 file;
//	if(0!=file.open(path,F64_READ,RDBF_RDB))
//	{
//		printf("#*** open RDB file fail! [%s] \n",path);
//		return -1;
//	}
//	if(0==file.eif_get_zip_info(ri))
//	{
//		printf("#----------------------------------------------------------#\n");
//		printf("# RDF EIF info:\n");
//		printf("# [RDB file]:: size:%lld    block_size:%d  \n",ri.rdb_file_size,ri.rdb_block_size);
//		printf("# [EIF]:: filenum:  %d  shownum:  %d \n",ri.all_file_num,ri.real_get_file_num);
//		printf("# %-5s : %-20s   %-5s    %s \n","[num]","[size]","[id]","[name]");
//		for(int i=0;i<ri.real_get_file_num;++i)
//		{
//			printf("# %-5d : %-20lld   %-5d    %s \n",i+1,ri.files[i].file_size,ri.files[i].id,ri.files[i].name);
//		}
//		printf("#----------------------------------------------------------#\n");
//	}
//	file.close();
//	delete[] ri.files;
//	return 0;
//}
int rdbeif_show_info(const char* path)
{
	rdbeif_handle_t* h = rdbeif_open(path,F64_READ);
	rdbeif_node_t* node;
	if(NULL==h)
	{
		printf("#*** open RDBEIF file fail! [%s] \n",path);
		return -1;
	}
	
	printf("#----------------------------------------------------------#\n");
	printf("# RDFEIF info:\n");
	printf("# [FILE TYPE]:: %d  \n",h->file_type);
	printf("# [EIF]:: filenum:  %d  \n",h->nl.size());
	printf("# %-5s : %-20s   %-5s    %s \n","[num]","[size]","[id]","[name]");
	int i=1;
	for(cl_cyclist<rdbeif_node_t*>::iterator it=h->nl.begin();it!=h->nl.end();++it)
	{
		node = *it;
		printf("# %-5d : %-20lld   %-5d    %s \n",i++,node->file_size,node->id,node->name);
	}
	printf("#----------------------------------------------------------#\n");

	rdbeif_close(h);
	return 0;
}
//int rdbeif_add_file_list(const char* cvi_path,list<string>& ls,bool bcover)
//{
//	string path;
//	int id,ret;
//	cl_ERDBFile64 file;
//
//	if(0!=file.open(cvi_path,F64_RDWR,RDBF_RDB))
//	{
//		printf("#*** open RDB fail (%s) \n",cvi_path);
//		return -1;
//	}
//	for(list<string>::iterator it=ls.begin();it!=ls.end();++it)
//	{
//		string& str2 = *it;
//		path = cl_util::get_string_index(str2,0,"::");
//		id = atoi(cl_util::get_string_index(str2,1,"::").c_str());
//		//printf(" add eiffile : %s ...... \n",path.c_str());
//		ret = file.eif_zip_file(path.c_str(),id);
//		if(0==ret)
//		{
//			printf("# RDB add eif-file succeed:(%s) \n",str2.c_str());
//		}
//		else if(1==ret)
//		{
//			printf("#*** RDB eif-file is exist:(%s) \n",str2.c_str());
//		}
//		else
//		{
//			printf("#*** RDB add eif-file fail:(%s) \n",str2.c_str());
//		}
//	}
//	file.close();
//	return 0;
//}
int rdbeif_add_file_list(const char* cvi_path,list<string>& ls,bool bcover)
{
	string path;
	int id,ret;
	rdbeif_handle_t* h = rdbeif_open(cvi_path,F64_RDWR);

	if(NULL==h)
	{
		printf("#*** open RDB fail (%s) \n",cvi_path);
		return -1;
	}
	for(list<string>::iterator it=ls.begin();it!=ls.end();++it)
	{
		string& str2 = *it;
		path = cl_util::get_string_index(str2,0,"::");
		id = atoi(cl_util::get_string_index(str2,1,"::").c_str());
		//printf(" add eiffile : %s ...... \n",path.c_str());
		ret = rdbeif_zip_file(h,path.c_str(),id,bcover);
		if(0==ret)
		{
			printf("# RDB add eif-file succeed:(%s) \n",str2.c_str());
		}
		else if(1==ret)
		{
			printf("#*** RDB eif-file is exist:(%s) \n",str2.c_str());
		}
		else
		{
			printf("#*** RDB add eif-file fail:(%s) \n",str2.c_str());
		}
	}
	rdbeif_close(h);
	return 0;
}
int rdbeif_add_file_dir(const string& dir,bool bcover)
{
	list<string> ls_path;
	list<int> ls_ino;
	int size=0;
	int n=0;
	int pos;
	string from_path,to_path;
	cl_util::get_folder_files(dir,ls_path,ls_ino,"cvi",0,false);
	size = ls_path.size();

	for(list<string>::iterator it=ls_path.begin();it!=ls_path.end();++it)
	{
		n++;
		from_path = *it;
		pos = (int)from_path.rfind('.');
		if(pos<=0)
			continue;
		to_path = from_path.substr(0,pos);

		list<string> ls_sub_path;
		list<int> ls_sub_ino;
		cl_util::get_folder_files(to_path,ls_sub_path,ls_sub_ino,"",0,false);
		if(!ls_sub_path.empty())
		{
			printf("eifdir...-> %s \n",from_path.c_str());
			rdbeif_add_file_list(from_path.c_str(),ls_sub_path,bcover);
		}
		
		printf("# eifdir(%d)... %.2f%% \n",size,n*100/(double)size);
	}

	printf("# eifdir(%d)... 100%% \n",size);
	return 0;
}
int rdbeif_encrypt_and_add_file_by_txt(const string& txtpath,const string& aoutdir,int filetype,bool bcover)
{
	list<string> ls, ls_path;
	int size=0;
	int n=0,i=0,num=0,ret;
	string from_path,to_path;
	if(txtpath.empty() || aoutdir.empty())
	{
		printf("# Please specify the path - txtpath and outdir !!!\n");
		return 0;
	}

	cl_util::get_stringlist_from_file(txtpath,ls);
	size = ls.size();
	string outdir = aoutdir;
	char c = outdir.at(outdir.length()-1);
	if(c!='/' && c!='\\')
		outdir += "/";

	for(list<string>::iterator it=ls.begin();it!=ls.end();++it)
	{
		n++;
		i = 0;
		string& str = *it;
		num = cl_util::get_string_index_count(str,"|");
		from_path =  cl_util::get_string_index(str,0,"|");
		if(from_path.empty())
			continue;
		if(RDBF_RDB==filetype)
			to_path = outdir + cl_util::get_filename_prename(from_path) + ".cvi";
		else
			to_path = outdir + cl_util::get_filename_prename(from_path) + ".scvi";
		ls_path.clear();
		for(i=1;i<num;++i)
		{
			ls_path.push_back(cl_util::get_string_index(str,i,"|"));
		}
		printf("eiftxt...-> %s \n",from_path.c_str());
		if(RDBF_RDB==filetype)
			ret = rdb_encrypt(from_path,to_path);
		else
			ret = rdbs_encrypt(from_path,to_path);
		if(-1!=ret)
		{
			rdbeif_add_file_list(to_path.c_str(),ls_path,bcover);
		}
		printf("# eiftxt(%d)... %.2f%% \n",size,n*100/(double)size);

		///////////////////////////////////
	}

	printf("# eiftxt(%d)... 100%% \n",size);
	return 0;
}
int rdbeif_add_file(const char* cvi_path,const char* src_paths,bool bcover)
{
	string str,str2;
	int n;
	list<string> ls;
	str = src_paths;
	//printf(" add eiffile to cvifile : %s <= %s ...... \n",cvi_path,src_paths);
	n = cl_util::get_string_index_count(str,"??");
	for(int i=0;i<n;++i)
	{
		ls.push_back(cl_util::get_string_index(str,i,"??"));
	}
	return rdbeif_add_file_list(cvi_path,ls,bcover);
}
//int rdbeif_release_all_file(const char* cvi_path,const char* dir)
//{
//	cl_ERDBFile64 file;
//	if(0!=file.open(cvi_path,F64_READ,RDBF_RDB))
//	{
//		printf("#*** open RDB file fail! [%s] \n",cvi_path);
//		return -1;
//	}
//	cl_util::MyCreateDirectory(dir);
//	file.eif_unzip_all_file(dir);
//	file.close();
//	return 0;
//}
int rdbeif_release_all_file(const char* cvi_path,const char* dir)
{
	rdbeif_handle_t *h = rdbeif_open(cvi_path,F64_READ);
	if(NULL==h)
	{
		printf("#*** open RDB file fail! [%s] \n",cvi_path);
		return -1;
	}
	if(!h->nl.empty())
	{
		cl_util::my_create_directory(dir);
		rdbeif_unzip_all(h,dir);
	}
	rdbeif_close(h);
	return 0;
}

int rdbeif_release_file_by_name(const char* cvi_path,const char* eifname,const char* eifdir)
{
	rdbeif_node_t* h;
	if(NULL==cvi_path||NULL==eifname||NULL==eifdir)
	{
		return -1;
	}
	h = rdbeif_node_open(cvi_path,eifname);
	if(NULL==h)
	{
		printf("#*** open RDB file fail! [%s] \n",cvi_path);
		return -1;
	}
	cl_util::my_create_directory(eifdir);
	char path[256];
	strcpy(path,eifdir);
	size_t n = strlen(path);
	if(path[n-1]!='/' && path[n-1]!='\\')
	{
		path[n] = '/';
		path[++n] = '\0';
	}
	strcat(path,eifname);
	cl_file64 tofile;
	if(0==tofile.open(path,F64_TRUN|F64_RDWR))
	{
		char buf[10240];
		int ret;
		while(!rdbeif_node_eof(h))
		{
			ret = rdbeif_node_read(h,buf,10240);
			if(ret<=0)
				break;
			tofile.write(buf,ret);
		}

		tofile.close();
	}
	
	rdbeif_node_close(h);
	return 0;
}
int rdbeif_uneifdir(const string& dir)
{
	list<string> ls_path;
	list<int> ls_ino;
	int size=0;
	int n=0;
	string from_path,to_path;
	cl_util::get_folder_files(dir,ls_path,ls_ino,"",0,false);
	size = ls_path.size();
	string filedir,name,ext;

	for(list<string>::iterator it=ls_path.begin();it!=ls_path.end();++it)
	{
		from_path = *it;
		//to_path = from_path + "-.cvi";
		cl_util::filepath_split(from_path,filedir,name,ext);
		to_path = filedir+name;
		rdbeif_release_all_file(from_path.c_str(),to_path.c_str());
		n++;
		printf("# uneifdir(%d)... %.2f%% \n",size,n*100/(double)size);
	}

	printf("# uneifdir(%d)... 100%% \n",size);
	return 0;
}

int rdbeif_uneiftxt_dir(const string& txtpath)
{
	list<string> ls;
	int size=0;
	int n=0;
	if(txtpath.empty())
	{
		printf("# Please specify the path - txtpath!!!\n");
		return 0;
	}

	cl_util::get_stringlist_from_file(txtpath,ls);
	size = ls.size();

	for(list<string>::iterator it=ls.begin();it!=ls.end();++it)
	{
		n++;
		rdbeif_uneifdir(*it);
		printf("# uneiftxt_dir_i(%d)... %.2f%% -- %s \n\n",size,n*100/(double)size,(*it).c_str());

		///////////////////////////////////
	}

	printf("# uneiftxt_dir(%d) 100%% -- %s \n",size,txtpath.c_str());
	return 0;
}
int rdbeif_eifclean(const char* cvi_path)
{
	int ret = rdbeif_clean(cvi_path);
	if(0==ret)
	{
		printf("# eifclean ok( %s ) \n\n",cvi_path);
	}
	return 0;
}
int rdbeif_eifclean_by_txt(const string& txtpath)
{
	list<string> ls;
	int size=0;
	int n=0;
	if(txtpath.empty())
	{
		printf("# Please specify the path - txtpath!!!\n");
		return 0;
	}

	cl_util::get_stringlist_from_file(txtpath,ls);
	size = ls.size();

	for(list<string>::iterator it=ls.begin();it!=ls.end();++it)
	{
		n++;
		rdbeif_eifclean((*it).c_str());
		printf("# eifclean(%d)... %.2f%% -- %s \n\n",size,n*100/(double)size,(*it).c_str());

		///////////////////////////////////
	}

	printf("# eifclean by txt end(%d) 100%% -- %s \n",size,txtpath.c_str()); 
	return 0;
}

int testread(const char* path)
{
	printf("testread %s start...\n",path);

	size64_t size=0,readsize=0;
	cl_file64 file;
	if(0!=file.open(path,F64_READ))
	{
		printf("#***open fail!\n");
		return 0;
	}
	file.seek(0,SEEK_END);
	size = file.tell();
	file.seek(0,SEEK_SET);

	char *buf = new char[10240];
	int tmp = 0;
	int seeksize = 0;
	while(readsize<size)
	{
		tmp = file.read(buf,10240);
		if(tmp<=0)
		{
			perror("*** file.read():");
			printf("\n*** read failed ret=%d, pos=%lld, seeksize=%d \n",tmp,file.tell(),seeksize);
			if(-1==file.seek(1,SEEK_SET))
			{
				perror("*** file.seek():");
				break;
			}
			seeksize++;
			//break;
		}
		readsize += tmp;
		printf("\r : %d/1000 %lld/%lld",(int)(readsize*1000/size),readsize,size);
	}
	
	file.close();
	delete[] buf;
	
	printf("testread end \n");
	return 0;
}
void crc32_folder(const char* dir)
{
	list<string> lspath;
	list<int> lsid;
	map<unsigned int, string> hashmap;
	map<unsigned int, string>::iterator mpit;
	unsigned int nhash;

	cl_util::get_folder_files(dir, lspath, lsid);
	
	for (list<string>::iterator it = lspath.begin(); it != lspath.end(); ++it)
	{
		if (0 == cl_fhash_crc32_file((*it).c_str(), nhash))
		{
			mpit = hashmap.find(nhash);
			if (mpit != hashmap.end())
			{
				mpit->second += " == ";
				mpit->second += *it;
			}
			else
			{
				hashmap[nhash] = *it;
			}
			printf("crc32: %-15x | %s \n", nhash, (*it).c_str());
		}
	}
	
	list<string> ls,ls2;
	char buf[4096];
	for (mpit = hashmap.begin(); mpit != hashmap.end(); ++mpit)
	{
		sprintf(buf, "%s ------- %x", mpit->second.c_str(), mpit->first);
		ls.push_back(buf);
		if (strstr(buf, "=="))
			ls2.push_back(buf);
	}
	ls.sort();
	ls2.sort();
	string sdir = dir;
	string path = sdir + "/fhash_list.txt";
	string path2 = sdir + "/fhash_list2.txt";
	cl_util::put_stringlist_to_file(path, ls);
	cl_util::put_stringlist_to_file(path2, ls2);
}