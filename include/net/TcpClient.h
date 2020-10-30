#ifndef __T__MUMA_TCPCLIENT_H__
#define __T__MUMA_TCPCLIENT_H__

#include "../base/Mutex.h"
#include "TcpConnection.h"
#include "TcpConnector.h"
#include "../base/Event.h"
#include "Callbacks.h"

namespace muma
{

	typedef std::shared_ptr<TcpConnector> TcpConnectorSharedPtr;
	typedef std::shared_ptr<TcpConnection> TcpConnectionSharedPtr;

	typedef std::function<void(const char* name, int errorcode)> ConnectExceptionMsgCallback;

	class TcpClient
	{
	public:
		TcpClient(const char* name, EventLoop* loop,
			const InetAddress& serverAddr, bool retry
			, int retryIntervalMs, int timeout);

		const char* getname() { return _name.c_str(); }

		void connect();
		void disconnect();

		bool retry() const { return _retry; };
		void enableRetry() { _retry = true; }

		EventLoop* getLoop() {
			return _loop;
		}
		/// Not thread safe.
		void setConnectionCallback(const ConnectionCallback& cb)
		{
			_connectionCallback = cb;
		}

		/// Not thread safe.
		void setMessageCallback(const MessageCallback& cb)
		{
			_messageCallback = cb;
		}

		/// Not thread safe.
		void setWriteCompleteCallback(const WriteCompleteCallback& cb)
		{
			_writeCompleteCallback = cb;
		}

		/// Not thread safe.
		void setConnectExceptionCallback(const ConnectExceptionMsgCallback& cb)
		{
			_connectExceptionCallback = cb;
		}

	private:
		~TcpClient(void);
		/// Not thread safe, but in loop
		void newConnection(SOCKET sockfd);
		/// Not thread safe, but in loop
		void removeConnection(const TcpConnectionPtr& conn);

		void connectionException(int errorcode);

		void disconnectInLoop();
		void reconnectInLoop();

	private:
		const std::string _name;
		EventLoop* _loop;
		TcpConnector* _connector; // avoid revealing Connector

		ConnectionCallback _connectionCallback;
		MessageCallback _messageCallback;
		WriteCompleteCallback _writeCompleteCallback;
		ConnectExceptionMsgCallback _connectExceptionCallback;

		bool _retry;
		bool _connect;

		// always in loop thread
		int _nextConnId;
		mutable MutexLock _mutex;
		TcpConnectionPtr _connection;

	};

}
#endif