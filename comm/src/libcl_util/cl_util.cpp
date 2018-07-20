#include "cl_util.h"
#include <assert.h>
#include <time.h>
#include <stdio.h>
#include "cl_rfdelete.h"
#include "cl_synchro.h"

#ifdef _WIN32
	#include <winsock2.h>
#define WIN32_LEAN_AND_MEAN		// �� Windows ͷ���ų�����ʹ�õ�����
	#include <windows.h>
	#include <crtdbg.h>
	#include <Iphlpapi.h>
	//#pragma comment(lib,"Iphlpapi.lib")
#else
	#include <unistd.h>
	#include <signal.h>
	#include <sys/types.h>
	#include <sys/stat.h>
	#include <sys/socket.h>
	#include <sys/select.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <fcntl.h>
	#include <netdb.h>
	#include <net/if_arp.h>  
	#include <net/if.h>
	//#include <stropts.h>
	#include <sys/ioctl.h> 
	#include <linux/hdreg.h>
	#include <dirent.h>
	#include <stdarg.h>
#ifdef _OS
#include <sys/param.h>
#include <sys/mount.h>
#else
	#include <sys/vfs.h>
#endif
	#include <pthread.h>
	#include <sys/file.h>
#endif

//#include "cl_DIPCache.h"

//int cl_util::socket_init()
//{
//#ifdef _WIN32
//	WSADATA wsaData;
//	if(0!=WSAStartup(0x202,&wsaData))
//	{
//		perror("WSAStartup false! : ");
//		return -1;
//	}
//	return 0;
//#else
//	signal(SIGPIPE, SIG_IGN); //����Broken pipe,����socket�Զ˹ر�ʱ��������д�����Broken pipe���ܵ����ѣ�
//#endif
//	return 0;
//}
//
//void cl_util::socket_fini()
//{
//#ifdef _WIN32
//	WSACleanup();
//#endif
//}
void cl_util::linux_register_signal_exit(void (*FUNC)(int)) //ֻ��linux��
{
	//�����Ǽ��ֳ������źţ�
	//SIGHUP=1 �����ն��Ϸ����Ľ����ź�.
	//SIGINT=2   �����Լ��̵��ж��ź� ( ctrl + c ) .
	//SIGKILL=9 �����źŽ��������źŵĽ��� . ���ܲ���
	//SIGTERM=15��kill ����� ���ź�.
	//SIGCHLD=17����ʶ�ӽ���ֹͣ��������ź�. 
	//SIGSTOP=19�����Լ��� ( ctrl + z ) ����Գ����ִֹͣ���ź�.. ���ܲ���
	//SIGSEGV=11: �δ��� 
	/*
	SIGSEGV core dump����
	��1��gcc -g ����     
	ulimit -c 20000      
	֮�����г���
	��core dump      
	���gdb -c core <exec file>      
	�������ջ
	��2��ʹ��strace execfile�����г��򣬳���ʱ����ʾ�Ǹ�ϵͳ���ô� 
	 */
#if defined(__GNUC__) && defined(__linux__)
	//��ʼ���źźţ���������ֹʱkill�������ӳ���
	signal(SIGHUP,FUNC);
	signal(SIGINT,FUNC);
	signal(SIGTERM,FUNC);
#endif
}
//*************************
cl_Semaphore g_cl_sem_exit;
void cl_util::wait_exit()
{
	g_cl_sem_exit.wait();
}
void cl_util::signal_exit()
{
	g_cl_sem_exit.signal();
}

void cl_util::debug_memleak()
{
#ifdef _WIN32
	//����ڴ�й©
	_CrtSetDbgFlag(_CRTDBG_LEAK_CHECK_DF | _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG));
	//_CrtDumpMemoryLeaks(); //ִ���������164B�ڴ�й©
	//_CrtSetBreakAlloc(116);
#else
	//Ҫ����core�ļ����벻����SIGSEGV��ͬʱϵͳ�ſ�ulimit -c unlimited ����,��������" -g -rdynamic" ����,dump��backtrace ���ҳ���ʱ����ջ��
	//signal(SIGSEGV,&uni_dump); 
#endif
}

int cl_util::chdir(const char* path)
{
#ifdef _WIN32
	return TRUE==::SetCurrentDirectoryA(path);
#else
	return ::chdir(path);
#endif
}

int _GetModuleFileName(const char* lpszModuleName,char* lpszModulePath, int cbModule)
{
#ifdef _WIN32
	return ::GetModuleFileNameA(::GetModuleHandleA(lpszModuleName), lpszModulePath, cbModule);
#else
//#ifndef MAX_PATH
//#define MAX_PATH 256
//#endif //MAX_PATH
	
	//sprintf(cmdline,"/proc/%d/cmdline",getpid());
	char exelink[128];
	sprintf(exelink,"/proc/%d/exe",getpid());
	int count = readlink(exelink,lpszModulePath,cbModule);
	if(count>0)
		lpszModulePath[count] = '\0';
	else
		lpszModulePath[0] = '\0';
	return count;
#endif
}
string cl_util::get_module_path()
{
	char buf[256]={0,};
	_GetModuleFileName(NULL,buf,256);
	return buf;
}
string cl_util::get_module_dir()
{
	char buf[256]={0,};
	if(_GetModuleFileName(NULL,buf,256))
	{
		char *p = ::strrchr(buf,'/');
		if(!p)
			p = ::strrchr(buf,'\\');
		if(p)
			*(p+1)='\0';
	}
	//DEBUGMSG("module path = %s \n",buf);
	return buf;
}
string cl_util::get_module_name()
{
	char buf[256]={0,};
	if(_GetModuleFileName(NULL,buf,256))
	{
		char *p = ::strrchr(buf,'/');
		if(!p)
			p = ::strrchr(buf,'\\');
		if(p)
			return (p+1);
	}
	//DEBUGMSG("module path = %s \n",buf);
	return buf;
}
PSL_HANDLE cl_util::lockname_create(const char* name)
{
#ifdef _WIN32
	HANDLE hMutex = NULL;
	SECURITY_DESCRIPTOR sd; 
	InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION); 
	SetSecurityDescriptorDacl(&sd, TRUE, (PACL) NULL, FALSE); 

	SECURITY_ATTRIBUTES sa; 
	sa.nLength = sizeof(sa); 
	sa.lpSecurityDescriptor = &sd; 
	sa.bInheritHandle = TRUE; 
	hMutex = ::CreateMutexA(&sa,TRUE, name);  //����������
	int err = GetLastError();
	if(hMutex)
	{
		if(err == ERROR_ALREADY_EXISTS)   
		{   
			//������л������������ͷž������λ������
			CloseHandle(hMutex);
			hMutex = NULL;
		}
	}
	return hMutex;
#else
	//����ٶ�open()���᷵��0�ľ��
	int fd = open(name,O_RDONLY,0666);
	if(-1==fd)
	{
		fd = open(name,O_CREAT|O_RDWR,0666);
		if(-1==fd)
		{
			perror("open():");
			return 0;
		}
	}
	int ret = flock(fd,LOCK_EX|LOCK_NB);
	if(-1==ret)
	{
		perror("flock():");
		close(fd);
		return 0;
	}
	return fd;
#endif
}
PSL_HANDLE cl_util::process_single_lockname_create()
{
	string markname;
#ifdef _WIN32
	markname = cl_util::get_module_name();
#else
	markname = cl_util::get_module_path();
#endif
	return cl_util::lockname_create(markname.c_str());
}
void cl_util::process_single_lockname_close(PSL_HANDLE h)
{
#ifdef _WIN32
	if(h)
		CloseHandle(h);
#else
	if(0!=h && -1!=h)
		close(h);
#endif
}
int cl_util::get_system_version()
{
	int ver=0;
#ifdef _WIN32
	OSVERSIONINFO *osvi = new OSVERSIONINFO();
	if(NULL == osvi)
		return 0;

	memset(osvi,0, sizeof(OSVERSIONINFO));
	osvi->dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(osvi);
	switch(osvi->dwPlatformId)
	{
	case VER_PLATFORM_WIN32s:
		{
			ver = 1; //win 3.1
		}
		break;
	case VER_PLATFORM_WIN32_WINDOWS:
		{
			ver = 11;    //win 95
			if(osvi->dwMajorVersion == 4L && osvi->dwMinorVersion == 10L)
				ver = 12; //win 98
			else if(osvi->dwMajorVersion == 4L && osvi->dwMinorVersion == 90L)
				ver = 13; //win ME
		}
		break;
	case VER_PLATFORM_WIN32_NT:
		{
			ver = 21;
			if(osvi->dwMajorVersion <= 4L)
				ver = 21; //win NT4
			else if(osvi->dwMajorVersion == 5L && osvi->dwMinorVersion == 0L)
				ver = 22; //win 2000
			else if(osvi->dwMajorVersion == 5L && osvi->dwMinorVersion == 1L)
				ver = 23; //win XP
			else if( osvi->dwMajorVersion == 5L && osvi->dwMinorVersion == 2L )
                ver = 24; //Microsoft Windows Server 2003 family
			else
				ver = 25; //WindowsVista;
		}
		break;
	default:
		break;
	}

	delete osvi;
#else
	ver = 50;
#endif
	return ver;
}

string cl_util::get_system_version_str()
{
	string ver="0";
#ifdef _WIN32
	OSVERSIONINFO *osvi = new OSVERSIONINFO();
	if(NULL == osvi)
		return ver;

	memset(osvi,0, sizeof(OSVERSIONINFO));
	osvi->dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(osvi);
	switch(osvi->dwPlatformId)
	{
	case VER_PLATFORM_WIN32s:
		{
			ver = "win 3.1"; //win 3.1
		}
		break;
	case VER_PLATFORM_WIN32_WINDOWS:
		{
			ver = "win 95";    //win 95
			if(osvi->dwMajorVersion == 4L && osvi->dwMinorVersion == 10L)
				ver = "win 98"; //win 98
			else if(osvi->dwMajorVersion == 4L && osvi->dwMinorVersion == 90L)
				ver = "win ME"; //win ME
		}
		break;
	case VER_PLATFORM_WIN32_NT:
		{
			ver = "win NT";
			if(osvi->dwMajorVersion <= 4L)
				ver =  "win NT4"; //win NT4
			else if(osvi->dwMajorVersion == 5L && osvi->dwMinorVersion == 0L)
				ver =  "win 2000"; //win 2000
			else if(osvi->dwMajorVersion == 5L && osvi->dwMinorVersion == 1L)
				ver =  "win XP"; //win XP
			else if( osvi->dwMajorVersion == 5L && osvi->dwMinorVersion == 2L )
                ver =  "win 2003"; //Microsoft Windows Server 2003 family
			else
				ver = "win Vista"; //WindowsVista;
		}
		break;
	default:
		break;
	}

	delete osvi;
#else
	char buf[1024] = {0,};
	read_buffer_from_file(buf,1024,"/etc/issue");
	char *p = strchr(buf,'\n');
	if(p) *p = '\0';
	ver = buf;
#endif
	return ver;
}
#ifdef _WIN32
int cl_util::get_all_volumes(list<string>& ls)
{
	char *ptr;
	int len = 0;
	len = GetLogicalDriveStringsA(0,NULL);
	char *buf = new char[len+10];
	memset(buf,0,len+10);
	len = GetLogicalDriveStringsA(len+10,buf);
	ptr = buf;
	ls.clear();
	while(*ptr)
	{
		if(DRIVE_FIXED == GetDriveTypeA(ptr))
			ls.push_back(ptr);
		while(*ptr++);
	}

	delete[] buf;
	return 0;
}
#else
int cl_util::get_all_volumes(list<string>&/* ls*/)
{
	assert(0);
	return 0;
}
#endif
bool cl_util::get_volume_size(const string& volume,ULONGLONG &total,ULONGLONG &used,ULONGLONG &free)
{
#ifdef _WIN32
	if(volume.length()<2)
		return false;
	string Driver;
	if(volume.at(1)!=':')
	{
		char buf[MAX_PATH];
		GetModuleFileNameA(NULL,buf,MAX_PATH);
		buf[3]='\0';
		Driver = buf;
	}
	else
		Driver = volume.substr(0,3);
	if(!Driver.empty())
	{
		//���ѡ���ļ���,������̿ռ���ô�С
		//��ô��̿ռ���Ϣ
		ULARGE_INTEGER FreeAv,TotalBytes,FreeBytes;
		if(GetDiskFreeSpaceExA(Driver.c_str(),&FreeAv,&TotalBytes,&FreeBytes))
		{
			//
			total = TotalBytes.QuadPart;
			free = FreeBytes.QuadPart;
			used = total - free;
			return true;
		}
	}
#else
	struct statfs buf;
	if(0==statfs(volume.c_str(), &buf))
	{
		total = (ULONGLONG)buf.f_blocks * buf.f_bsize;
		free = (ULONGLONG)buf.f_bavail * buf.f_bsize;
		used = total - free;
	}
	else
	{
		printf("#: ***statfs(%s,statfs) failed: \n",volume.c_str());
		perror("****** statfs() failed:");
		total = (ULONGLONG)1000000000 * 10;//10G
		used = (ULONGLONG)0;
		free = total-used;
	}
	return true;
#endif
	return false;
}

int cl_util::my_create_directory(const string& path)
{
	if(path.empty())
		return -1;
	string str = path;
	size_t pos = str.find("\\");
	while(pos != string::npos)
	{
		str.replace(pos,1,"/",1);
		pos = str.find("\\");
	}
	if(str.substr(str.length()-1) == "/")
		str = str.substr(0,str.length()-1);

	pos = str.rfind('/');
	string lstr;
	if(pos!=string::npos)
		lstr = str.substr(0,pos);
	else
		lstr = "";
	my_create_directory(lstr);
#ifdef _WIN32
	return CreateDirectoryA(str.c_str(),NULL)?0:-1;
#else
	return mkdir(str.c_str(),0777);
#endif
}

int cl_util::create_directory_by_filepath(const string& filepath)
{
	//���ļ�����ȫ·��
	int pos1,pos2;
	pos1 = (int)filepath.rfind('\\');
	pos2 = (int)filepath.rfind('/');
	if(pos1==-1 && pos2==-1)
		return -1;
	if(pos1>pos2)
		return my_create_directory(filepath.substr(0,pos1));
	else
		return my_create_directory(filepath.substr(0,pos2));
}
int cl_util::file_state(const char* path)
{
	//0=�����ڣ�1=�ļ���2=�ļ���
	int state = 0;
	if(NULL==path || 0==strlen(path))
		return 0;
#ifdef _MSC_VER
	WIN32_FIND_DATAA fd;
	HANDLE hFind = ::FindFirstFileA(path,&fd);
	if(hFind!=INVALID_HANDLE_VALUE)
	{
		if(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			state = 2;
		else
			state = 1;
		FindClose(hFind);
	}
#elif defined(__GNUC__)
	struct stat statbuf;
	if(0==stat(path,&statbuf))
	{
		if(S_ISREG(statbuf.st_mode))
			state = 1;
		else if(S_ISDIR(statbuf.st_mode))
			state = 2;
	}
#endif
	return state;
}

bool cl_util::file_same(const string& path1, const string& path2)
{
	string p1, p2;
	p1 = path1;
	p2 = path2;
	filepath_format(p1);
	filepath_format(p2);
	if (p1 == p2)
		return true;
#ifdef _MSC_VER
	if (0 == stricmp(p1.c_str(), p2.c_str()))
		return true;
#elif defined(__GNUC__)
	struct stat s1, s2;
	if (0 != stat(p1.c_str(), &s1))
		return false;
	if (0 != stat(p2.c_str(), &s2))
		return false;
	if (s1.st_ino == s2.st_ino)
		return true;
#endif
	return false;
}

bool cl_util::file_exist(const string& path)
{
	return 1==file_state(path.c_str());
}
int cl_util::file_delete(const string& path)
{
	return cl_rfdelete(path.c_str());
}
int cl_util::file_rename(const string& from,const string& to)
{
	return rename(from.c_str(),to.c_str());
}

bool cl_util::get_stringlist_from_file(const string& path,list<string>& ls)
{
	const int BUFLEN=20480;
	char *buf = new char[BUFLEN];
	char *ptr = NULL;
	bool bret = false;
	FILE *fp = fopen(path.c_str(),"rb");
	if(fp)
	{
		ls.clear();
		while(!feof(fp))
		{
			memset(buf,0,BUFLEN);
			if(NULL==fgets(buf,BUFLEN,fp))
				continue;
			if((ptr=strstr(buf,"\r")))
				*ptr = '\0';
			if((ptr=strstr(buf,"\n")))
				*ptr = '\0';

			ls.push_back(buf);
		}
		fclose(fp);
		bret = true;
	}
	delete[] buf;
	return bret;
}
bool cl_util::put_stringlist_to_file(const string& path,list<string>& ls)
{
	string tmppath = (string&)path + ".tmp";
	FILE *fp = fopen(tmppath.c_str(),"wb+");
	if(!fp)
		return false;
	for(list<string>::iterator it=ls.begin();it!=ls.end(); ++it)
	{
		if(!(*it).empty())
			fwrite((*it).c_str(),(*it).length(),1,fp);
		fwrite("\r\n",2,1,fp);
	}
	fflush(fp);
	fclose(fp);
	cl_util::file_delete(path);
	Sleep(0);
	cl_util::file_rename(tmppath,path);
	return true;
}
int cl_util::get_file_modifytime(const char* path)
{
#ifdef _WIN32
	WIN32_FIND_DATAA ffd ;
	HANDLE hFind = FindFirstFileA(path,&ffd);
	if(hFind != INVALID_HANDLE_VALUE)
	{
		FindClose(hFind);
		return ffd.ftLastWriteTime.dwLowDateTime;
	}
#else
	struct stat statbuf;
	if(0==lstat(path,&statbuf))
	{
		return statbuf.st_mtime;
	}
#endif
	return 0;
}
int cl_util::get_filetimes(const char* path,time_t& ctime,time_t& mtime,time_t& atime)
{
#ifdef _WIN32
	//32λwindows: 8=sizeof(time_t);
	//windows�µ�ctimeΪ�ļ�����ʱ��
#define FILETIME_TO_TIMET(ft) (((((ULONGLONG)ft.dwHighDateTime) << 32) + ft.dwLowDateTime - 116444736000000000)/10000000)
	WIN32_FIND_DATAA ffd ;
	HANDLE hFind = FindFirstFileA(path,&ffd);
	if(hFind != INVALID_HANDLE_VALUE)
	{
		FindClose(hFind);
		ctime = (time_t)FILETIME_TO_TIMET(ffd.ftCreationTime);
		mtime = (time_t)FILETIME_TO_TIMET(ffd.ftLastWriteTime);
		atime = (time_t)FILETIME_TO_TIMET(ffd.ftLastAccessTime);
		return 0;
	}
#else
	//32λlinux: 4=sizeof(time_t);
	//linux�µ�ctimeΪinode�ڵ�(״̬)������޸�ʱ��
	struct stat statbuf;
	if(0==lstat(path,&statbuf))
	{
		//ע��:���潫st_mtime��ctime������ʱ��
		//ctime = statbuf.st_ctime;
		ctime = statbuf.st_mtime;
		mtime = statbuf.st_mtime;
		atime = statbuf.st_atime;
		return 0;
	}
#endif
	////time to string (ctime: 2010-7-30 15:39:39)
	//tm* t = localtime(&ctime);
	//sprintf(buf,"ctime: %d-%d-%d %d:%d:%d\r\n",t->tm_year+1900,t->tm_mon+1,t->tm_mday,t->tm_hour,t->tm_min,t->tm_sec);
	//printf("-%s",::ctime(&ctime));
	//printf("-%s",asctime(t));
	return -1;
}

//���ָ����չ��,��ֻȡָ����չ��,����̳�,��ݹ���Ŀ¼����.������ȡ�ļ�����.
long cl_util::get_folder_files(const string& dir,
						list<string>& ls_path,
						list<int>& ls_ino,
						const string& suffix/* = ""*/,
						int enable_samefile/* = 0*/,
						bool inherit/* = true*/)
{
#ifdef _WIN32
	if(dir.empty())
		return 0;

	string str;
	list<string> ls_suffix;
	list<string>::iterator it;
	int n = get_string_index_count(suffix,"|");
	for(int i=0;i<n;++i)
	{
		str = get_string_index(suffix,i,"|");
		string_trim(str);
		if(!str.empty())
			ls_suffix.push_back(str);
	}

	bool bfind = false;
	long count = 0;
	string strdir = dir;
	string path,name;
	size_t iExLen = suffix.length();
	if(strdir.at(strdir.length()-1) != '\\' && strdir.at(strdir.length()-1) != '/')
		strdir += "\\";

	WIN32_FIND_DATAA fd;
	HANDLE hFind = ::FindFirstFileA((strdir+"*.*").c_str(),&fd);
	if(hFind!=INVALID_HANDLE_VALUE)
	{
		do{
			if(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				if(inherit && strcmp(fd.cFileName,".") && strcmp(fd.cFileName,".."))
				{
					//count += get_dirfiles(arrName,arrPath,strdir + fd.cFileName,strEx,bInherit);
					count += get_folder_files(strdir + fd.cFileName,ls_path,ls_ino,suffix,enable_samefile,inherit);
				}
			}
			else
			{
				if(strcmp(fd.cFileName,".") && strcmp(fd.cFileName,".."))
				{
					path = strdir + fd.cFileName;
					name = fd.cFileName;

					bfind = false;
					if(ls_suffix.empty())
					{
						bfind = true;
					}
					else
					{
						int pos1,pos2;
						pos1 = (int)name.rfind('.');
						pos2 = (int)name.rfind('\\');
						if(pos2>=pos1)
							str = "";
						else
							str = name.substr(pos1+1);
						if(!str.empty())
						{
							for(it=ls_suffix.begin();it!=ls_suffix.end();++it)
							{
								//printf("----------scan suffix:::: %s --- %s \n",str.c_str(),(*it).c_str());
								if(0==stricmp(str.c_str(),(*it).c_str()))
								{
									
									bfind = true;
									break;
								}
							}
						}
					}
					if(bfind)
					{
						ls_path.push_back(path);
						//arrName.push_back(name);
						count ++;
					}
				}
			}
		}while(FindNextFileA(hFind,&fd));
		FindClose(hFind);
	}
	return count;
#else
	string str,new_dir;
	int n = 0;

	new_dir = dir;
	if(new_dir.empty())
		return 0;
	if(new_dir.at(new_dir.length()-1)=='/')
		new_dir.erase(new_dir.length()-1);
	if(new_dir.empty())
		return 0;

	list<string> ls_suffix;
	list<string>::iterator it;
	int count = get_string_index_count(suffix,"|");
	for(int i=0;i<count;++i)
	{
		str = get_string_index(suffix,i,"|");
		string_trim(str);
		if(!str.empty())
			ls_suffix.push_back(str);
	}

	DIR *dp=NULL;
	struct dirent *entry=NULL;
	struct stat statbuf;

	dp = opendir(new_dir.c_str());
	if(NULL==dp)
	{
		printf("********** opendir faild : dir = %s \n",new_dir.c_str());
		return n;
	}

	char old_dir[1024]={0};
	getcwd(old_dir,1024);
	chdir(new_dir.c_str());

	bool bfind = false;
	while(NULL!=(entry=readdir(dp)))
	{
		if(0==strcmp(entry->d_name,".") || 0==strcmp(entry->d_name,".."))
			continue;
		//ע�⣺ʹ��lstat()�Ļ�������������ֻ�Ƕ������ӵ���Ϣ�����Ƕ������������õ��ļ�����Ϣ
		//if(0==lstat(entry->d_name,&statbuf))
		if(0==stat(entry->d_name,&statbuf))
		{
			//printf("------->d_ino=%d,  d_type=%d  path=%s/%s,  dev=%d,  ino=%d,  mode=%d,  nlink=%d,\n",(int)entry->d_ino,(int)entry->d_type,new_dir.c_str(),entry->d_name,
			//	(int)statbuf.st_dev,(int)statbuf.st_ino,(int)statbuf.st_mode,(int)statbuf.st_nlink);
			//////DT_LNK=10,DT_REG=8,DT_DIR=4
			//printf("d_ino_len =%d ,st_ino_len = %d \n",sizeof(entry->d_ino),sizeof(statbuf.st_ino));

			if(S_ISREG(statbuf.st_mode))
			{
				bfind = false;
				if(ls_suffix.empty())
				{
					bfind = true;
				}
				else
				{
					str = entry->d_name;
					int pos1,pos2;
					pos1 = str.rfind('.');
					pos2 = str.rfind('/');
					if(pos2>=pos1)
						str = "";
					else
						str = str.substr(pos1+1);

					if(!str.empty())
					{
						for(it=ls_suffix.begin();it!=ls_suffix.end();++it)
						{
							//printf("----------scan suffix:::: %s --- %s \n",str.c_str(),(*it).c_str());
							if(0==strcasecmp(str.c_str(),(*it).c_str()))
							{
								
								bfind = true;
								break;
							}
						}
					}
				}
				if(bfind)
				{
					//����������ظ��Ļ�����Ƿ��Ѿ�����
					if(!enable_samefile)
					{
						for(list<int>::iterator it=ls_ino.begin();it!=ls_ino.end();++it)
						{
							if((*it) == (int)statbuf.st_ino)
							{
								bfind = false;
							}
						}
					}
				}
				if(bfind)
				{
					ls_path.push_back(new_dir + "/" + entry->d_name);
					ls_ino.push_back((int)statbuf.st_ino);
					++n;
				}
			}
			else if(S_ISDIR(statbuf.st_mode))
			{
				if(inherit && 0!=strcmp(entry->d_name,".") && 0!=strcmp(entry->d_name,".."))
					n += get_folder_files(new_dir + "/" + entry->d_name,ls_path,ls_ino,suffix,enable_samefile,inherit);
			}
			else
			{
				printf("*******unkown path : %s \n",entry->d_name);
			}
		}
	}

	//printf("+++++++++++++old_dir=%s \n",old_dir);
	chdir(old_dir);
	closedir(dp);
	return n;
#endif
}
int cl_util::get_folder_nodes(const string& dir,list<string>& folders,list<string>& files)
{
#ifdef _WIN32
	if(dir.empty())
		return -1;

	int n = 0;
	string strdir = dir;
	if(strdir.at(strdir.length()-1) != '\\' && strdir.at(strdir.length()-1) != '/')
		strdir += "\\";

	WIN32_FIND_DATAA fd;
	HANDLE hFind = ::FindFirstFileA((strdir+"*.*").c_str(),&fd);
	if(hFind!=INVALID_HANDLE_VALUE)
	{
		do{
			if(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				if(strcmp(fd.cFileName,".") && strcmp(fd.cFileName,".."))
				{
					folders.push_back(fd.cFileName);
					n++;
				}
			}
			else
			{
				if(strcmp(fd.cFileName,".") && strcmp(fd.cFileName,".."))
				{
					files.push_back(fd.cFileName);
					n++;
				}
			}
		}while(FindNextFileA(hFind,&fd));
		FindClose(hFind);
	}
	else
		n = -1;
	return n;
#else
	string str,new_dir;
	int n = 0;

	new_dir = dir;
	if(new_dir.empty())
		return -1;
	if(new_dir.at(new_dir.length()-1)=='/')
		new_dir.erase(new_dir.length()-1);
	if(new_dir.empty())
		return -1;

	DIR *dp=NULL;
	struct dirent *entry=NULL;
	struct stat statbuf;

	dp = opendir(new_dir.c_str());
	if(NULL==dp)
	{
		return -1;
	}

	char old_dir[1024]={0};
	getcwd(old_dir,1024);
	chdir(new_dir.c_str());

	while(NULL!=(entry=readdir(dp)))
	{
		if(0==strcmp(entry->d_name,".") || 0==strcmp(entry->d_name,".."))
			continue;
		//ע�⣺ʹ��lstat()�Ļ�������������ֻ�Ƕ������ӵ���Ϣ�����Ƕ������������õ��ļ�����Ϣ
		//if(0==lstat(entry->d_name,&statbuf))
		if(0==stat(entry->d_name,&statbuf))
		{
			if(S_ISREG(statbuf.st_mode))
			{
				files.push_back(entry->d_name);
				n++;
			}
			else if(S_ISDIR(statbuf.st_mode))
			{
				if(0!=strcmp(entry->d_name,".") && 0!=strcmp(entry->d_name,".."))
				{
					folders.push_back(entry->d_name);
					n++;
				}
			}
			else
			{
				//unkown type
				folders.push_back(entry->d_name);
				n++;
			}
		}
	}

	chdir(old_dir);
	closedir(dp);
	return n;
#endif
}
string cl_util::get_filename(const string& path)
{
	int pos1 = (int)path.rfind('\\');
	int pos2 = (int)path.rfind('/');
	if(pos1<pos2)
		return path.substr(pos2+1);
	else if(pos1>pos2)
		return path.substr(pos1+1);
	else
		return path;
}
string cl_util::get_filename_prename(const string& path)
{
	string name = get_filename(path);
	size_t pos = name.rfind('.');
	if(pos!=string::npos)
		return name.substr(0,pos);
	return name;
}
string cl_util::get_filename_extension(const string& path)
{
	string name = get_filename(path);
	size_t pos = name.rfind('.');
	if(pos!=string::npos)
		return name.substr(pos);
	return "";
}
string cl_util::get_filedir(const string& path)
{
	int pos1 = (int)path.rfind('\\');
	int pos2 = (int)path.rfind('/');
	if(pos1<pos2)
		return path.substr(0,pos2+1);
	else if(pos1>pos2)
		return path.substr(0,pos1+1);
	else
		return "";
}
void cl_util::filepath_split(const string& path,string& dir,string& prename,string& ext)
{
	int pos = (int)path.rfind('\\');
	int pos2 = (int)path.rfind('/');
	if(pos<pos2) pos = pos2;
	string name;
	if(pos>=0)
	{
		dir = path.substr(0,pos+1);
		name = path.substr(pos+1);
	}
	else
	{
		dir = "";
		name = path;
	}
	pos = (int)name.rfind('.');
	if(pos>=0)
	{
		prename = name.substr(0,pos);
		ext = name.substr(pos);
	}
	else
	{
		prename = name;
		ext = "";
	}
}
string& cl_util::filepath_format(string& path)
{
	//����м���/../,��ȥ��.
	string str2,str3;
	int n,n2;
	cl_util::str_replace(path,"\\","/");
	while(1)
	{
		n = (int)path.find("/../");
		if(n>0)
		{
			str2 = path.substr(0,n);
			str3 = path.substr(n+3);
			n2 = (int)str2.rfind('/');
			if(n2>=0)
				str2.erase(n2);
			path = str2 + str3;
		}
		else
			break;
	}
	
	cl_util::str_replace(path,"/./","/");
	return path;
}
string& cl_util::dir_add_tail_symbol(string& dir) //β����"/"
{
	if(!dir.empty())
	{
		char c = dir.at(dir.length()-1);
		if(c!='\\' && c!='/')
			dir += "/";
	}
	return dir;
}
string& cl_util::dir_del_tail_symbol(string& dir) //β��ȥ��"/"
{
	if(!dir.empty())
	{
		char c = dir.at(dir.length()-1);
		if(c=='\\' || c=='/')
			dir.erase(dir.length()-1);
	}
	return dir;
}
string& cl_util::dir_to_fulldir(string& dir,const string& referdir) //ת�ɾ���·��
{
	string newdir;
	dir_add_tail_symbol(dir);
	if((dir.length()>1&&dir.at(1)==':') || (!dir.empty()&&dir.at(0)=='/'))
		return dir;
	newdir = referdir;
	dir = newdir + dir;

	filepath_format(dir);
	return dir;
}
/////////////////////////////////////////////////////////////
int cl_util::atoi(const char* _Str,int nullval/*=0*/) //��һ���ж�NULLʱ����0
{
	if(NULL==_Str)
		return nullval;
	return ::atoi(_Str);
}
long long cl_util::atoll(const char* _Str,int nullval/*=0*/)
{
	long long i=0;
	if(NULL==_Str)
		return nullval;
	if(1!=sscanf(_Str,"%lld",&i))
		return nullval;
	return i;
}
char* cl_util::itoa_buf(char* buf,int i)
{
	sprintf(buf,"%d",i);
	return buf;
}
string cl_util::itoa(int i)
{
	char buf[64];
	sprintf(buf,"%d",i);
	return buf;
}


string cl_util::hex2str_x(const unsigned char* dgst,unsigned int dgst_len)
{
	string str;
	char *data = new char[dgst_len*2+2];
	char *p = data;
	for (int i = 0; i < (int)dgst_len; i++,p+=2)
        sprintf(p, "%02x", dgst[i]);
	*p = '\0';
	str = data;
	delete[] data;
	return str;
}
/////////////////////////////////////////////////////////////
//*****************************************
string cl_util::get_string_index(const string& source,int index,const string& sp)
{
	if(sp.empty() || index<0)
		return source;

	int splen = (int)sp.length();
	int pos1=0-splen,pos2=0-splen;

	int i=0;
	for(i=0;i<(index+1);i++)
	{
		pos1 = pos2+splen;
		pos2 = (int)source.find(sp,pos1);
		if(pos2<0)
			break;
	}
	if(i<index)
		return "";

	pos2 = pos2<pos1?(int)source.length():pos2;
	return source.substr(pos1,pos2-pos1);

}
int cl_util::get_string_index_pos(const string& source,int index,const string& sp)
{
	if(index < 0 || sp.empty())
		return -1;
	int splen = (int)sp.length();
	int pos = 0 - splen;
	for(int i=0;(pos=(int)source.find(sp,pos+splen))>=0 && i<index;i++);
	return pos;
}
int cl_util::get_string_index_count(const string& source,const string& sp)
{
	if(sp.empty() || source.empty())
		return 0;
	int i=1,pos=0,splen = (int)sp.length();
	pos = 0-splen;
	for(i=1;(pos=(int)source.find(sp,pos+splen))>=0;i++);
	return i;

}
string cl_util::set_string_index(string &source,int index,const string& val, const string& sp)
{
	if(index<0 || sp.empty())
		return source;

	int splen = (int)sp.length();
	int i=0,pos1,pos2=0;

	pos1=0-splen,pos2=0-splen;
	for(i=0;i<(index+1);i++)
	{
		pos1 = pos2+splen;
		pos2 = (int)source.find(sp,pos1);
		if(pos2<0)
		{
			if(i==index)
				break;
			else
			{
				source += sp;//����;
				pos2  = (int)source.find(sp,pos1);//���һ���ҵ�
			}
		}
	}

	if(pos2<0)
	{
		assert(i == index);
		source = source.substr(0,pos1) + val;
	}
	else
	{
		source = source.substr(0,pos1) + val + source.substr(pos2);
	}
	return source;

}
bool cl_util::get_stringlist_from_string(const string& source,list<string>& ls)
{
	ls.clear();
	int n = get_string_index_count(source,"\n");
	string str;
	size_t len;
	for(int i=0;i<n;++i)
	{
		str = get_string_index(source,i,"\n");
		if(str.empty())
			continue;
		len = str.length();
		if(str.at(len-1) == '\r')
		{
			if(1==len)
				continue;
			str.erase(len-1);
		}
		ls.push_back(str);
	}
	return true;
}
char* cl_util::string_trim(char* sz,char c/*=' '*/)
{
	if(NULL==sz) return sz;
	int pos1=0,pos2=(int)strlen(sz)-1;
	for(; pos1<=pos2 && c==sz[pos1];++pos1);//ȫ�յĻ���Խ�߽�
	for(; pos2>pos1 && c==sz[pos2];--pos2);//�����Ѿ��жϹ�=,�˴�����
	if( pos1>pos2)
		sz[0]='\0';
	else
	{
		if(pos1>0)
			memmove(sz,sz+pos1,pos2-pos1+1);
		sz[pos2-pos1+1] = '\0';
	}
	return sz;
}
string& cl_util::string_trim(string& str,char c/*=' '*/)
{
	int pos1=0,pos2=(int)str.length()-1;
	for(; pos1<=pos2 && c==str.at(pos1);++pos1);//ȫ�յĻ���Խ�߽�
	for(; pos2>pos1 && c==str.at(pos2);--pos2);//�����Ѿ��жϹ�=,�˴�����
	if( pos1>pos2)
		str = "";
	else
		str = str.substr(pos1,pos2-pos1+1);
	return str;
}

string& cl_util::string_trim_endline(string& str)
{
	int pos1=0,pos2=(int)str.length()-1;
	for(; pos1<=pos2 && ('\r'==str.at(pos1)||'\n'==str.at(pos1));++pos1);//ȫ�յĻ���Խ�߽�
	for(; pos2> pos1 && ('\r'==str.at(pos2)||'\n'==str.at(pos2));--pos2);//�����Ѿ��жϹ�=,�˴�����
	if( pos1>pos2)
		str = "";
	else
		str = str.substr(pos1,pos2-pos1+1);
	return str;
}
string& cl_util::str_replace(string& str,const string& str_old,const string& str_new)
{
	if(str_old == str_new)
		return str;
	int pos = (int)str.find(str_old);
	while(pos > -1)
	{
		str.replace(pos,str_old.length(),str_new.c_str());
		pos = (int)str.find(str_old);
	}
	return str;
}

//**********************************************


string cl_util::get_disksn(const char* path) 
{     
	string sn = "";
#ifndef _WIN32
	int fd;
	struct hd_driveid hid;
	char buf[1024];
	fd = open(path, O_RDONLY); 
	if(fd>=0)
	{
		if(-1!=ioctl(fd, HDIO_GET_IDENTITY, &hid))
		{
			memcpy(buf,hid.serial_no,sizeof(hid.serial_no));
			buf[sizeof(hid.serial_no)] = '\0';
			sn = buf;
			string_trim(sn);
		}
		close (fd);
	}
	if(sn.empty())
	{
		//ͨ��/opt/suexec /sbin/hdparm -i /dev/sda;
		sprintf(buf,"/opt/suexec /sbin/hdparm -i %s",path);
		FILE *fp = popen(buf,"r");
		if(fp)
		{
			int n = fread(buf,1,1023,fp);
			pclose(fp);
			if(n>0)
			{
				buf[n] = '\0';
				//DEBUGMSG("#=======get disk sn:\n%s\n========\n",buf);
				char* p1 = strstr(buf,"SerialNo=");
				if(p1)
				{
					p1 += 9;
					char* p2 = strstr(p1,"\n");
					if(p2)
					{
						p2[0] = '\0';
					}
					sn = p1;
					string_trim(sn);
				}
			}
		}
	}
#endif
	//DEBUGMSG("# get_disk sn = %s \n",sn.c_str());
	return sn; 
} 
//int cl_util::get_umac(unsigned char umac[])
//{
//	unsigned int iumac[6];
//	string mac = get_mac();
//	sscanf(mac.c_str(),"%02X%02X%02X%02X%02X%02X", 
//		&iumac[0], &iumac[1], &iumac[2], &iumac[3], &iumac[4], &iumac[5]);
//	for(int i=0;i<6;++i)
//		umac[i] = (unsigned char) iumac[i];
//	return 0;
//}
//
//string cl_util::get_mac()
//{
//	ifinfo_s inf;
//	inf.ifcount = 10;
//	int i=0;
//	if(get_macall(&inf)>0)
//	{
//#ifndef _WIN32
//		//����eth0
//		for(i=0;i<inf.ifcount;++i)
//		{
//			if(0==strcmp("eth0",inf.ifs[i].name))
//			{
//				return inf.ifs[i].mac;
//			}
//		}
//#endif
//		//�����ڹ��� �� && !lookback
//		for(i=0;i<inf.ifcount;++i)
//		{
//			if(inf.ifs[i].isup && !inf.ifs[i].isloopback)
//			{
//				return inf.ifs[i].mac;
//			}
//		}
//		//����!lookback
//		for(i=0;i<inf.ifcount;++i)
//		{
//			if(!inf.ifs[i].isloopback)
//			{
//				return inf.ifs[i].mac;
//			}
//		}
//		return inf.ifs[0].mac;
//	}
//	return "000000000000";
//}
//int cl_util::get_macall(ifinfo_s *inf)
//{
//#ifdef _WIN32
//	int i = 0;
//	int n = inf->ifcount;
//	inf->ifcount = 0;
//	IP_ADAPTER_INFO AdapterInfo[16];       // Allocate information 
//	DWORD dwBufLen = sizeof(AdapterInfo);  // Save memory size of buffer
//	DWORD dwStatus = GetAdaptersInfo(AdapterInfo,&dwBufLen);
//	if(dwStatus != ERROR_SUCCESS)
//		return -1;
//	//memcpy(umac,AdapterInfo->Address,6);
//	PIP_ADAPTER_INFO pAdapterInfo = AdapterInfo; // Contains pointer to
//	do
//	{
//		strcpy(inf->ifs[i].name,pAdapterInfo->AdapterName);
//		sprintf(inf->ifs[i].mac,"%02X%02X%02X%02X%02X%02X", 
//			pAdapterInfo->Address[0], 
//			pAdapterInfo->Address[1], 
//			pAdapterInfo->Address[2], 
//			pAdapterInfo->Address[3], 
//			pAdapterInfo->Address[4], 
//			pAdapterInfo->Address[5]); 
//		inf->ifs[i].isup = true;
//		if(MIB_IF_TYPE_LOOPBACK==AdapterInfo->Type)
//			inf->ifs[i].isloopback = true;
//		else
//			inf->ifs[i].isloopback = false;
//		strcpy(inf->ifs[i].ip,pAdapterInfo->IpAddressList.IpAddress.String);
//		strcpy(inf->ifs[i].mask,pAdapterInfo->IpAddressList.IpMask.String);
//
//
//		i = ++inf->ifcount;
//		if(inf->ifcount>=n) break;
//	}while(NULL!=(pAdapterInfo=pAdapterInfo->Next));
//	return inf->ifcount;
//#else
//	int fd;
//	struct ifconf conf;
//	struct ifreq *ifr;
//	sockaddr_in *sin;
//	int i;
//	char buff[2048];
//
//	struct ifreq ifr2;
//	
//
//	conf.ifc_len = 2048;
//	conf.ifc_buf = buff;
//	fd = socket(AF_INET,SOCK_DGRAM,0);
//	ioctl(fd,SIOCGIFCONF,&conf);
//
//	inf->ifcount = conf.ifc_len / sizeof(struct ifreq);
//	ifr = conf.ifc_req;
//	if(inf->ifcount>10) inf->ifcount = 10;
//	for(i=0;i < inf->ifcount;++i)
//	{
//		sin = (struct sockaddr_in*)(&ifr->ifr_addr);
//		ioctl(fd,SIOCGIFFLAGS,ifr);
//		strcpy(inf->ifs[i].name,ifr->ifr_name);
//		strcpy(inf->ifs[i].ip,inet_ntoa(sin->sin_addr));
//		if(ifr->ifr_flags & IFF_LOOPBACK) 
//			inf->ifs[i].isloopback = true;
//		else
//			inf->ifs[i].isloopback = false;
//		if(ifr->ifr_flags & IFF_UP)
//			inf->ifs[i].isup = true;
//		else
//			inf->ifs[i].isup = false;
//		ifr++;
//	}
//
//	for(i=0;i < inf->ifcount;++i)
//	{
//		memset(inf->ifs[i].mac,0,16);
//		memset(&ifr2,0,sizeof(ifr2));
//		strcpy(ifr2.ifr_name,inf->ifs[i].name);
//#ifndef _OS
//		if(0==ioctl(fd,SIOCGIFHWADDR,&ifr2,sizeof(struct ifreq)))
//		{
//			unsigned char* umac = (unsigned char*)ifr2.ifr_hwaddr.sa_data;
//			sprintf(inf->ifs[i].mac,"%02X%02X%02X%02X%02X%02X", umac[0], umac[1], umac[2], umac[3], umac[4], umac[5]);
//		}
//#endif
//	}
//	close(fd);
//	return inf->ifcount;
//#endif
//}
//int cl_util::get_mtu()
//{
//	//ȡ������������С��mtu
//#ifdef _WIN32
//	PIP_ADAPTER_ADDRESSES pad = NULL;
//	ULONG padlen = 0;
//	DWORD mtu = 1500;
//	DWORD ret = 0;
//	GetAdaptersAddresses(AF_UNSPEC,0, NULL, pad,&padlen);
//	pad = (PIP_ADAPTER_ADDRESSES) malloc(padlen);
//	if(NO_ERROR==(ret=GetAdaptersAddresses(AF_INET,GAA_FLAG_SKIP_ANYCAST,0,pad,&padlen)))
//	{
//		mtu = pad->Mtu;
//		PIP_ADAPTER_ADDRESSES p = pad->Next;
//		while(p)
//		{
//			if(p->Mtu>0 && mtu > p->Mtu)
//				mtu = p->Mtu;
//			p = p->Next;
//		};
//	}
//
//	free(pad);
//	if(mtu<=0) mtu = 1500;
//	return (int)mtu;
//#else
//	struct ifreq *ifr;
//	struct ifconf conf;
//	int fd,mtu,n,i;
//
//	conf.ifc_len = 0;
//	conf.ifc_buf = NULL;
//	mtu = 1500;
//
//	if((fd = socket(AF_INET,SOCK_DGRAM,0))<=0)
//		goto fail;
//
//	if(0!=ioctl(fd,SIOCGIFCONF,&conf))
//		goto fail;
//	conf.ifc_buf = (char*)malloc(conf.ifc_len);
//	if(0!=ioctl(fd,SIOCGIFCONF,&conf))
//		goto fail;
//	
//	n = conf.ifc_len / sizeof(struct ifreq);
//	//printf("ifc_len = %d, n = %d, sizeof(ifreq)=%d \n",conf.ifc_len,n,sizeof(struct ifreq));
//	if(n>0)
//	{
//		ifr = conf.ifc_req;
//		if(0==ioctl(fd, SIOCGIFMTU, (void*)ifr))
//		{
//			mtu = ifr->ifr_mtu;
//			//printf("ifr: %s , mtu=%d \n",ifr->ifr_name,ifr->ifr_mtu);
//			for(i=1;i<n;++i)
//			{
//				ifr++;
//				if(0==ioctl(fd, SIOCGIFMTU, (void*)ifr))
//				{
//					if(ifr->ifr_mtu>0 && mtu>ifr->ifr_mtu)
//						mtu = ifr->ifr_mtu;
//					//printf("ifr: %s , mtu=%d \n",ifr->ifr_name,ifr->ifr_mtu);
//				}
//			}
//		}
//	}
//
//fail:
//	if(fd>0)
//		close(fd);
//	if(conf.ifc_buf)
//		free(conf.ifc_buf);
//	if(mtu<=0) mtu = 1500;
//	return mtu;
//
//#endif
//}
//
//int cl_util::get_server_time(time_t *t) //��time-nw.nist.gov 37 �л�ȡ1970������������
//{
//	string ip = cl_util::ip_explain_ex("time-nw.nist.gov");
//	SOCKET sock = socket(AF_INET,SOCK_STREAM,0);
//	if(sock == INVALID_SOCKET)
//		return -1;
//	
//	sockaddr_in addr;
//	memset(&addr,0,sizeof(addr));
//	addr.sin_family = AF_INET;
//	addr.sin_port = htons(37);
//	addr.sin_addr.s_addr = inet_addr(ip.c_str());
//	if(SOCKET_ERROR == connect(sock,(sockaddr*)&addr,sizeof(addr)))
//	{
//		closesocket(sock);
//		return -1;
//	}
//	unsigned char nTime[16];
//	int n = recv(sock,(char*)nTime,16,0);
//	if(n<=0)
//	{
//		closesocket(sock);
//		return -1;
//	}
//	closesocket(sock);
//	assert(4==n);
//	unsigned int dwTime = 0;
//	dwTime += nTime[0] << 24;		//��������	
//	dwTime += nTime[1] << 16;
//	dwTime += nTime[2] << 8;
//	dwTime += nTime[3];
//	dwTime -= (unsigned int)2208988800UL; //ȡ�õ���1970����ʱ�䣬ת��1900��ʱ�䣨��70�꣩
//	dwTime += 8*3600; //תΪ����ʱ��
//	*t = (time_t)dwTime;
//	return 0;
//}
////**********************************************
//bool cl_util::is_ip(const char* ip)
//{
//	unsigned int ip_n[4];
//	if(NULL!=ip && 4==sscanf(ip,"%d.%d.%d.%d",&ip_n[0],&ip_n[1],&ip_n[2],&ip_n[3]))
//		return true;
//	return false;
//}
//bool cl_util::is_dev(const char* ip)
//{
//	if(ip && ip[0]!='\0' && !is_ip(ip))
//		return true;
//	return false;
//}
//
//string cl_util::ip_explain(const char* s)
//{
//	string ip="";
//	if(NULL==s || 0==strlen(s))
//		return ip;
//	if(INADDR_NONE != inet_addr(s))
//	{
//		return s;
//	}
//	else
//	{
//		ip = cl_DIPCache::findip(s);
//		if(!ip.empty())
//			return ip;
//
//		in_addr sin_addr;
//		hostent* host = gethostbyname(s);
//		if (host == NULL) 
//		{
//			printf("gethostbyname return null\n");
//#ifdef _WIN32
//
//#else
//			printf("----cl_util::ip_explain: use ping to explain (%s) ----\n",s);
//			char cmd[1024];
//			sprintf( cmd, "ping -c 1 %s", s );
//			FILE * pd = popen( cmd , "r");
//			if ( pd != NULL )
//			{
//				if( fgets( cmd, 1000, pd ) != NULL )
//				{
//					char * s1 = strtok( cmd, "(" );
//					if ( s1 != NULL )
//					{
//						char * ip_e = strtok(NULL,")");
//						if ( is_ip(ip_e) )
//						{
//							printf("read ip from ping: %s\n", ip_e );
//							ip = ip_e;
//						}
//					}
//				}
//				pclose( pd );
//			}
//#endif
//		}
//		else
//		{
//			sin_addr.s_addr = *((unsigned long*)host->h_addr);
//			ip = inet_ntoa(sin_addr);
//		}
//		if(!ip.empty() && is_ip(ip.c_str()))
//			cl_DIPCache::addip(s,ip);
//		return ip;
//	}
//}
//typedef struct tagIPFD
//{
//	string dns;
//	string ip;
//	int is_set;
//	int del;
//	tagIPFD(void)
//	{
//		is_set = 0;
//		del = 0;
//	}
//}IPFD;
//
//#ifdef _WIN32
//DWORD WINAPI ip_explain_T(void *p)
//#else
//void *ip_explain_T(void *p)
//#endif
//{
//	IPFD *ipf = (IPFD*)p;
//	ipf->ip = cl_util::ip_explain(ipf->dns.c_str());
//	ipf->is_set = 1;
//	while(!ipf->del) 
//		Sleep(100);
//	delete ipf;
//
//#ifdef _WIN32
//	return 0;
//#else
//	return (void*)0;
//#endif
//}
//string cl_util::ip_explain_ex(const char* s,int maxTick/*=5000*/)
//{
//	if(NULL==s || 0==strlen(s))
//		return "";
//	string ip;
//	if(INADDR_NONE != inet_addr(s))
//	{
//		return s;//����ip
//	}
//	else
//	{
//		ip = cl_DIPCache::findip(s);
//		if(!ip.empty())
//			return ip;
//
//		IPFD *ipf = new IPFD();
//		if(!ipf)
//			return s;
//		ipf->dns = s;
//#ifdef _WIN32
//		DWORD thid=0;
//		HANDLE h = CreateThread(NULL,0,ip_explain_T,(void*)ipf,0,&thid);
//		if(INVALID_HANDLE_VALUE==h)
//			return s;
//		else
//			CloseHandle(h);
//#else
//		pthread_t hthread;
//		if(0==pthread_create(&hthread,NULL,ip_explain_T,(void*)ipf))
//			pthread_detach(hthread);
//		else
//			return s;
//#endif
//		Sleep(10);
//		int i=maxTick/100;
//		
//		while(1)
//		{
//			if(ipf->is_set)
//			{
//				ip = ipf->ip;
//				ipf->del = 1; 
//				return ip;
//			}
//			if(i<=0)
//				break;
//			Sleep(100);
//			i--;
//		}
//		ipf->del = 1;
//		//return s;
//		return "";
//		//inet_addr("")=0,inet_addr(NULL)=-1,inet_addr("asdfs.s322.dassadf")=-1;
//		//
//	}
//}
////****************************************************
//
//char* cl_util::ip_htoa(unsigned int ip)
//{
//	//���̰߳�ȫ
//	static char buf[32];
//	unsigned char ip_n[4];
//	ip_n[0] = ip >> 24;
//	ip_n[1] = ip >> 16;
//	ip_n[2] = ip >> 8;
//	ip_n[3] = ip;
//	sprintf(buf,"%d.%d.%d.%d",ip_n[0],ip_n[1],ip_n[2],ip_n[3]);
//	return buf;
//}
//char* cl_util::ip_ntoa(unsigned int nip)
//{
//	//���̰߳�ȫ
//	//inet_addr()ʹ��ͬһ���ڴ棬�������һ��������������������ִ����ip_ntoa����������ͬһ��
//	static char buf[32];
//	unsigned char *ip_n = (unsigned char*)&nip;
//	sprintf(buf,"%d.%d.%d.%d",ip_n[0],ip_n[1],ip_n[2],ip_n[3]);
//	return buf;
//}
//string cl_util::ip_htoas(unsigned int ip)
//{
//	return ip_htoa(ip);;
//}
//string cl_util::ip_ntoas(unsigned int nip)
//{
//	return ip_ntoa(nip);
//}
//unsigned int cl_util::ip_atoh(const char* ip)
//{
//	//atonl:inet_addr("")=0,inet_addr(NULL)=-1,inet_addr("asdfs.s322.dassadf")=-1
//	//sscanf���أ�EOF=-1Ϊ����������ʾ�ɹ���������ĸ���,ʧ�ܷ���0��-1
//	unsigned int ip_n[4]={0,0,0,0};
//	if(4!=sscanf(ip,"%d.%d.%d.%d",&ip_n[0],&ip_n[1],&ip_n[2],&ip_n[3]))
//		return 0;
//	unsigned int iip;
//	iip = ip_n[3];
//	iip += (ip_n[2] << 8);
//	iip += (ip_n[1] << 16);
//	iip += (ip_n[0] << 24);
//	return iip;
//}
//unsigned int cl_util::ip_atoh_try_explain_ex(const char* s,int maxTick/*=5000*/)
//{
//	return  ip_atoh(ip_explain_ex(s,maxTick).c_str());
//}
//
////*******************************************************
//#ifdef _WIN32	
//string cl_util::get_local_private_ip()
//{
//	string tmp;
//	hostent* he = NULL;
//	//he = gethostbyname(buf);
//	//printf("$: gethostbyname(%s) \n",buf);
//	////test:
//	he = gethostbyname(NULL);
//	if(he == NULL || he->h_addr_list[0] == 0)
//	{
//		return "";
//	}
//
//	in_addr addr;
//	int i = 0;
//	
//	// We take the first ip as default, but if we can find a better one, use it instead...
//	memcpy(&addr, he->h_addr_list[i++], sizeof addr);
//	tmp = inet_ntoa(addr);
//	if(strncmp(tmp.c_str(), "127", 3) == 0 || (!cl_util::is_private_ip(tmp) && strncmp(tmp.c_str(), "169", 3) != 0) )
//	{
//		while(he->h_addr_list[i]) 
//		{
//			memcpy(&addr, he->h_addr_list[i], sizeof addr);
//			string tmp2 = inet_ntoa(addr);
//			if(strncmp(tmp2.c_str(), "127", 3) !=0 && (cl_util::is_private_ip(tmp2) || strncmp(tmp2.c_str(), "169", 3) == 0) )
//			{
//				tmp = tmp2;
//			}
//			i++;
//		}
//	}
//	printf("$: ====Local_ip:%s \n",tmp.c_str());
//
//	return tmp;
//}
//#else
//string cl_util::get_local_private_ip()
//{
//	 int fd, num,i;
//    struct ifconf ifc;
//    struct ifreq buf[32];
//	struct ifreq *ifr;
//	sockaddr_in *sin;
//	string ip="0.0.0.0";
//	string tmp;
//
//	printf("$: //***********************//\n");
//	printf("$: //**get local private ip : \n");
//    if((fd = socket(AF_INET, SOCK_DGRAM, 0)) >= 0)
//	{
//        ifc.ifc_len = sizeof buf;
//        ifc.ifc_buf = (caddr_t) buf;
//        if(!ioctl(fd, SIOCGIFCONF, (char *) &ifc)) 
//		{
//            num = ifc.ifc_len / sizeof(struct ifreq);
//			ifr = ifc.ifc_req;
//            printf("$:interface num=%d\n", num);
//			for(i=0;i<num;++i,ifr++)
//			{
//				sin = (struct sockaddr_in*)(&ifr->ifr_addr);
//				if(0==ioctl(fd,SIOCGIFFLAGS,ifr))
//				{
//					if( !(ifr->ifr_flags & IFF_LOOPBACK) && ifr->ifr_flags & IFF_UP ) 
//					{
//						tmp = inet_ntoa(sin->sin_addr);
//						if(strncmp(tmp.c_str(), "127", 3) !=0 && (is_private_ip(tmp) || strncmp(tmp.c_str(), "169", 3) == 0) )
//						{
//							ip = tmp;
//							break;
//						}
//					}
//				}
//
//			}
//		}
//		close(fd);
//	}
//	else
//        perror("cpm: socket()");
//	printf("$: local private ip : %s \n",ip.c_str());
//	printf("$: //***********************//\n");
//	return ip;
//}
//#endif
///////////////////////////////////////////////////////////////
//typedef struct tagGPIP
//{
//	string ip;
//	int is_set;
//	int del;
//	tagGPIP(void)
//	{
//		is_set = 0;
//		del = 0;
//	}
//}GPIP;
//#ifdef _WIN32
//DWORD WINAPI get_local_private_ip_ex_T(void *p)
//#else
//void *get_local_private_ip_ex_T(void *p)
//#endif
//{
//	GPIP *ipf = (GPIP*)p;
//	ipf->ip = cl_util::get_local_private_ip();
//	ipf->is_set = 1;
//	while(!ipf->del) 
//		Sleep(100);
//	delete ipf;
//#ifdef _WIN32
//	return 0;
//#else
//	return (void*)0;
//#endif
//}
//string cl_util::get_local_private_ip_ex(int timeout_tick/*=5000*/)
//{
//	string tmp="0.0.0.0";
//
//	GPIP *ipf = new GPIP();
//		if(!ipf)
//			return tmp;
//#ifdef _WIN32
//		DWORD thid=0;
//		HANDLE h = CreateThread(NULL,0,get_local_private_ip_ex_T,(void*)ipf,0,&thid);
//		if(INVALID_HANDLE_VALUE==h)
//			return tmp;
//		else
//			CloseHandle(h);
//#elif defined(_ECOS_8203)
//		//TODO_ECOS: ��ʱ�������̴߳���
//		delete ipf;
//		tmp = get_local_private_ip();
//		return tmp;
//#else
//		pthread_t hthread;
//		if(0==pthread_create(&hthread,NULL,get_local_private_ip_ex_T,(void*)ipf))
//			pthread_detach(hthread);
//		else
//			return tmp;
//#endif
//		Sleep(10);
//		int i=timeout_tick/100;
//		while(1)
//		{
//			if(ipf->is_set)
//			{
//				tmp = ipf->ip;
//				ipf->del = 1; //��ʱ�������߳�ɾ�����ȫ
//				return tmp;
//			}
//			if(i<=0)
//				break;
//			Sleep(100);
//			i--;
//		}
//		ipf->del = 1;
//	return tmp;
//}
//
//bool cl_util::is_private_ip(string const& ip) 
//{
//	unsigned long naddr;
//
//	naddr = inet_addr(ip.c_str());
//
//	if (naddr != INADDR_NONE) {
//		unsigned long haddr = ntohl(naddr);
//		return ((haddr & 0xff000000) == 0x0a000000 || // 10.0.0.0/8
//				(haddr & 0xff000000) == 0x7f000000 || // 127.0.0.0/8
//				(haddr & 0xfff00000) == 0xac100000 || // 172.16.0.0/12
//				(haddr & 0xffff0000) == 0xc0a80000);  // 192.168.0.0/16
//	}
//	return false;
//}

//*******************************************************
int cl_util::url_element_split(const string& url,string& server,unsigned short& port,string& cgi)
{
	//����url
	string str;
	int pos = 0,pos2 = 0, pos3 = 0;
	port = 80;
	server = "";
	cgi = "";

	pos = (int)url.find("://",0);
	if(pos >= 0)
		pos += 3;
	else
	{
		pos = 0;
	}

	pos2 = (int)url.find(":",pos);
	pos3 = (int)url.find("/",pos);

	if(pos3 > 0 && pos2 > pos3)
		pos2 = -1;

	if(pos3 > pos)
	{
		if(pos2>pos)
		{
			server = url.substr(pos,pos2-pos);
			str = url.substr(pos2+1,pos3-pos2-1);
			port = atoi(str.c_str());
		}
		else
		{
			server = url.substr(pos,pos3-pos);
		}
		cgi = url.substr(pos3);
	}
	else
	{
		if(pos2>pos)
		{
			server = url.substr(pos,pos2-pos);
			str = url.substr(pos2+1);
			port = atoi(str.c_str());
		}
		else
		{
			server = url;
		};
	}

	return 0;
}
string cl_util::url_get_name(const string& url)
{
	if(url.empty())
		return "";
	int n = 0;
	string str = url;
	n = (int)str.find("?");
	if(n>=0)
		str.erase(n); //����������ܴ���/,������ȥ����������
	n = (int)str.rfind('/');
	return str.substr(n+1);
}
string cl_util::url_get_cgi(const string& url)
{
	if(url.empty())
		return "";
	int n = 0;
	string str = url;
	n = (int)str.find("://");
	if(n>=0)
	{
		str = str.substr(n+3);
		n = (int)str.find("/");
		assert(n>=0);
		if(n>0)
			str = str.substr(n);
	}
	return str;
}
string cl_util::url_get_parameter(const string& url,const string& parameter)
{
	if(url.empty() || parameter.empty())
		return "";
	int pos,plen;
	string str,str1,str2;
	str = parameter;
	str += "=";
	str1 = "?";str1 += str;
	str2 = "&";str2 += str;
	plen = parameter.length()+2;
	pos = (int)url.find(str1);
	if(pos<0)
		pos = (int)url.find(str2);
	if(pos<0)
	{
		plen = parameter.length()+1;
		pos = (int)url.find(str);
	}
	if(pos<0)
		return "";
	str = url.substr(pos + plen);

	pos = (int)str.find("&");
	if(pos >= 0)
		str = str.substr(0,pos);
	return str;
}
//��ȡURL����������url����Ϊcgi�е����һ������
string cl_util::url_get_last_parameter(const string& url,const string& parameter)
{
	string val;
	string str;
	str = parameter;
	str += "=";
	int pos = (int)url.find(str);
	if(pos>0)
	{
		val = url.substr(pos+4);
	}
	return val;
}
string cl_util::url_cgi_get_path(const string& cgi)
{
	int pos = (int)cgi.find("?");
	if(pos>=0)
		return cgi.substr(0,pos);
	return cgi;
}

//****************************************************
string cl_util::time_to_time_string(const time_t& _Time)
{
	tm *t = localtime(&_Time);
	char buf[128];
	sprintf(buf,"%02d:%02d:%02d",
		t->tm_hour,t->tm_min,t->tm_sec);
	return buf;
}
string cl_util::time_to_datetime_string(const time_t& _Time)
{
	tm *t = localtime(&_Time);
	char buf[128];
	sprintf(buf,"%d-%02d-%02d %02d:%02d:%02d",
		t->tm_year+1900,t->tm_mon+1,t->tm_mday,
		t->tm_hour,t->tm_min,t->tm_sec);
	return buf;
}
string cl_util::time_to_datetimeT_string(const time_t& _Time)
{
	tm *t = localtime(&_Time);
	char buf[128];
	sprintf(buf,"%d-%02d-%02dT%02d:%02d:%02d",
		t->tm_year+1900,t->tm_mon+1,t->tm_mday,
		t->tm_hour,t->tm_min,t->tm_sec);
	return buf;
}
string cl_util::time_to_datetime_string2(const time_t& _Time)
{
	tm *t = localtime(&_Time);
	char buf[128];
	sprintf(buf,"%d%02d%02d.%02d%02d%02d",
		t->tm_year+1900,t->tm_mon+1,t->tm_mday,
		t->tm_hour,t->tm_min,t->tm_sec);
	return buf;
}
string cl_util::time_to_date_string(const time_t& _Time)
{
	tm *t = localtime(&_Time);
	char buf[128];
	sprintf(buf,"%d-%02d-%02d",
		t->tm_year+1900,t->tm_mon+1,t->tm_mday);
	return buf;
}

//*******************************************************
int cl_util::string_array_find(int argc,char** argv,const char* str)
{
	for(int i=0;i<argc;++i)
	{
		if(0==strcmp(argv[i],str))
			return i;
	}
	return -1;
}
//*******************************************************
bool cl_util::is_write_debug_log()
{
	static int s_writelog = 2;
	if(0==s_writelog)
	{
		return false;
	}
	else if(2==s_writelog)
	{
		if(cl_util::file_exist("./debug.log"))
			s_writelog = 1;
		else
		{
			s_writelog = 0;
			return false;
		}
	}
	return true;
}
int cl_util::write_debug_log(const char *strline,const char *path)
{
	if(!is_write_debug_log())
		return -1;
	return write_log(strline,path);
}
int cl_util::write_log(const char *strline,const char *path)
{
	char *buf=new char[strlen(strline)+128];
	if(!buf)
		return -1;
	time_t tt = time(0);
	tm *t = localtime(&tt);
	sprintf(buf,"[%d-%d-%d %02d:%02d:%02d] %s\r\n",
		t->tm_year+1900,t->tm_mon+1,t->tm_mday,
		t->tm_hour,t->tm_min,t->tm_sec,strline);

	FILE *fp = fopen(path,"ab+");
	if(fp)
	{
		fwrite(buf,strlen(buf),1,fp);
		fclose(fp);
	}
	delete[] buf;
	return 0;
}
int cl_util::write_tlog(const char *path,int maxsize,const char* format,...)
{
	char *buf = new char[maxsize];
	int size = 0;
	int ret = -1;
	sprintf(buf,"[%s]: ",time_to_datetime_string(time(0)).c_str());
	va_list ap;
	va_start(ap,format);
	size = strlen(buf);
	maxsize-=(size+3); //�������кͽ�����
	size = vsnprintf(buf+size,maxsize,format,ap);
	va_end(ap);
	if(size>0 && size<maxsize)
	{
		strcat(buf,"\r\n");
		FILE *fp = fopen(path,"ab+");
		if(fp)
		{
			fwrite(buf,strlen(buf),1,fp);
			fclose(fp);
			ret = 0;
		}
	}
	delete[] buf;
	return ret;
}
int cl_util::write_buffer_to_file(const char *buf,int size,const char *path)
{
	FILE *fp = fopen(path,"wb+");
	if(fp)
	{
		fwrite(buf,size,1,fp);
		fclose(fp);
	}
	return 0;
}
int cl_util::read_buffer_from_file(char *buf,int size,const char *path)
{
	int ret=0,n=0;
	FILE *fp = fopen(path,"rb");
	if(fp)
	{
		while(size>0)
		{
			n = fread(buf,1,size,fp);
			if(n<=0)
				break;
			size -= n;
			buf += n;
			ret += n;
		}
		if(!feof(fp))
			ret = -2;
		fclose(fp);
		return ret;
	}
	return -1;
}

void cl_util::get_cpuid(unsigned int cpuinfo[4], unsigned int fn_id)
{
	unsigned int deax, debx, decx, dedx;
#ifdef WIN32
	__asm
	{
		mov eax, fn_id; ��������ֵ��eax
		cpuid; ִ��cpuidָ��
		mov deax, eax; ���Ĵ���ֵ��ֵ����ʱ����
		mov debx, ebx
		mov decx, ecx
		mov dedx, edx
	}
#elif defined(ANDROID)
	deax = debx = decx = dedx = 0;
#elif defined(__GNUC__)
	__asm__("cpuid"
		:"=a"(deax),
		"=b"(debx),
		"=c"(decx),
		"=d"(dedx)
		: "a"(fn_id));
#endif
	cpuinfo[0] = deax;
	cpuinfo[1] = debx;
	cpuinfo[2] = dedx;
	cpuinfo[3] = decx;
}


//
#if defined(__GNUC__) && !defined(ANDROID)
//sataӲ�̻�ȡSN����
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <scsi/sg.h>

#define SCSI_TIMEOUT 5000 /* ms */

int scsi_io(int fd, unsigned char *cdb, unsigned char cdb_size, int xfer_dir,
	unsigned char *data, unsigned int *data_size,
	unsigned char *sense, unsigned int *sense_len)
{
	sg_io_hdr_t io_hdr;

	memset(&io_hdr, 0, sizeof(sg_io_hdr_t));
	io_hdr.interface_id = 'S';
	/* CDB */
	io_hdr.cmdp = cdb;
	io_hdr.cmd_len = cdb_size;

	/* Where to store the sense_data, if there was an error */
	io_hdr.sbp = sense;
	io_hdr.mx_sb_len = *sense_len;
	*sense_len = 0;

	/* Transfer direction, either in or out. Linux does not yet
	support bidirectional SCSI transfers ?
	*/
	io_hdr.dxfer_direction = xfer_dir;

	/* Where to store the DATA IN/OUT from the device and how big the
	buffer is
	*/
	io_hdr.dxferp = data;
	io_hdr.dxfer_len = *data_size;

	/* SCSI timeout in ms */
	io_hdr.timeout = SCSI_TIMEOUT;

	if (ioctl(fd, SG_IO, &io_hdr) < 0) {
		perror("SG_IO ioctl failed");
		return -1;
	}
	/* now for the error processing */
	if ((io_hdr.info & SG_INFO_OK_MASK) != SG_INFO_OK) {
		if (io_hdr.sb_len_wr > 0) {
			*sense_len = io_hdr.sb_len_wr;
			return 0;
		}
	}
	if (io_hdr.masked_status) {
		printf("status=0x%x\n", io_hdr.status);
		printf("masked_status=0x%x\n", io_hdr.masked_status);
		return -2;
	}
	if (io_hdr.host_status) {
		printf("host_status=0x%x\n", io_hdr.host_status);
		return -3;
	}
	if (io_hdr.driver_status) {
		printf("driver_status=0x%x\n", io_hdr.driver_status);
		return -4;
	}
	return 0;
}

int scsi_inquiry_unit_serial_number(int fd, char* content)
{
	unsigned char cdb[] = { 0x12,0x01,0x80,0,0,0 };
	unsigned int data_size = 0x00ff;
	unsigned char data[data_size];
	unsigned int sense_len = 32;
	unsigned char sense[sense_len];
	int res, pl, i;

	cdb[3] = (data_size >> 8) & 0xff;
	cdb[4] = data_size & 0xff;
	printf("INQUIRY Unit Serial Number:\n");

	res = scsi_io(fd, cdb, sizeof(cdb), SG_DXFER_FROM_DEV, data, &data_size, sense, &sense_len);
	if (res) {
		printf("SCSI_IO failed\n");
		return -1;
	}
	if (sense_len) {
		return -1;
	}

	/* Page Length */
	pl = data[3];

	/* Unit Serial Number */
	//printf("Unit Serial Number:");
	//for(i=4;i<(pl+4);i++)printf("%c",data[i]&0xff);printf("\n");
	char t_str[1];
	for (i = 4; i<(pl + 4); i++)
	{
		//printf("%d:%c\n",data[i]&0xff,data[i]&0xff);
		sprintf(t_str, "%c", data[i] & 0xff);
		strcat(content, t_str);
	}
	//printf("pl = %d\n",pl);
	return 0;
}

int open_scsi_device(const char *dev)
{
	int fd, vers;
	if ((fd = open(dev, O_RDWR))<0) {
		printf("ERROR could not open device %s\n", dev);
		return -1;
	}
	if ((ioctl(fd, SG_GET_VERSION_NUM, &vers) < 0) || (vers < 30000)) {
		printf("/dev is not an sg device, or old sg driver\n");
		close(fd);
		return -1;
	}

	return fd;
}

int GetSCSIDevicehdr(char *device, char* content)
{
	int fd;
	fd = open_scsi_device(device);
	if (fd<0) {
		printf("Could not open SCSI device %s\n", device);
		_exit(10);
	}
	scsi_inquiry_unit_serial_number(fd, content);
	return 0;
}
#endif


