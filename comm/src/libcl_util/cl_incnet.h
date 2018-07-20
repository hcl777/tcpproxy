#pragma once

#ifndef CL_INCNET_H
#define CL_INCNET_H

//#if !defined(ANDROID)
////FD_SETSIZE ���� fd_set �������С�������select��Ŀ��windows��winsock2.h�ж���
//#ifdef FD_SETSIZE
//	#undef FD_SETSIZE
//#endif
//#define FD_SETSIZE 1024    
//#endif

#ifdef _WIN32
	#include <winsock2.h>
	typedef int socklen_t;
#else
	#include <unistd.h>
	#include <sys/time.h>
	#include <sys/types.h>
	#include <sys/stat.h>
	#include <sys/socket.h>
	#include <sys/select.h>
#ifndef NO_EPOLL
	#include <sys/epoll.h>
#endif
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <fcntl.h>
	#include <netdb.h>
	#include <net/if_arp.h>  
	#include <net/if.h>
	#include <sys/ioctl.h> 
	#include <errno.h>
	//#include <dirent.h>

	//******************************
	//������win ��ͬ�ĺ�
	typedef int SOCKET;
	#define INVALID_SOCKET -1
	#define SOCKET_ERROR -1
	#define closesocket(s) close(s)

#endif

enum {CL_DISCONNECTED=0,CL_CONNECTING=1,CL_CONNECTED=2};

#endif
