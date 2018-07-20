#pragma once

//********************* cl_Bitmap **********************
typedef struct tag_cl_BitmapHeader
{
	int	width; //宽度,一行大小:linesize=step=width*(bitsPerPixel/8)
	int height;
	int bitsPerPixel; //像素位数,如32位真彩色
	int origin; //原点位置,0=左上角,1=左下角
}cl_BitmapHeader_t;

typedef struct tag_cl_Bitmap
{
	cl_BitmapHeader_t inf;
	unsigned char* data;
	int size; //其实固定=width*height*(bitsPerPixel/8)
	unsigned long long pts;
}cl_Bitmap_t;
//*********************************************************

//********************* cl_PcmPacket **********************
typedef struct tag_cl_PcmHeader
{
	int channels;		//声道数.2
	int samplesPerSec;	//每秒采样数rate,44100/48000
	int	bitsPerSample;	//每个采样的位数.16
	int nBlockAlign;	//block size of data,一个block指channels*bitsPerSample/8. 如2声道16位=4.
}cl_PcmHeader_t;
typedef struct tag_cl_PcmPacket
{
	cl_PcmHeader_t inf;
	unsigned char* data;
	int length; //分配的内存长度
	int size;
	unsigned long long pts;
}cl_PcmPacket_t;
//*********************************************************


