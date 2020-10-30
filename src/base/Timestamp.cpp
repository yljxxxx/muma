#include "..\common.h"
#include "../../include/base/Timestamp.h"
#include <time.h>

#define MILLISECONEDS_PERSECOND 1000

namespace muma
{


Timestamp::Timestamp(int64_t millisecondsSince1970)
  : _millisecondsSince1970(millisecondsSince1970)
{
}

Timestamp::Timestamp(DateTime& dt)
{
	struct tm tm;

	tm.tm_year     = dt.year - 1900;
	tm.tm_mon     = dt.month - 1;
	tm.tm_mday     = dt.day;
	tm.tm_hour     = dt.hour;
	tm.tm_min     = dt.minute;
	tm.tm_sec     = dt.second;

	int64_t secsSince1970 = ::_mktime64(&tm);
	_millisecondsSince1970 = secsSince1970*MILLISECONEDS_PERSECOND + dt.milsecs;
}

void Timestamp::toDateTime(DateTime& dt) const
{
	__time64_t seconds = static_cast<__time64_t>(_millisecondsSince1970 / MILLISECONEDS_PERSECOND);
	int milliseconds = static_cast<int>(_millisecondsSince1970 % MILLISECONEDS_PERSECOND);

	struct tm tm_time;
	::_localtime64_s(&tm_time, &seconds);

	dt.year = tm_time.tm_year + 1900;
	dt.month = tm_time.tm_mon + 1;
	dt.day = tm_time.tm_mday;
	dt.hour = tm_time.tm_hour;
	dt.minute = tm_time.tm_min;
	dt.second = tm_time.tm_sec;
	dt.milsecs = milliseconds;
}

Timestamp Timestamp::now()
{
	struct tm tm;
	SYSTEMTIME sys_tm;

	::GetLocalTime(&sys_tm);
	tm.tm_year     = sys_tm.wYear - 1900;
	tm.tm_mon     = sys_tm.wMonth - 1;
	tm.tm_mday     = sys_tm.wDay;
	tm.tm_hour     = sys_tm.wHour;
	tm.tm_min     = sys_tm.wMinute;
	tm.tm_sec     = sys_tm.wSecond;

	int64_t secsSince1970 = ::_mktime64(&tm);
	return Timestamp(secsSince1970*MILLISECONEDS_PERSECOND + sys_tm.wMilliseconds);
}

Timestamp Timestamp::invalid()
{
  return Timestamp();
}


}