#ifndef __T_MUMA_TIMESTAMP_H
#define __T_MUMA_TIMESTAMP_H

#include "../Types.h"
#include <stdint.h>
#include <string>

namespace muma
{


class Timestamp
{
public:
	Timestamp()
    : _millisecondsSince1970(0)
	{
	}

	explicit Timestamp(int64_t millisecondsSince1970);

	explicit Timestamp(DateTime& dt);

	void toDateTime(DateTime& dt) const;

	int64_t milliSecondsSince1970() const { 
		return _millisecondsSince1970; 
	}

	bool valid() const { return _millisecondsSince1970 != 0; }

	static Timestamp now();
	static Timestamp invalid();

	
private:
	int64_t  _millisecondsSince1970;

	
};


inline bool operator<(Timestamp lhs, Timestamp rhs)
{
  return lhs.milliSecondsSince1970() < rhs.milliSecondsSince1970();
}

inline bool operator==(Timestamp lhs, Timestamp rhs)
{
  return lhs.milliSecondsSince1970() == rhs.milliSecondsSince1970();
}

inline bool operator<=(Timestamp lhs, Timestamp rhs)
{
  return lhs.milliSecondsSince1970() <= rhs.milliSecondsSince1970();
}

/// Add @c seconds to given timestamp.
/// @return timestamp+seconds as Timestamp
inline Timestamp addTime(Timestamp timestamp, int64_t milliseconds)
{
  return Timestamp(timestamp.milliSecondsSince1970() + milliseconds);
}

}

#endif