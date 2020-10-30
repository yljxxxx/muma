#ifndef __T__MUMA_BUFFER_H__
#define __T__MUMA_BUFFER_H__

#include <algorithm>
#include <vector>

namespace muma
{

/// A buffer class modeled after org.jboss.netty.buffer.ChannelBuffer
///
/// @code
/// +-------------------+------------------+------------------+
/// | prependable bytes |  readable bytes  |  writable bytes  |
/// |                   |     (CONTENT)    |                  |
/// +-------------------+------------------+------------------+
/// |                   |                  |                  |
/// 0      <=      readerIndex   <=   writerIndex    <=     size
/// @endcode

class Buffer
{
public:
	static const size_t kCheapPrepend = 8;
	static const size_t kInitialSize = 4*1024;

public:
	Buffer(size_t initialSize = kInitialSize)
		: _buffer(kCheapPrepend + initialSize),
		  _readerIndex(kCheapPrepend),
		  _writerIndex(kCheapPrepend)
	{
		assert(readableBytes() == 0);
		assert(writableBytes() == initialSize);
		assert(prependableBytes() == kCheapPrepend);
	}


	void swap(Buffer& rhs)
	{
		_buffer.swap(rhs._buffer);
		std::swap(_readerIndex, rhs._readerIndex);
		std::swap(_writerIndex, rhs._writerIndex);
	}

	size_t readableBytes() const
	{ return _writerIndex - _readerIndex; }

	size_t writableBytes() const
	{ return _buffer.size() - _writerIndex; }

	size_t prependableBytes() const
	{ return _readerIndex; }

	const char* peek() const
	{ return begin() + _readerIndex; }

	const char* findCRLF() const
	{
		const char* crlf = std::search(peek(), beginWrite(), kCRLF, kCRLF+2);
		return crlf == beginWrite() ? NULL : crlf;
	}

	const char* findCRLF(const char* start) const
	{
		assert(peek() <= start);
		assert(start <= beginWrite());
		const char* crlf = std::search(start, beginWrite(), kCRLF, kCRLF+2);
		return crlf == beginWrite() ? NULL : crlf;
	}


	void retrieve(size_t len)
	{
		assert(len <= readableBytes());
		if (len < readableBytes())
		{
			_readerIndex += len;
		}
		else
		{
			retrieveAll();
		}
	}

	void retrieveUntil(const char* end)
	{
		assert(peek() <= end);
		assert(end <= beginWrite());
		retrieve(end - peek());
	}

	void retrieveAll()
	{
		_readerIndex = kCheapPrepend;
		_writerIndex = kCheapPrepend;
	}

	void append(const char*  data, size_t len)
	{
		ensureWritableBytes(len);
		std::copy(data, data+len, beginWrite());
		hasWritten(len);
	}

	void append(const void* data, size_t len)
	{
		append(static_cast<const char*>(data), len);
	}

	void ensureWritableBytes(size_t len)
	{
		if (writableBytes() < len)
		{
			makeSpace(len);
		}
		assert(writableBytes() >= len);
	}

	char* beginWrite()
	{ return begin() + _writerIndex; }

	const char* beginWrite() const
	{ return begin() + _writerIndex; }

	void hasWritten(uint32_t len)
	{ 
		assert(len <= writableBytes());
		_writerIndex += len; 
	}


	void prepend(const void* data, size_t len)
	{
		assert(len <= prependableBytes());
		_readerIndex -= len;
		const char* d = static_cast<const char*>(data);
		std::copy(d, d+len, begin()+_readerIndex);
	}

	size_t internalCapacity() const
	{
		return _buffer.capacity();
	}

	/// Read data directly into buffer.
	int32_t readFd(SOCKET fd, int* savedErrno);

private:
	char* begin()
	{ return &*_buffer.begin(); }

	const char* begin() const
	{ return &*_buffer.begin(); }


	void makeSpace(size_t len)
	{
		if (writableBytes() + prependableBytes() < len + kCheapPrepend)
		{
			// FIXME: move readable data
			_buffer.resize(_writerIndex+len);
		}
		else
		{
			// move readable data to the front, make space inside buffer
			assert(kCheapPrepend < _readerIndex);

			size_t readable = readableBytes();
			std::copy(begin()+_readerIndex,
				begin()+_writerIndex,
				begin()+kCheapPrepend);
			_readerIndex = kCheapPrepend;
			_writerIndex = _readerIndex + readable;
			assert(readable == readableBytes());
		}
	}

private:
  std::vector<char> _buffer;
  size_t _readerIndex;
  size_t _writerIndex;

  static const char kCRLF[];
};


}

#endif