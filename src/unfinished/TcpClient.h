#ifndef __T__MUMA_TCPCLIENT_H__
#define __T__MUMA_TCPCLIENT_H__

#include <muma/base/Mutex.h>
#include <muma/net/TcpConnection.h>
#include <muma/net/TcpConnector.h>

namespace muma
{

typedef boost::shared_ptr<TcpConnector> TcpConnectorPtr;

class TcpClient
{
public:
	TcpClient(EventLoop* loop,
            const InetAddress& serverAddr);
	~TcpClient(void);

	void connect();
	void disconnect();
	void stop();

	TcpConnectionPtr connection() const
	{
		MutexLockGuard lock(_mutex);
		return _connection;
	}

	bool retry() const;
	void enableRetry() { _retry = true; }


	/// Not thread safe.
	void setConnectionCallback(const ConnectionCallback& cb)
	{ _connectionCallback = cb; }

	/// Not thread safe.
	void setMessageCallback(const MessageCallback& cb)
	{ _messageCallback = cb; }

	/// Not thread safe.
	void setWriteCompleteCallback(const WriteCompleteCallback& cb)
	{ _writeCompleteCallback = cb; }

private:
	/// Not thread safe, but in loop
	void newConnection(SOCKET sockfd);
	/// Not thread safe, but in loop
	void removeConnection(const TcpConnectionPtr& conn);


private:
	EventLoop* _loop;
	TcpConnectorPtr _connector; // avoid revealing Connector

	ConnectionCallback _connectionCallback;
	MessageCallback _messageCallback;
	WriteCompleteCallback _writeCompleteCallback;
	bool _retry;   
	bool _connect; 

	// always in loop thread
	int _nextConnId;
	mutable MutexLock _mutex;
	TcpConnectionPtr _connection; 

};


}

#endif