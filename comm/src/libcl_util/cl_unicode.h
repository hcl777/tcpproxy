#pragma once
#include "cl_basetypes.h"

/*
˵����
linux ʹ��wcstombs() �� mbstowcs() ʵ�ֶ��ֽ���unicode�Ļ�ת��
ʹ�������������뱾��setlocale() �ַ����йء���������gb2312,��wcstombs��ʾתΪGB2312.
*/

//cl_unicode
class cl_unicode
{
public:
	static void setlocal_gb2312();

	static void UTF_8ToUnicode(wchar_t* pOut,const char *pText);  // ��UTF-8ת����Unicode  

	static void UnicodeToUTF_8(char* pOut,wchar_t* pText);  //Unicode ת����UTF-8  

	static void UnicodeToGB2312(char* pOut,wchar_t uData);  // ��Unicode ת���� GB2312    

	static void Gb2312ToUnicode(wchar_t* pOut,const char *gbBuffer);// GB2312 ת���ɡ�Unicode  

	static void GB2312ToUTF_8(string& pOut,const char *pText, int pLen);//GB2312 תΪ UTF-8  

	static void UTF_8ToGB2312(string& pOut,const char *pText, int pLen);//UTF-8 תΪ GB2312 
};
