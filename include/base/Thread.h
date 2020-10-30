#ifndef __T__MUMA_THREAD_H__
#define __T__MUMA_THREAD_H__

#include <functional>

namespace muma
{

	class Thread
	{
	public:
		typedef std::function<void()> ThreadFunc;

		explicit Thread(const ThreadFunc& func);
		~Thread();

		int start();
		int join();

		bool started() const { return _start; }
		unsigned int GetId() const { return _id; }

	private:
		ThreadFunc _func;
		unsigned int _id;
		HANDLE _handle;
		bool _start;

		static unsigned int _stdcall entry(void* obj);
	};

}


#endif