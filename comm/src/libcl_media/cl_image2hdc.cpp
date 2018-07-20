#include "cl_image2hdc.h"


#ifdef _WIN32

int cl_bitmap2hdc(void* bmp,int w,int h,int bitsPerPixel,int origin,HDC hdc,int dcw,int dch)
{
	unsigned buffer[sizeof(BITMAPINFOHEADER) + 1024];
	BITMAPINFO* bmi = (BITMAPINFO*)buffer;
	BITMAPINFOHEADER& bh = bmi->bmiHeader;
	memset(&bmi->bmiHeader,0,sizeof(BITMAPINFOHEADER));
	bh.biSize = sizeof(BITMAPINFOHEADER);
	bh.biWidth = w;
	bh.biHeight = origin?abs(h):-abs(h);
	bh.biPlanes = 1;
	bh.biBitCount = (WORD)bitsPerPixel;
	bh.biClrImportant = BI_RGB;
	memset(bmi->bmiColors,0,sizeof(bmi->bmiColors));
	if(w==dcw && h==dch)
	{
		SetDIBitsToDevice(hdc,0,0,dcw,dch,0,0,0,w*(bitsPerPixel/8),bmp,bmi,DIB_RGB_COLORS);
		return 0;
	}
	else if(w>dcw)
	{
		SetStretchBltMode(hdc,HALFTONE);
	}
	else
	{
		SetStretchBltMode(hdc,COLORONCOLOR);
	}
	StretchDIBits(hdc,0,0,dcw,dch,0,0,w,h,bmp,bmi,DIB_RGB_COLORS, SRCCOPY);
	return 0;
}

#endif


