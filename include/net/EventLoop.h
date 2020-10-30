#ifndef __T__MUMA_EVENTLOOP_H__
#define __T__MUMA_EVENTLOOP_H__

#include <memory>

#include "Channel.h"
#include "../../include/net/Poller.h"
#include "../../include/net/TimerQueue.h"
#include "../../include/base/Mutex.h"


namespace muma
{
	class EventLoop
	{
	public:
		typedef std::function<void()> Functor;

		EventLoop();
		~EventLoop();

		Channel* NewChannel(SOCKET socket, HANDLE handle, Channel::FD_TYPE fdtype);
		void DeleteChannel(Channel* chan, const Functor& cb = NULL);
		

		/// Loops forever.
		/// Must be called in the same thread as creation of the object.
		void loop();

		void quit();

		/// Queues callback in the loop thread.
		/// Runs after finish pooling.
		/// Safe to call from other threads.
		void queueInLoop(const Functor& cb);
	
		void runInLoop(const Functor& cb);
		

		// timers

		/// Runs callback at 'time'.
		/// Safe to call from other threads.
		TimerId runAt(const Timestamp& time, const TimerCallback& cb);

		/// Runs callback after @c delay seconds.
		/// Safe to call from other threads.
		TimerId runAfter(int64_t delay, const TimerCallback& cb);

		/// Runs callback every @c interval seconds.
		/// Safe to call from other threads.
		TimerId runEvery(int64_t interval, const TimerCallback& cb);

		/// Cancels the timer.
		/// Safe to call from other threads.
		void cancel(TimerId timerId);
		void cancel(TimerId timerId, const Functor& cb);

		
		
		bool isInLoopThread() const { return _threadId == ::GetCurrentThreadId(); }

		static int GetMaxChannelCapcity(){ return Poller::MAX_CHANNEL_CAPCITY-2; }
		int GetAvailableChannelCapcity(){ return _poller->GetAvailableChannelCapcity(); }

	public: 
		const static unsigned int kInvalidChannelIndex = 0xFFFFFFFF;

	private:
		friend class Channel;
		// internal usage for channel
		int updateForChannel(Channel* channel);
		void removeForChannel(Channel* channel);

	private:
		void doPendingFunctors();

		void AddChannel(Channel* chan);
		void RemoveChannel(Channel* chan, const Functor& cb);
		
		void DeleteAllChannel(const Functor& cb = NULL);
		void RemoveAllChannel(const Functor& cb);

	private:
		const static unsigned int g_pollTimeountMs = 10000;

		std::vector<Channel*> _channels;

		bool _quit;
		bool _looping;
		const DWORD _threadId;
		std::unique_ptr<Poller> _poller;
		std::unique_ptr<TimerQueue> _timerQueue;
		
		MutexLock _mutex;
		std::vector<Functor> _pendingFunctors;
	};
}



#endif