#pragma once

#ifndef __T__MUMA_EVENTLOOPTHREAD_H__
#define __T__MUMA_EVENTLOOPTHREAD_H__

#include "../../include/net/EventLoop.h"
#include "../../include/base/Thread.h"

namespace muma
{


class EventLoopThread
{
public:
	typedef std::function<void(EventLoop*)> ThreadInitCallback;

	EventLoopThread(const ThreadInitCallback& cb = ThreadInitCallback());
	~EventLoopThread();

	EventLoop* startLoop();
	EventLoop* getEventLoop();

private:
	void threadFunc();

private:
	EventLoop* _loop;
	bool _isRunning;

	Thread _thread;
	HANDLE _event;

	ThreadInitCallback _callback;
};


}
#endif
