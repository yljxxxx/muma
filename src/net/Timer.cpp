#include "..\common.h"
#include "../../include/net/Timer.h"

namespace muma
{

	MutexLock Timer::s_timeidCountLock;
	int64_t Timer::s_timeidCount = 0;

	Timer::Timer(const TimerCallback& cb, Timestamp when, int64_t interval)
		: _callback(cb),
		_expiration(when),
		_interval(interval),
		_repeat(interval > 0)
	{
		s_timeidCountLock.lock();
		_sequence = ++s_timeidCount;
		s_timeidCountLock.unlock();
	}


	void Timer::restart(Timestamp now)
	{
		if (_repeat)
		{
			_expiration = addTime(now, _interval);
		}
		else
		{
			_expiration = Timestamp::invalid();
		}
	}


}