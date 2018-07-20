#include "cl_ffmpegPlayWin.h"
#include <assert.h>
#include "cl_image2hdc.h"

cl_ffmpegPlayWin::cl_ffmpegPlayWin(void)
	:m_bopen(false)
	,m_mb(NULL)
	,m_mb_ffread(NULL)
{
}


cl_ffmpegPlayWin::~cl_ffmpegPlayWin(void)
{
	close();
}
int cl_ffmpegPlayWin::open(const char* url,HDC hDC,int dc_w,int dc_h)
{
	close();
	m_hDC = hDC;
	m_dc_w = dc_w;
	m_dc_h = dc_h;
	strcpy(m_url,url);
	if(m_fe.empty())
	{
		memblock_t *mb;
		for(int i=0;i<5000;++i)
		{
			mb = new memblock_t();
			m_fe.put_empty_node(mb);
		}
	}
	
	if(0!=m_decode.open(s_on_ffmpeg_read,this,s_on_ffmpeg_out_bmp,this,s_on_ffmpeg_out_pcm,this))
	{
		close();
		return -1;
	}
	m_bopen = true;
	return 0;
}

int cl_ffmpegPlayWin::close()
{	
	m_bopen = false;
	m_decode.close();
	m_source.close();
	m_pcmrender.close();

	memblock_t* mb;
	while(NULL!=(mb=m_fe.get_full_node()))
		delete mb;
	while(NULL!=(mb=m_fe.get_empty_node()))
		delete mb;
	if(m_mb)
	{
		delete m_mb;
		m_mb = NULL;
	}
	if(m_mb_ffread)
	{
		delete m_mb_ffread;
		m_mb_ffread = NULL;
	}
	return 0;
}
int cl_ffmpegPlayWin::s_on_http_data(void* param,const char* buf,int size)
{
	return ((cl_ffmpegPlayWin*)param)->on_http_data(buf,size);
}
int cl_ffmpegPlayWin::s_on_ffmpeg_read(void* param,unsigned char* buf,int size)
{
	return ((cl_ffmpegPlayWin*)param)->on_ffmpeg_read(buf,size);
}
int cl_ffmpegPlayWin::s_on_ffmpeg_out_bmp(void* param,cl_Bitmap_t* bmp)
{
	return ((cl_ffmpegPlayWin*)param)->on_ffmpeg_out_bmp(bmp);
}
int cl_ffmpegPlayWin::s_on_ffmpeg_out_pcm(void* param,cl_PcmPacket_t* pcm)
{
	return ((cl_ffmpegPlayWin*)param)->on_ffmpeg_out_pcm(pcm);
}

//*************************
int cl_ffmpegPlayWin::on_http_data(const char* buf,int size)
{
	
	int pos=0;
	int n = 0;
	while(pos<size)
	{
		if(NULL==m_mb)
		{
			m_mb = m_fe.get_empty_node();
			m_mb->size = 0;
			m_mb->pos = 0;
		}
		if(NULL==m_mb)
		{
			assert(false);
			break;
		}
		n = size-pos;
		if(n> 4096-m_mb->size)
			n = 4096-m_mb->size;
		memcpy(m_mb->data+m_mb->size,buf+pos,n);
		pos+=n;
		m_mb->size+=n;
		if(m_mb->size>=4096)
		{
			m_fe.put_full_node(m_mb);
			m_mb = NULL;
		}
	}
	return 0;
}
int cl_ffmpegPlayWin::on_ffmpeg_read(unsigned char* buf,int size)
{
	int copy_size = 0;
	int count = 0;
	int n = 0;
	memblock_t *mb = NULL;

	if(!m_source.is_open())
	{
		m_source.open(m_url,s_on_http_data,this);
		while(NULL==(m_mb_ffread = m_fe.get_full_node()))
		{
			//初始化最多等5秒
			if(count++>500)
				break;
			Sleep(10);
		}
	}
	while(copy_size<size)
	{
		if(NULL==m_mb_ffread)
			m_mb_ffread = m_fe.get_full_node();
		if(NULL==m_mb_ffread)
		{
			if(copy_size>0)
				break;
			else
			{
				//最多等100
				if(count++>20)
					break;
				Sleep(10);
				continue;
			}
		}
		n = size-copy_size;
		if(n>m_mb_ffread->size)
			n = m_mb_ffread->size;
		memcpy(buf+copy_size,m_mb_ffread->data+m_mb_ffread->pos,n);
		m_mb_ffread->pos += n;
		m_mb_ffread->size -= n;
		copy_size += n;
		if(0==m_mb_ffread->size)
		{
			m_fe.put_empty_node(m_mb_ffread);
			m_mb_ffread = NULL;
		}
	}
	return copy_size;
}
int cl_ffmpegPlayWin::on_ffmpeg_out_bmp(cl_Bitmap_t* bmp)
{
	cl_bitmap2hdc(bmp->data,bmp->inf.width,bmp->inf.height,bmp->inf.bitsPerPixel,bmp->inf.origin,m_hDC,m_dc_w,m_dc_h);
	return 0;
}
int cl_ffmpegPlayWin::on_ffmpeg_out_pcm(cl_PcmPacket_t* pcm)
{
	if(!m_pcmrender.is_open())
		m_pcmrender.open(pcm->inf.channels,pcm->inf.bitsPerSample,pcm->inf.samplesPerSec);
	m_pcmrender.write_pcm(pcm->data,pcm->size);
	return 0;
}

