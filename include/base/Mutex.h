#ifndef __T_MUMA_MUTEX_H__
#define __T_MUMA_MUTEX_H__

#include <Windows.h>
#include <limits.h>

namespace muma
{

	class MutexLock
	{
	public:
		MutexLock(void)
		{
			InitializeCriticalSection(&_mutex);
		}

		~MutexLock(void)
		{
			DeleteCriticalSection(&_mutex);
		}

		void lock()
		{
			EnterCriticalSection(&_mutex);
		}

		void unlock()
		{
			LeaveCriticalSection(&_mutex);
		}

	private:
		CRITICAL_SECTION _mutex;
	};

	class MutexLockGuard
	{

	public:
		explicit MutexLockGuard(MutexLock& mutex)
			: _mutex(mutex)
		{
			_mutex.lock();
		}

		~MutexLockGuard()
		{
			_mutex.unlock();
		}

	private:
		MutexLock& _mutex;
	};

	//可共享读的读写锁，供后续类使用
	class RwLock
	{
	public:
		RwLock() : _read_count(0)
		{
			InitializeCriticalSection(&_read_mutex);
			InitializeCriticalSection(&_write_mutex);
		}

		~RwLock()
		{
			DeleteCriticalSection(&_read_mutex);
			DeleteCriticalSection(&_write_mutex);
		}

	public:
		void read()
		{
			EnterCriticalSection(&_read_mutex);
			if(_read_count++ == 0)
			{
				EnterCriticalSection(&_write_mutex);
			}

			LeaveCriticalSection(&_read_mutex);
		}

		void unread()
		{
			EnterCriticalSection(&_read_mutex);;
			if( --_read_count == 0)
			{
				LeaveCriticalSection(&_write_mutex);
			}

			LeaveCriticalSection(&_read_mutex);
		}

		void write()
		{ 
			EnterCriticalSection(&_write_mutex);
		}

		void unwrite()
		{
			LeaveCriticalSection(&_write_mutex);
		}

	private:
		CRITICAL_SECTION _read_mutex;
		CRITICAL_SECTION _write_mutex; 

		int _read_count;
	};


	//读锁，析构时自动释放
	class ReadLockGuard
	{
	public:
		ReadLockGuard(RwLock& lock) : _lock(lock)
		{
			_lock.read();
		}

		~ReadLockGuard()
		{
			_lock.unread();
		}

	private:
		RwLock& _lock;
	};

	//写锁，析构时自动释放
	class WriteLockGuard
	{
	public:
		WriteLockGuard(RwLock& lock) : _lock(lock)
		{
			_lock.write();
		}

		~WriteLockGuard() 
		{
			_lock.unwrite();
		}

	private:
		RwLock& _lock;
	};


	/*** Condition variables ***/
	class Condition
	{
	public:

		Condition(void) //: _count(0)
		{
			_sem = CreateSemaphore(NULL, 0, LONG_MAX, NULL);
		}

		~Condition(void)
		{
			CloseHandle(_sem);
		}

		void signal()			// must hold the external mutex before enter
		{	
			ReleaseSemaphore(_sem, 1, NULL);
		}

		void broadcast()		// must hold the external mutex before enter
		{
			//ReleaseSemaphore(_sem, _count, NULL);
			ReleaseSemaphore(_sem, LONG_MAX, NULL);   //直接使用最大值，简单粗暴
		}

		/*void wait(MutexLock& mutex)		// must hold the external mutex before enter
		{
			_count++;
			mutex.unlock();

			WaitForSingleObject (_sem, INFINITE);

			mutex.lock();
			_count--;
		}*/

		void wait()	
		{
			//_count++;
			WaitForSingleObject(_sem, INFINITE);
			//_count--;
		}

	private:
		HANDLE _sem;
		//LONG _count;
	};


}

#endif