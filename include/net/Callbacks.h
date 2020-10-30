#ifndef __T__MUMA_CALLBACK_H__
#define __T__MUMA_CALLBACK_H__


#include <functional>
#include <memory>

namespace muma
{

class Buffer;
// All client visible callbacks go here.

class TcpConnection;
//typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;
typedef TcpConnection* TcpConnectionPtr;

typedef std::function<void (const TcpConnectionPtr&)> ConnectionCallback;
typedef std::function<void (const TcpConnectionPtr&)> CloseCallback;
typedef std::function<void (const TcpConnectionPtr&)> WriteCompleteCallback;
typedef std::function<void (const TcpConnectionPtr&, size_t)> HighWaterMarkCallback;

// the data has been read to (buf, len)
typedef std::function<void (const TcpConnectionPtr&,
                              Buffer*)> MessageCallback;


typedef std::function<void()> TimerCallback;

//
typedef std::function<void(void*)> DestructionCallback;

}

#endif
