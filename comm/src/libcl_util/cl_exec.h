#pragma once

namespace cl
{
//cmd��ʽ: exe p1 p2 ...; ����exe Ϊ��0������.

//��ȡ�����в�������
int cle_cmd_argc(const char* cmd);

//��ȡ�����е�i����������ʼλ�úͳ���,begָ��cmd�ڴ�,��������ڴ�
int cle_cmd_argv_i(const char* cmd,int i,const char** pbeg,int* length);


//�˽ӿڻ�����ֱ��cmdִ�����
int cle_system(const char* cmd);

int cle_shell(const char* cmd);

};

