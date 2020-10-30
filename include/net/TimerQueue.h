#ifndef __T__MUMA_TIMERQUEUE_H__
#define __T__MUMA_TIMERQUEUE_H__


#include <set>
#include <vector>

#include "../../include/net/Timer.h"
#include "../../include/net/TimerId.h"
#include "../../include/net/Channel.h"
#include "../../include/base/Timestamp.h"

namespace muma
{
class EventLoop;


class TimerQueue
{
public:
	typedef std::function<void()> Functor;

	TimerQueue(EventLoop* loop);
	~TimerQueue(void);

	/// Schedules the callback to be run at given time,
	/// repeats if @c interval > 0.
	/// Must be thread safe. Usually be called from other threads.
	TimerId addTimer(const TimerCallback& cb,
		Timestamp when,
		int64_t interval);

	void cancel(TimerId timerId, const Functor& cb = NULL);

private:
	// FIXME: use unique_ptr<Timer> instead of raw pointers.
	typedef std::pair<Timestamp, Timer*> Entry;
	typedef std::set<Entry> TimerList;
	typedef std::pair<Timer*, int64_t> ActiveTimer;
	typedef std::set<ActiveTimer> ActiveTimerSet;

private:
	void addTimerInLoop(Timer* timer);
	void cancelInLoop(TimerId timerId, const Functor& cb);

	bool insert(Timer* timer);
	// called when timerfd alarms
	void handleRead();
	// move out all expired timers
	std::vector<Entry> getExpired(Timestamp now);
	void reset(const std::vector<Entry>& expired, Timestamp now);

private:
	EventLoop* _loop;
	HANDLE _timerfd;
	Channel* _timerfdChannel;
	
	// Timer list sorted by expiration
	TimerList _timers;
	// for cancel()
	ActiveTimerSet _activeTimers;
	
};


}

#endif