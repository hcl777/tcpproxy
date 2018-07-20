#pragma once

#if !defined(ANDROID)
//FD_SETSIZE 决定 fd_set 的数组大小，即最大select数目，windows在winsock2.h中定义
#ifdef FD_SETSIZE
	#undef FD_SETSIZE
#endif
#define FD_SETSIZE 1024    
#endif


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <time.h>

#include "cl_incnet.h"
#include "cl_basetypes.h"
#include "cl_singleton.h"
#include "cl_speaker.h"
#include "cl_thread.h"

