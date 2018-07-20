#pragma once
#ifdef _WIN32

#include <Windows.h>
#include <mmsystem.h>

//PCM waveheader ���ݻ������,����>1 ��Ȼ���Ż���
#define CL_PCM_WH_SIZE 3
class cl_pcmrenderWin
{
public:
	cl_pcmrenderWin(void);
	~cl_pcmrenderWin(void);


public:
	int open(int channels,int bitsPerSample,int samplePerSec);
	void close();
	int write_pcm(void* data,int size);
	bool is_open()const {return m_bopen;}

private:
	bool m_bopen;
	HWAVEOUT m_hWaveOut;

	WAVEHDR m_wh[CL_PCM_WH_SIZE];
	int m_i;
};

#endif

