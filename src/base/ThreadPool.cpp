#include "..\common.h"
#include "../../include/base/ThreadPool.h"

#include <algorithm>

namespace muma
{
	ThreadPool::ThreadPool()
		: _running(false)
	{
	}

	ThreadPool::~ThreadPool()
	{
		if (_running)
		{
			stop();
		}
	}

	int ThreadPool::start(int thread_nums)
	{
		if(!_threads.empty())
			return -1;

		_running = true;

		_threads.reserve(thread_nums);
		for (int i = 0; i < thread_nums; ++i)
		{
			std::shared_ptr<muma::Thread> thread(new muma::Thread(std::bind(&ThreadPool::workThread, this)));
			_threads.push_back(thread);
			thread->start();
		}

		return 0;
	}

	void ThreadPool::stop()
	{
		_running = false;
		
		_mutex.lock();
		_queue.clear();
		_mutex.unlock();

		_cond.broadcast();

		for_each(_threads.begin(),
			_threads.end(),
			std::bind(&muma::Thread::join, std::placeholders::_1));

		_threads.clear();
	}

	void ThreadPool::execute_task(const Task& task)
	{
		_mutex.lock();
		_queue.push_back(task);
		_mutex.unlock();

		_cond.signal();
	}

	size_t ThreadPool::queueSize() {
		MutexLockGuard lock(_mutex);
		return _queue.size();
	}


	ThreadPool::Task ThreadPool::wait_take_task()
	{
		_cond.wait();

		MutexLockGuard lock(_mutex);
		Task task;
		if (!_queue.empty())
		{
			task = _queue.front();
			_queue.pop_front();
		}
		else if(_running)
		{
			//exception, please add log
		}

		return task;
	}


	void ThreadPool::workThread()
	{
		try
		{
			while (_running)
			{
				Task task = wait_take_task();
				if (task)
				{
					task();
				}
			}
		}catch (...)
		{
			throw;
		}
	}


}


