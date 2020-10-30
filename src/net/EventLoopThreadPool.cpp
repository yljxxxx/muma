#include "..\common.h"
#include "../../include/net/EventLoopThreadPool.h"


namespace muma
{


EventLoopThreadPool::EventLoopThreadPool(void) : 
_next(0)
{
}

EventLoopThreadPool::~EventLoopThreadPool(void)
{
	std::vector<EventLoopThread*>::iterator iter = _threads.begin();
	for(; iter != _threads.end(); ++iter)
	{
		EventLoopThread* thread = *iter;
		delete thread;
	}
}


void EventLoopThreadPool::run(int numThreads, const ThreadInitCallback& cb)
{
	for(int i=0; i<numThreads; i++)
	{
		EventLoopThread* thread = new EventLoopThread(cb);
		_threads.push_back(thread);
		_loops.push_back(thread->startLoop());
	}
}

//fixme 添加新线程后，getNextLoop逻辑需要变更
EventLoop* EventLoopThreadPool::addOneThread(const ThreadInitCallback& cb)
{
	EventLoopThread* thread = new EventLoopThread(cb);
	_threads.push_back(thread);

	EventLoop* eventLoop = thread->startLoop();
	_loops.push_back(eventLoop);

	return eventLoop;
}

EventLoop* EventLoopThreadPool::getNextLoop()
{
	EventLoop* loop = NULL;

	if(!_loops.empty())
	{
		loop = _loops[_next];
		++_next;

		if(_next >= _loops.size())
			_next = 0;
	}
		
	return loop;
	
}

const std::vector<EventLoop*>& EventLoopThreadPool::getAllEventLoops()
{
	return _loops;
}

}
