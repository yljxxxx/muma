#ifndef __T__MUMA_TIMERID_H__
#define __T__MUMA_TIMERID_H__

namespace muma
{

	class Timer;
///
/// An opaque identifier, for canceling Timer.
///
class TimerId
{
 public:
  TimerId()
    : _timer(NULL),
      _sequence(0)
  {
  }

  TimerId(Timer* timer, int64_t seq)
    : _timer(timer),
      _sequence(seq)
  {
  }

  BOOL IsValid() {
	  return _timer != NULL;
  }

  void SetInvalid()
  {
	  _timer = NULL;
	  _sequence = 0;
  }

  // default copy-ctor, dtor and assignment are okay
  friend class TimerQueue;

 private:
  Timer* _timer;
  int64_t _sequence;
};


}

#endif  // MUDUO_NET_TIMERID_H
