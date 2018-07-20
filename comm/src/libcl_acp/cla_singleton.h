#pragma once

//如考虑使用cl_timer等.避免与其它应用使用同一实例.而另外建一个singleton

template<class TYPE>
class cla_singleton
{
public:
	cla_singleton(void){}
	virtual ~cla_singleton(void){}
private:
	static TYPE* _instance;

public:
	static TYPE* instance()
	{
		if(0==_instance)
		{
			_instance = new TYPE();
		}
		return _instance;
	}
	static void instance(TYPE *ins)
	{
		destroy();
		_instance = ins;
	}
	static void destroy()
	{
		if(_instance)
		{
			delete _instance;
			_instance = 0;
		}
	}

private:
	cla_singleton(const cla_singleton&);
	cla_singleton& operator=(const cla_singleton&);

};
template<class TYPE> TYPE* cla_singleton<TYPE>::_instance = 0;
