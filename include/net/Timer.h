#ifndef __T__MUMA_TIMER_H__
#define __T__MUMA_TIMER_H__

#include "../../include/base/Timestamp.h"
#include "../../include/net/Callbacks.h"
#include "../../include/base/Mutex.h"

namespace muma
{
///
/// Internal class for timer event.
///
class Timer
{
public:
	Timer(const TimerCallback& cb, Timestamp when, int64_t interval);

	void run() const
	{
		_callback();
	}

	Timestamp expiration() const  { return _expiration; }
	bool repeat() const { return _repeat; }
	int64_t sequence() const { return _sequence; }

	void restart(Timestamp now);

private:
	const TimerCallback _callback;
	Timestamp _expiration;
	int64_t _interval;
	const bool _repeat;

	int64_t _sequence;

	static MutexLock s_timeidCountLock;
	static int64_t s_timeidCount;

	
};


}


#endif
