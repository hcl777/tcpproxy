#pragma once

//********************* cl_Bitmap **********************
typedef struct tag_cl_BitmapHeader
{
	int	width; //���,һ�д�С:linesize=step=width*(bitsPerPixel/8)
	int height;
	int bitsPerPixel; //����λ��,��32λ���ɫ
	int origin; //ԭ��λ��,0=���Ͻ�,1=���½�
}cl_BitmapHeader_t;

typedef struct tag_cl_Bitmap
{
	cl_BitmapHeader_t inf;
	unsigned char* data;
	int size; //��ʵ�̶�=width*height*(bitsPerPixel/8)
	unsigned long long pts;
}cl_Bitmap_t;
//*********************************************************

//********************* cl_PcmPacket **********************
typedef struct tag_cl_PcmHeader
{
	int channels;		//������.2
	int samplesPerSec;	//ÿ�������rate,44100/48000
	int	bitsPerSample;	//ÿ��������λ��.16
	int nBlockAlign;	//block size of data,һ��blockָchannels*bitsPerSample/8. ��2����16λ=4.
}cl_PcmHeader_t;
typedef struct tag_cl_PcmPacket
{
	cl_PcmHeader_t inf;
	unsigned char* data;
	int length; //������ڴ泤��
	int size;
	unsigned long long pts;
}cl_PcmPacket_t;
//*********************************************************


