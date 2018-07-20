#pragma once

#include "cl_rlist.h"
#include "cl_synchro.h"


typedef struct tagMessage
{
	int		cmd;
	void*	data;
	cl_Semaphore*  psem; //�����sendmsg��ִ���꣬��ֵ
	tagMessage(void):cmd(0),data(NULL),psem(0) {}
	tagMessage(int aCmd, void* aData)
		:cmd(aCmd),data(aData),psem(NULL)
	{}
	~tagMessage(void){ if(psem) psem->signal();} //����Ϣ��ɾ��ʱsendmsg�����ŷ���
}cl_Message;

class cl_MessageQueue
{
public:
	cl_MessageQueue(void);
	~cl_MessageQueue(void);

	typedef struct tagMessageNode
	{
		cl_Message *msg;
		rlist_head_t leaf;
		tagMessageNode(cl_Message *aMsg) : msg(aMsg) {}
	}MessageNode;

	typedef cl_SimpleMutex Mutex;
	typedef cl_TLock<Mutex> Lock;

public:
	cl_Message* GetMessage(unsigned int millsec=INFINITE);
	void AddMessage(cl_Message *msg);
	void SendMessage(cl_Message *msg); //����Ϣ�������ұ�ɾ���ٷ���
	void ClearMessage(void (*CLEAR_MESSAGE_FUNC)(cl_Message*) = 0);
	unsigned int Size()const {return m_size;}
private:
	rlist_head_t	m_head;
	int				m_size;
	cl_Semaphore	m_sem;
	Mutex			m_mt;
};

