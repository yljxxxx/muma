#include "..\common.h"
#include "../../include/net/EventLoopThread.h"

namespace muma
{


EventLoopThread::EventLoopThread(const ThreadInitCallback& cb)
  : _loop(NULL),
    _isRunning(false),
    _thread(std::bind(&EventLoopThread::threadFunc, this)),
	_event(::CreateEvent(NULL, TRUE, FALSE, NULL)),
    _callback(cb)
{
}

EventLoopThread::~EventLoopThread()
{
	if (!_isRunning)
	{
		::CloseHandle(_event);
		return;
	}

	_loop->quit();
	_thread.join();
	::CloseHandle(_event);
}

EventLoop* EventLoopThread::startLoop()
{
	if(_isRunning)
		return NULL;

	assert(!_thread.started());
	_thread.start();

	::WaitForSingleObject(_event, INFINITE);
	_isRunning = true;

	return _loop;
}

EventLoop* EventLoopThread::getEventLoop()
{
	return _loop;
}

void EventLoopThread::threadFunc()
{
	EventLoop loop;

	if (_callback)
	{
		_callback(&loop);
	}

	_loop = &loop;

	::SetEvent(_event);

	loop.loop();
}

}