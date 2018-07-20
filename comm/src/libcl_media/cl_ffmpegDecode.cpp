#include "cl_ffmpegDecode.h"
#include <assert.h>
#include "cl_basetime.h"

#ifdef __cplusplus
extern "C" {
#endif
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#ifdef __cplusplus
}
#endif

#ifdef _WIN32
#pragma warning(disable:4996)
#pragma comment(lib,"D:/1WORK/code/comm/osrc/ffmpeg/win32/lib/avutil.lib")
#pragma comment(lib,"D:/1WORK/code/comm/osrc/ffmpeg/win32/lib/avcodec.lib")
#pragma comment(lib,"D:/1WORK/code/comm/osrc/ffmpeg/win32/lib/avformat.lib")
#pragma comment(lib,"D:/1WORK/code/comm/osrc/ffmpeg/win32/lib/swscale.lib")
#pragma comment(lib,"D:/1WORK/code/comm/osrc/ffmpeg/win32/lib/swresample.lib")
#endif


typedef struct tag_cl_ffmpeg_handle
{
	uint8_t *readbuf;
	AVIOContext *pb;
	AVInputFormat *piFmt;
	AVFormatContext *pFmt;
	
	AVCodec *pVideoCodec;
	AVCodec *pAudioCodec;
	AVCodecContext *pVideoCodecCtx;
	AVCodecContext *pAudioCodecCtx;

	int videoIndex;
	int audioIndex;

	CL_FF_OUT_VIDEO_FUNC out_video_func;
	void* out_video_func_param;
	CL_FF_OUT_AUDIO_FUNC out_audio_func;
	void* out_audio_func_param;

}cl_ffmpeg_handle_t;


cl_ffmpegDecode::cl_ffmpegDecode(void)
:m_brun(false)
,m_handle(NULL)
,m_curr_bmppts(0)
{
}


cl_ffmpegDecode::~cl_ffmpegDecode(void)
{
}
int cl_ffmpegDecode::work(int e)
{
	if(0==e)
		return thread_decode();
	else if(1==e)
		return thread_sendbitmap();
	else 
		return thread_sendpcm();
	return 0;
}

//打开解码器,会开启双线程,1线程解码,1线程按PTS传递解码数据.
int cl_ffmpegDecode::open(CL_FF_READ_FUNC read_func,void* read_func_param,
			CL_FF_OUT_VIDEO_FUNC out_video_func,void* out_video_func_param,
			CL_FF_OUT_AUDIO_FUNC out_audio_func,void* out_audio_func_param)
{
	if(m_brun)
		return 1;
	int ret = -1;
	m_curr_bmppts = 0;
	cl_ffmpeg_handle_t *h = new cl_ffmpeg_handle_t();
	memset(h,0,sizeof(cl_ffmpeg_handle_t));
	h->out_video_func = out_video_func;
	h->out_video_func_param = out_video_func_param;
	h->out_audio_func = out_audio_func;
	h->out_audio_func_param = out_audio_func_param;
	try
	{
		av_register_all();
		h->readbuf = (uint8_t*)av_malloc(10240000); //10MB
		//read_func 必须每次都有数据返回,否则初始化可能失败,而播放过程中可能花屏
		h->pb = avio_alloc_context(h->readbuf,10240000,0,read_func_param,read_func,NULL,NULL);
		if(!h->pb)
		{
			ret = -1;
			goto end;
		}
		//探测流格式
		if(0!=av_probe_input_buffer(h->pb,&h->piFmt,"",NULL,0,0))
		{
			ret = -2;
			goto end;
		}
		h->pFmt = avformat_alloc_context();
		h->pFmt->pb = h->pb;
		//打开格式信息
		if(0!=avformat_open_input(&h->pFmt, NULL, h->piFmt, NULL))
		{
			ret = -3;
			goto end;
		}
		//查找流格式
		if(avformat_find_stream_info(h->pFmt,NULL) < 0)
		{
			ret = -4;
			goto end;
		}
		//av_dump_format(pFmt, 0, "", 0);
		//find 音视频解码器
		for(int i=0;i<(int)h->pFmt->nb_streams;++i)
		{
			if(AVMEDIA_TYPE_VIDEO==h->pFmt->streams[i]->codec->codec_type)
			{
				h->videoIndex = i;
				h->pVideoCodecCtx = h->pFmt->streams[i]->codec;
				//查找视频解码器
				if(NULL==(h->pVideoCodec=avcodec_find_decoder(h->pVideoCodecCtx->codec_id)))
				{
					ret = -5;
					goto end;
				}
				if(avcodec_open2(h->pVideoCodecCtx,h->pVideoCodec,NULL)<0)
				{
					ret = -6;
					goto end;
				}
			}
			else if(AVMEDIA_TYPE_AUDIO==h->pFmt->streams[i]->codec->codec_type)
			{
				h->audioIndex = i;
				h->pAudioCodecCtx = h->pFmt->streams[i]->codec;
				//查找视频解码器
				if(NULL==(h->pAudioCodec=avcodec_find_decoder(h->pAudioCodecCtx->codec_id)))
				{
					ret = -7;
					goto end;
				}
				if(avcodec_open2(h->pAudioCodecCtx,h->pAudioCodec,NULL)<0)
				{
					ret = -8;
					goto end;
				}
			}
		}
		if(!h->pVideoCodecCtx && !h->pAudioCodecCtx)
		{
			ret = -9;
			goto end;
		}
		m_handle = h;
	}catch(...)
	{
	}
end:

	if(NULL==m_handle)
	{
		if(h->pAudioCodecCtx)
			avcodec_close(h->pAudioCodecCtx);
		if(h->pVideoCodecCtx)
			avcodec_close(h->pVideoCodecCtx);
		if(h->pFmt)
		{
			h->pFmt->pb = NULL;
			avformat_close_input(&h->pFmt);
			avformat_free_context(h->pFmt);
		}
		if(h->pb)
			av_free(h->pb);
		////前面会释放
		//if(h->readbuf)
		//	av_free(h->readbuf);
		delete h;
		return -1;
	}

	m_brun = true;
	this->activate(3);
	return 0;
}

int cl_ffmpegDecode::close()
{
	if(!m_brun)
		return 0;
	m_brun = false;
	wait();
	cl_ffmpeg_handle_t *h = (cl_ffmpeg_handle_t*)m_handle;
	if(h)
	{
		if(h->pAudioCodecCtx)
			avcodec_close(h->pAudioCodecCtx);
		if(h->pVideoCodecCtx)
			avcodec_close(h->pVideoCodecCtx);
		if(h->pFmt)
		{
			h->pFmt->pb = NULL;
			avformat_close_input(&h->pFmt);
			avformat_free_context(h->pFmt);
		}
		if(h->pb)
			av_free(h->pb);
		////前面会释放
		//if(h->readbuf)
		//	av_free(h->readbuf);
		delete h;
	}
	m_handle = NULL;
	febmp_clear();
	fepcm_clear();
	return 0;
}

void cl_ffmpegDecode::febmp_init(int w,int h,int bitsPerPixel,int origin)
{
	cl_Bitmap_t *bmp;
	cl_BitmapHeader_t bmp_header;
	int bmp_size;
	bmp_header.bitsPerPixel = bitsPerPixel;
	bmp_header.origin = origin;
	bmp_header.width = w;
	bmp_header.height = h;
	bmp_size = bmp_header.width*bmp_header.height*(bmp_header.bitsPerPixel/8);
	for(int i=0;i<200;++i)
	{
		bmp = new cl_Bitmap_t();
		bmp->inf = bmp_header;
		bmp->size = bmp_size;
		bmp->data = new unsigned char[bmp_size];
		m_febmp.put_empty_node(bmp);
	}
}
void cl_ffmpegDecode::febmp_clear()
{
	cl_Bitmap_t *bmp;
	while((bmp=m_febmp.get_full_node()))
	{
		delete[] bmp->data;
		delete bmp;
	}
	while((bmp=m_febmp.get_empty_node()))
	{
		delete[] bmp->data;
		delete bmp;
	}
}
void cl_ffmpegDecode::fepcm_init(int channels,int samplesPerSec,int bitsPerSample,int length)
{
	cl_PcmPacket_t *pcm;
	cl_PcmHeader_t h;
	h.channels = channels;
	h.samplesPerSec = samplesPerSec;
	h.bitsPerSample = bitsPerSample;
	h.nBlockAlign = channels*bitsPerSample/8;
	for(int i=0;i<1000;++i)
	{
		pcm = new cl_PcmPacket_t();
		pcm->inf = h;
		pcm->length = length;
		pcm->data = new unsigned char[pcm->length];
		pcm->size = 0;
		m_fepcm.put_empty_node(pcm);
	}
}
void cl_ffmpegDecode::fepcm_clear()
{
	cl_PcmPacket_t *pcm;
	while((pcm=m_fepcm.get_full_node()))
	{
		delete[] pcm->data;
		delete pcm;
	}
	while((pcm=m_fepcm.get_empty_node()))
	{
		delete[] pcm->data;
		delete pcm;
	}
}

int cl_ffmpegDecode::thread_decode() //解码线程
{
	cl_ffmpeg_handle_t *h = (cl_ffmpeg_handle_t*)m_handle;
	AVFrame *pFrame = av_frame_alloc();
	AVFrame *pFrameRGB = av_frame_alloc();
	AVPacket pkt;
	cl_Bitmap_t *bmp;
	int bmp_linesize[8];
	cl_PcmPacket_t *pcm;
	
	int ret;
	int got_picture=0;
	bool bvideo_first = true;
	bool baudio_first = true;
	struct SwsContext * img_convert_ctx = NULL; //视频格式转换
	struct SwrContext *au_convert_ctx = NULL;
	int audio_out_linesize = 0;
	int audio_out_buffer_size = 0;

	memset(bmp_linesize,0,sizeof(bmp_linesize));

	AVCodecContext *vi_cc,*au_cc;
	vi_cc = h->pVideoCodecCtx;
	au_cc = h->pAudioCodecCtx;

	av_init_packet(&pkt);
	while(m_brun)
	{
		if(!bvideo_first && 0==m_febmp.get_empty_size())
		{
			Sleep(10);
			continue;
		}
		if((ret=av_read_frame(h->pFmt,&pkt))<0)
		{
			av_packet_unref(&pkt);
			Sleep(10);
			continue;
		}
		if(pkt.stream_index==h->videoIndex)
		{
			ret = avcodec_decode_video2(h->pVideoCodecCtx,pFrame,&got_picture,&pkt);
			if(ret>=0 && got_picture>0)
			{
				//在此初始化因为前面可能取不到width,height
				if(bvideo_first)
				{
					bvideo_first=false;
					febmp_init(pFrame->width,pFrame->height,24,0);
					bmp_linesize[0] = pFrame->width*3;
					bmp_linesize[1] = 0;
					if(AV_PIX_FMT_RGB24!=pFrame->format)
						img_convert_ctx = sws_getContext(pFrame->width, pFrame->height,(AVPixelFormat)pFrame->format,pFrame->width, pFrame->height, AV_PIX_FMT_RGB24, SWS_BICUBIC, NULL, NULL, NULL);
				}

				bmp = m_febmp.get_empty_node();
				assert(bmp && pFrame->width==bmp->inf.width && pFrame->height==bmp->inf.height);
				if(NULL==bmp || pFrame->width!=bmp->inf.width || pFrame->height!=bmp->inf.height)
				{
					av_packet_unref(&pkt);
					if(bmp)
						m_febmp.put_empty_node(bmp);
					continue;
				}
				if(AV_PIX_FMT_RGB24!=pFrame->format)
				{
					uint8_t *tmp = pFrame->data[1];
					pFrame->data[1] = pFrame->data[2];
					pFrame->data[2] = tmp;
					ret = sws_scale(img_convert_ctx,pFrame->data,pFrame->linesize,0,pFrame->height,&bmp->data,bmp_linesize);
				}
				else
				{
					memcpy(bmp->data,pFrame->data[0],bmp->size);
				}
				bmp->pts = pFrame->pkt_pts; // 90000 /s
				m_febmp.put_full_node(bmp);
			}
		}
		else if(pkt.stream_index==h->audioIndex)
		{
			//解码一帧音频
			int frame_size = AV_CODEC_CAP_VARIABLE_FRAME_SIZE*3/2;
			ret = avcodec_decode_audio4(h->pAudioCodecCtx, pFrame, &frame_size, &pkt);
			if(ret>=0 && frame_size > 0) 
			{
				if(baudio_first)
				{
					baudio_first = false;			
					audio_out_buffer_size = av_samples_get_buffer_size(&audio_out_linesize,au_cc->channels,au_cc->frame_size,au_cc->sample_fmt,1);
					fepcm_init(au_cc->channels,au_cc->sample_rate,16,audio_out_buffer_size);
					au_convert_ctx = swr_alloc();
					au_convert_ctx=swr_alloc_set_opts(au_convert_ctx,
								AV_CH_LAYOUT_STEREO, AV_SAMPLE_FMT_S16, au_cc->sample_rate,  
								av_get_default_channel_layout(au_cc->channels),au_cc->sample_fmt , au_cc->sample_rate,0, NULL);  
					swr_init(au_convert_ctx);
				}
				pcm = m_fepcm.get_empty_node();
				if(NULL!=pcm)
				{
					pcm->pts = pFrame->pkt_pts;
					pcm->size = swr_convert(au_convert_ctx,&pcm->data,audio_out_linesize,(const uint8_t**)pFrame->data,pFrame->nb_samples);
					if(pcm->size>0)
					{
						pcm->size = pcm->size*pcm->inf.channels*2;
						m_fepcm.put_full_node(pcm);
					}
					else
						m_fepcm.put_empty_node(pcm);
				}
				else
				{
				}
			}	
		}
		av_packet_unref(&pkt);	
	}
	return 0;
}

int cl_ffmpegDecode::thread_sendbitmap() //音视频数据同步返回
{
	cl_ffmpeg_handle_t *h = (cl_ffmpeg_handle_t*)m_handle;
	DWORD begin_tick = 0;
	DWORD t1,t2;
	uint64_t begin_pts = 0;
	cl_Bitmap_t* bmp = NULL;
	while(m_brun && NULL==(bmp=m_febmp.get_full_node()))
		Sleep(10);
	if(m_brun && bmp)
	{
		begin_tick = GetTickCount();
		begin_pts = bmp->pts;
		h->out_video_func(h->out_video_func_param,bmp);
		m_febmp.put_empty_node(bmp);
	}
	else
		return 0;
	while(m_brun)
	{
		bmp = m_febmp.get_full_node();
		if(!bmp)
		{
			Sleep(10);
			continue;
		}
		t1 = GetTickCount();
		if(t1<begin_tick)
		{
			//重置时间
			begin_tick = t1;
			begin_pts = bmp->pts;
		}
		else
		{
			t1 -= begin_tick;
			t2 = (DWORD)((bmp->pts - begin_pts)/90);
			//if(t1<t2-5)
			//	Sleep(t2-t1-5);
		}
		m_curr_bmppts = bmp->pts;
		h->out_video_func(h->out_video_func_param,bmp);
		m_febmp.put_empty_node(bmp);
	}
	return 0;
}
int cl_ffmpegDecode::thread_sendpcm()
{
	cl_ffmpeg_handle_t *h = (cl_ffmpeg_handle_t*)m_handle;
	DWORD begin_tick = 0;
	DWORD t1,t2;
	uint64_t begin_pts = 0;
	cl_PcmPacket_t* pcm = NULL;
	char logbuf[1024];
	while(m_brun && NULL==(pcm=m_fepcm.get_full_node()))
		Sleep(10);
	if(m_brun && pcm)
	{
		begin_tick = GetTickCount();
		begin_pts = pcm->pts;
		h->out_audio_func(h->out_audio_func_param,pcm);
		m_fepcm.put_empty_node(pcm);
	}
	else
		return 0;
	while(m_brun)
	{
		pcm = m_fepcm.get_full_node();
		if(!pcm)
		{
			Sleep(10);
			continue;
		}
		t1 = GetTickCount();
		if(t1<begin_tick)
		{
			//重置时间
			begin_tick = t1;
			begin_pts = pcm->pts;
		}
		else
		{
			t1 -= begin_tick;
			t2 = (DWORD)((pcm->pts - begin_pts)/90);
			//if(t1<t2-5)
			//	Sleep(t2-t1-5);
		}
		//100毫秒以内数据播放
		if(pcm->pts+(100*90)>m_curr_bmppts)
			h->out_audio_func(h->out_audio_func_param,pcm);
		else
		{
			//slow
			t1 = (DWORD)((m_curr_bmppts - pcm->pts)/90);
			sprintf(logbuf,"#: pcm slow(tick=%d) \n",t1);
			OutputDebugStringA(logbuf);
		}
		m_fepcm.put_empty_node(pcm);
	}
	return 0;
}


