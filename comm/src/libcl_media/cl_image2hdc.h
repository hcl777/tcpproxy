#pragma once

#ifdef _WIN32
#include <WTypes.h>

int cl_bitmap2hdc(void* bmp,int w,int h,int bitsPerPixel,int origin,HDC hdc,int dcw,int dch);

#endif



