#pragma once
#include "cl_thread.h"
#include "cl_mediatypes.h"
#include "cl_feList.h"

typedef int (*CL_FF_READ_FUNC)(void* param,unsigned char* buf,int size);
typedef int (*CL_FF_OUT_VIDEO_FUNC)(void* param,cl_Bitmap_t* bmp);
typedef int (*CL_FF_OUT_AUDIO_FUNC)(void* param,cl_PcmPacket_t* pcm);

class cl_ffmpegDecode : public cl_thread
{
public:
	cl_ffmpegDecode(void);
	virtual ~cl_ffmpegDecode(void);
public:
	virtual int work(int e);
	//打开解码器,会开启双线程,1线程解码,1线程按PTS传递解码数据.
	int open(CL_FF_READ_FUNC read_func,void* read_func_param,
			CL_FF_OUT_VIDEO_FUNC out_video_func,void* out_video_func_param,
			CL_FF_OUT_AUDIO_FUNC out_audio_func,void* out_audio_func_param);
	int close();
	bool is_open()const {return m_brun;}
private:
	void febmp_init(int w,int h,int bitsPerPixel,int origin);
	void febmp_clear();
	void fepcm_init(int channels,int samplesPerSec,int bitsPerSample,int length);
	void fepcm_clear();

	
	int thread_decode(); //解码线程
	int thread_sendbitmap(); //音视频数据同步返回
	int thread_sendpcm();
private:
	bool m_brun;
	void* m_handle;
	cl_feList<cl_Bitmap_t*> m_febmp;
	cl_feList<cl_PcmPacket_t*> m_fepcm;
	unsigned long long m_curr_bmppts;
};

