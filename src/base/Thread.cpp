#include "../common.h"
#include "../../include/base/Thread.h"

#include <process.h> 

namespace muma
{
	Thread::Thread(const ThreadFunc& func)
		: _func(func),
		_handle(NULL),
		_id(0),
		_start(false)
	{
	}

	Thread::~Thread()
	{
		if(_handle != NULL)
			CloseHandle(_handle);
	}

	int Thread::start()
	{
		assert(!_start);

		_handle = (HANDLE)_beginthreadex(NULL, 0, &Thread::entry, (void*)this, 0, &_id);
		if(_handle == NULL)
			return -1;

		_start = true;
		return 0;
	}

	int Thread::join()
	{
		assert(_start);

		if(_handle != NULL)
		{
			WaitForSingleObject(_handle, INFINITE);
			CloseHandle(_handle);
			_handle = NULL;
			
		}

		_start = false;
		_id = 0;
		return 0;
	}


	//线程入口函数
	unsigned int _stdcall Thread::entry(void* obj)
	{
		Thread* thread = static_cast<Thread*>(obj);
		thread->_func();

		return 0;
	}

}


