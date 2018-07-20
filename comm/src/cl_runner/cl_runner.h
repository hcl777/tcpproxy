#pragma once
#include "cl_thread.h"
#include "cl_basetypes.h"

class cl_runner : public cl_thread
{
public:
	cl_runner(void);
	virtual ~cl_runner(void);

public:
	int run();
	void end();
	virtual int work(int e);

	static int stop();
private:
	bool	m_brun;
	list<string> m_ls;
};

