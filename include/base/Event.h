#ifndef __T_MUMA_EVENT_H__
#define __T_MUMA_EVENT_H__

#include <Windows.h>

namespace muma
{

class Event
{
public:
	explicit Event()
	{ 
		_handle = CreateEvent(NULL, TRUE, FALSE, NULL);
	}
	~Event()
	{
		CloseHandle(_handle);
	}

	bool Wait(unsigned long milliseconds = INFINITE)
	{
		return (WAIT_OBJECT_0 == WaitForSingleObject(_handle, milliseconds));
	}

	void Set()
	{
		SetEvent(_handle);
	}

	void Reset()
	{
		ResetEvent(_handle);
	}

private:
	HANDLE _handle;
};


}//namespace custom


#endif