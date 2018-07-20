#pragma once
#include "cl_feList.h"
#include "cl_ffmpegDecode.h"
#include "cl_sourceHttp.h"
#include "cl_pcmrenderWin.h"

class cl_ffmpegPlayWin
{
public:
	cl_ffmpegPlayWin(void);
	~cl_ffmpegPlayWin(void);
	typedef struct tag_memblock
	{
		char data[4096];
		int pos;
		int size;
	}memblock_t;
public:
	int open(const char* url,HDC hDC,int dc_w,int dc_h);
	int close();
	bool isOpen() const {return m_bopen;}

public:
	static int s_on_http_data(void* param,const char* buf,int size);
	static int s_on_ffmpeg_read(void* param,unsigned char* buf,int size);
	static int s_on_ffmpeg_out_bmp(void* param,cl_Bitmap_t* bmp);
	static int s_on_ffmpeg_out_pcm(void* param,cl_PcmPacket_t* pcm);

	int on_http_data(const char* buf,int size);
	int on_ffmpeg_read(unsigned char* buf,int size);
	int on_ffmpeg_out_bmp(cl_Bitmap_t* bmp);
	int on_ffmpeg_out_pcm(cl_PcmPacket_t* pcm);

private:
	bool m_bopen;
	cl_ffmpegDecode m_decode;
	cl_sourceHttp m_source;
	cl_pcmrenderWin m_pcmrender;
	char m_url[1024];
	HDC m_hDC;
	int m_dc_w,m_dc_h;
	cl_feList<memblock_t*> m_fe;
	memblock_t* m_mb;
	memblock_t* m_mb_ffread;

};

