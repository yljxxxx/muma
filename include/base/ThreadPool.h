#ifndef __T__MUMA_THREAD_POOL_H__
#define __T__MUMA_THREAD_POOL_H__

#include "Thread.h"
#include "Mutex.h"

#include <functional>
#include <memory>
#include <deque>
#include <vector>

namespace muma
{

	class ThreadPool
	{
	public:
		typedef std::function<void()> Task;

		explicit ThreadPool();
		~ThreadPool();

		int start(int thread_nums);
		void stop();

		void execute_task(const Task& task);

		size_t queueSize();

	private:
		void workThread();
		Task wait_take_task();

	private:
		bool _running;
		muma::MutexLock _mutex;
		muma::Condition _cond;

		std::vector<std::shared_ptr<muma::Thread>> _threads; 
		std::deque<Task> _queue;

		
	};

}



#endif