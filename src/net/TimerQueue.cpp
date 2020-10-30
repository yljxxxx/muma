#include "..\common.h"
#include "../../include/base/Timestamp.h"
#include "../../include/net/EventLoop.h"

#include <iterator>

namespace muma
{

HANDLE createTimerfd()
{
	return ::CreateWaitableTimer(NULL, FALSE, NULL);
}

void resetTimerfd(HANDLE timerfd, Timestamp expiration)
{
	// wake up loop by SetWaitableTimer()
	LARGE_INTEGER  duetime;
	int64_t millisecs = expiration.milliSecondsSince1970() - Timestamp::now().milliSecondsSince1970();
	if(millisecs < 0)
		millisecs = 0;

	duetime.QuadPart = -(millisecs*1000*10);   
	::SetWaitableTimer(timerfd, &duetime, 0, NULL, NULL, FALSE);
}



TimerQueue::TimerQueue(EventLoop* loop) 
: _loop(loop),
 _timerfd(createTimerfd())
{
	assert(_loop->isInLoopThread());
	_timerfdChannel = loop->NewChannel(INVALID_SOCKET, _timerfd, Channel::FD_OF_TIMER);

	_timerfdChannel->setReadCallback(
      std::bind(&TimerQueue::handleRead, this));

	// we are always reading the timerfd, we disarm it with SetWaitableTimer.
	_timerfdChannel->enableReading();
}

TimerQueue::~TimerQueue(void)
{
	assert(_loop->isInLoopThread());
	_timerfdChannel->disableReading();
	_timerfdChannel->setReadCallback(NULL);
	_loop->DeleteChannel(_timerfdChannel);

	::CloseHandle(_timerfd);

	for (TimerList::iterator it = _timers.begin();
		it != _timers.end(); ++it)
	{
		delete it->second;
	}
}


TimerId TimerQueue::addTimer(const TimerCallback& cb,
		Timestamp when,
		int64_t interval)
{
	Timer* timer = new Timer(cb, when, interval);  
	_loop->queueInLoop(
      std::bind(&TimerQueue::addTimerInLoop, this, timer));

	return TimerId(timer, timer->sequence());
}


void TimerQueue::cancel(TimerId timerId, const Functor& cb)
{
  _loop->runInLoop(
      std::bind(&TimerQueue::cancelInLoop, this, timerId, cb));
}



void TimerQueue::addTimerInLoop(Timer* timer)
{
	assert(_loop->isInLoopThread());
	bool earliestChanged = insert(timer);

	if (earliestChanged)
	{
		resetTimerfd(_timerfd, timer->expiration());
	}
}

void TimerQueue::cancelInLoop(TimerId timerId, const Functor& cb)
{
	assert(_loop->isInLoopThread());
	assert(_timers.size() == _activeTimers.size());

	ActiveTimer timer(timerId._timer, timerId._sequence);
	ActiveTimerSet::iterator it = _activeTimers.find(timer);
	if (it != _activeTimers.end())
	{
		size_t n = _timers.erase(Entry(it->first->expiration(), it->first));
		assert(n == 1);
		delete it->first; // FIXME: no delete please
		_activeTimers.erase(it);
	}

	assert(_timers.size() == _activeTimers.size());

	if (cb != NULL)
		cb();
}


void TimerQueue::handleRead()
{
	assert(_loop->isInLoopThread());
	Timestamp now(Timestamp::now());

	std::vector<Entry> expired = getExpired(now);

	 // safe to callback outside critical section
	for (std::vector<Entry>::iterator it = expired.begin();
      it != expired.end(); ++it)
	{
		it->second->run();
	}


	reset(expired, now);
}

std::vector<TimerQueue::Entry> TimerQueue::getExpired(Timestamp now)
{
	assert(_timers.size() == _activeTimers.size());

	std::vector<Entry> expired;
	Entry sentry(now, reinterpret_cast<Timer*>(MAXULONG_PTR));

	TimerList::iterator iterEnd = _timers.lower_bound(sentry);
	assert(iterEnd == _timers.end() || now <= iterEnd->first);

	std::copy(_timers.begin(), iterEnd, std::back_inserter(expired));
	_timers.erase(_timers.begin(), iterEnd);

	for (std::vector<Entry>::iterator it = expired.begin();
		it != expired.end(); ++it)
	{
		ActiveTimer timer(it->second, it->second->sequence());
		size_t n = _activeTimers.erase(timer);
		assert(n == 1);
	}

	assert(_timers.size() == _activeTimers.size());
	return expired;
}

void TimerQueue::reset(const std::vector<Entry>& expired, Timestamp now)
{
	Timestamp nextExpire;

	for (std::vector<Entry>::const_iterator it = expired.begin();
		it != expired.end(); ++it)
	{
		ActiveTimer timer(it->second, it->second->sequence());
		if (it->second->repeat())
		{
			it->second->restart(now);
			insert(it->second);
		}
		else
		{
			// FIXME move to a free list
			delete it->second; // FIXME: no delete please
		}
	}

	if (!_timers.empty())
	{
		nextExpire = _timers.begin()->second->expiration();
		resetTimerfd(_timerfd, nextExpire);
	}
}

bool TimerQueue::insert(Timer* timer)
{
  assert(_loop->isInLoopThread());
  assert(_timers.size() == _activeTimers.size());

  bool earliestChanged = false;
  Timestamp when = timer->expiration();
  TimerList::iterator it = _timers.begin();
  if (it == _timers.end() || when < it->first)
  {
    earliestChanged = true;
  }
  {
    std::pair<TimerList::iterator, bool> result
      = _timers.insert(Entry(when, timer));
    assert(result.second); 
  }
 
  {
    std::pair<ActiveTimerSet::iterator, bool> result
      = _activeTimers.insert(ActiveTimer(timer, timer->sequence()));
    assert(result.second); 
  }

  assert(_timers.size() == _activeTimers.size());
  return earliestChanged;
}

}
