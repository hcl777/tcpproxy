#include "cl_pcmrenderWin.h"
#ifdef _WIN32
cl_pcmrenderWin::cl_pcmrenderWin(void)
	:m_bopen(false)
	,m_hWaveOut(NULL)
{
	
}


cl_pcmrenderWin::~cl_pcmrenderWin(void)
{
}
int cl_pcmrenderWin::open(int channels,int bitsPerSample,int samplePerSec)
{
	if(m_bopen)
		return 1;

	m_i = 0;
	for(int i=0;i<CL_PCM_WH_SIZE;++i)
	{
		memset(&m_wh[i],0,sizeof(WAVEHDR));
		m_wh[i].dwFlags |= WHDR_DONE; //表示buffer已经播放完,可以释放
	}

	WAVEFORMATEX fm;
	memset(&fm,0,sizeof(fm));
	fm.wFormatTag = WAVE_FORMAT_PCM;
	fm.nChannels = channels;
	fm.wBitsPerSample = bitsPerSample;
	fm.nSamplesPerSec = (DWORD)(samplePerSec);//*0.979
	fm.nBlockAlign = fm.nChannels * fm.wBitsPerSample / 8;
	fm.nAvgBytesPerSec = (DWORD)(fm.nSamplesPerSec * fm.nBlockAlign );
	fm.cbSize = 0; //额外信息大小,添加在WAVEFORMATEX尾部

	MMRESULT ret = waveOutOpen(NULL,WAVE_MAPPER,&fm,NULL,NULL,WAVE_FORMAT_QUERY);
	if(MMSYSERR_NOERROR != ret) 
	{
		return -1;
	}
    
	// 第二步: 获取WAVEOUT句柄
	ret = waveOutOpen(&m_hWaveOut, WAVE_MAPPER, &fm,0, 0, CALLBACK_NULL);
	if(MMSYSERR_NOERROR != ret) 
	{
		return -1;
	}
	m_bopen = true;

	return 0;
}
void cl_pcmrenderWin::close()
{
	if(m_bopen)
	{
		m_bopen = false;
		while(WAVERR_STILLPLAYING==::waveOutClose(m_hWaveOut))
			waveOutReset(m_hWaveOut);
	}
}
int cl_pcmrenderWin::write_pcm(void* data,int size)
{
	
	while(0==(m_wh[m_i].dwFlags&WHDR_DONE))
		Sleep(8);
	WAVEHDR *pwh = &m_wh[m_i];
	m_i = (m_i+1)%CL_PCM_WH_SIZE;
	while(WAVERR_STILLPLAYING==waveOutUnprepareHeader(m_hWaveOut,pwh,sizeof(WAVEHDR)))
		Sleep(0);

	memset(pwh,0,sizeof(WAVEHDR));
	pwh->lpData = (LPSTR)data;
	pwh->dwBufferLength = size;
	MMRESULT ret = 0;
	if(m_bopen)
	{
		waveOutPrepareHeader(m_hWaveOut, pwh, sizeof (WAVEHDR)) ;
		ret = waveOutWrite(m_hWaveOut,pwh,sizeof(WAVEHDR));
	}
	return 0;
}

#endif


