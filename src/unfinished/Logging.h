#ifndef __T_MUMA_LOGGING_H__
#define __T_MUMA_LOGGING_H__

#include <muma/base/Timestamp.h>

namespace muma
{

class Logger
{
public:
	enum LogLevel
	{
		TRACE,
		DEBUG,
		INFO,
		WARN,
		ERROR,
		FATAL,
		NUM_LOG_LEVELS,
	};

// compile time calculation of basename of source file
class SourceFile
{
 public:
  template<int N>
  inline SourceFile(const char (&arr)[N])
    : _data(arr),
      _size(N-1)
  {
    const char* slash = strrchr(_data, '/'); // builtin function
    if (slash)
    {
      _data = slash + 1;
      _size -= static_cast<int>(_data - arr);
    }
  }

  explicit SourceFile(const char* filename)
    : _data(filename)
  {
    const char* slash = strrchr(filename, '/');
    if (slash)
    {
      _data = slash + 1;
    }
    _size = static_cast<int>(strlen(_data));
  }

  const char* _data;
  int _size;
};

Logger(SourceFile file, int line);
Logger(SourceFile file, int line, LogLevel level);
Logger(SourceFile file, int line, LogLevel level, const char* func);
Logger(SourceFile file, int line, bool toAbort);
~Logger();

}





#endif