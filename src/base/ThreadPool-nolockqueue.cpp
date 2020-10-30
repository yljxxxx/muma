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
		assert(_threads.empty());

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
		//_mutex.lock();
		_running = false;
		//_cond.broadcast();
		//_mutex.unlock();

		for_each(_threads.begin(),
			_threads.end(),
			std::bind(&muma::Thread::join, std::placeholders::_1));
	}

	size_t ThreadPool::queueSize() {
		return _queue.size_approx();
	}

	void ThreadPool::execute_task(const Task& task)
	{
		while (!_queue.enqueue(task));
		//_cond.signal();
		/*_mutex.lock();
		_queue.push_back(task);
		_cond.signal();
		_mutex.unlock();*/
	}

	ThreadPool::Task ThreadPool::take()
	{
		Task task;

		//while (_queue.size_approx() == 0 && _running)
		{
		//	_cond.wait();
		}

		while (!_queue.try_dequeue(task) && _running);

		return task;
	}

	/*ThreadPool::Task ThreadPool::wait_take_task()
	{
		MutexLockGuard lock(_mutex);

		while (_queue.empty() && _running)
		{
			_cond.wait(_mutex);
		}

		Task task;
		if(!_queue.empty())
		{
			task = _queue.front();
			_queue.pop_front();
		}

		
		return task;
	}*/

	void ThreadPool::workThread()
	{
		try
		{
			while (_running)
			{
				Task task(take());
				if (task)
				{
					task();
				}
			}
		}
		catch (...)
		{
			throw;
		}
		
	}
}


