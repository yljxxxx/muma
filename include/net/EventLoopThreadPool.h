#ifndef __T__MUMA_EVENTLOOPTHREADPOOL_H__
#define __T__MUMA_EVENTLOOPTHREADPOOL_H__

#include "../../include/net/EventLoopThread.h"
#include <vector>

namespace muma
{


class EventLoopThreadPool
{
public:
	typedef std::function<void(EventLoop*)> ThreadInitCallback;

	EventLoopThreadPool(void);
	~EventLoopThreadPool(void);

	void run(int numThreads, const ThreadInitCallback& cb = ThreadInitCallback());
	EventLoop* addOneThread(const ThreadInitCallback& cb = ThreadInitCallback());

	unsigned long getThreadNum(){ return _threads.size(); }

	EventLoop* getNextLoop();
	const std::vector<EventLoop*>& getAllEventLoops();
	
	

private:
	std::vector<EventLoopThread*> _threads;
	std::vector<EventLoop*> _loops;

	unsigned int _next;
};


}

#endif