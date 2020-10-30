#ifndef __T_MUMA_LOGSTREAM_H__
#define __T_MUMA_LOGSTREAM_H__

namespace muma
{

const int kSmallBuffer = 4096;
const int kLargeBuffer = 4096*1024;

template<int SIZE>
class FixedBuffer
{
 public:
  FixedBuffer()
    : _cur(_data)
  {
  }

  ~FixedBuffer()
  {
  }

  void append(const char* /*restrict*/ buf, size_t len)
  {
    // FIXME: append partially
    if (implicit_cast<size_t>(avail()) > len)
    {
      memcpy(_cur, buf, len);
      _cur += len;
    }
  }
  
  const char* data() const { return _data; }
  int length() const { return static_cast<int>(_cur - _data); }

  // write to _data directly
  char* current() { return _cur; }
  int avail() const { return static_cast<int>(end() - _cur); }
  void add(size_t len) { _cur += len; }

  void reset() { _cur = _data; }
  void bzero() { memset(_data, 0, sizeof(_data)); }

 private:
  const char* end() const { return _data + sizeof(_data); }

  char _data[SIZE];
  char* _cur;
};

//class LogStream
//{
//  typedef LogStream self;
// public:
//  typedef detail::FixedBuffer<detail::kSmallBuffer> Buffer;
//
//  self& operator<<(bool v)
//  {
//    buffer_.append(v ? "1" : "0", 1);
//    return *this;
//  }
//
//  self& operator<<(short);
//  self& operator<<(unsigned short);
//  self& operator<<(int);
//  self& operator<<(unsigned int);
//  self& operator<<(long);
//  self& operator<<(unsigned long);
//  self& operator<<(long long);
//  self& operator<<(unsigned long long);
//
//  self& operator<<(const void*);
//
//  self& operator<<(float v)
//  {
//    *this << static_cast<double>(v);
//    return *this;
//  }
//  self& operator<<(double);
//  // self& operator<<(long double);
//
//  self& operator<<(char v)
//  {
//    buffer_.append(&v, 1);
//    return *this;
//  }
//
//  // self& operator<<(signed char);
//  // self& operator<<(unsigned char);
//
//  self& operator<<(const char* v)
//  {
//    buffer_.append(v, strlen(v));
//    return *this;
//  }
//
//  self& operator<<(const string& v)
//  {
//    buffer_.append(v.c_str(), v.size());
//    return *this;
//  }
//
//#ifndef MUDUO_STD_STRING
//  self& operator<<(const std::string& v)
//  {
//    buffer_.append(v.c_str(), v.size());
//    return *this;
//  }
//#endif
//
//  self& operator<<(const StringPiece& v)
//  {
//    buffer_.append(v.data(), v.size());
//    return *this;
//  }
//
//  void append(const char* data, int len) { buffer_.append(data, len); }
//  const Buffer& buffer() const { return buffer_; }
//  void resetBuffer() { buffer_.reset(); }
//
// private:
//  void staticCheck();
//
//  template<typename T>
//  void formatInteger(T);
//
//  Buffer buffer_;
//
//  static const int kMaxNumericSize = 32;
//};


}




#endif