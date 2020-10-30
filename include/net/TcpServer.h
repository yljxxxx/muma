#ifndef __T__MUMA_TCPSERVER_H__
#define __T__MUMA_TCPSERVER_H__

#include "../../include/net/TcpAcceptor.h"
#include "../../include/net/TcpConnection.h"
#include "../../include/net/EventLoopThreadPool.h"

#include <map>
#include <memory>

namespace muma
{


class TcpServer
{
public:
	typedef std::function<void(EventLoop*)> ThreadInitCallback;
	
	 TcpServer(EventLoop* loop,
            const InetAddress& listenAddr, bool isReusePort);
	~TcpServer();  

	const std::string& hostport() const { return _hostport; }

	/// Starts the server if it's not listenning.
	void start(int numThreads, const ThreadInitCallback& cb = ThreadInitCallback());

	void deleteConnection(TcpConnectionPtr& conn, std::function<void()> cb = NULL);

	/// Set connection callback.
	/// Not thread safe.
	void setConnectEstablishedCallback(const ConnectionCallback& cb)
	{ _connectEstablishedCallback = cb; }

	void setConnectDestroyedCallback(const ConnectionCallback& cb)
	{ _connectDestroyedCallback = cb; }

	/// Set message callback.
	/// Not thread safe.
	void setMessageCallback(const MessageCallback& cb)
	{ _messageCallback = cb; }

	/// Set write complete callback.
	/// Not thread safe.
	void setWriteCompleteCallback(const WriteCompleteCallback& cb)
	{ _writeCompleteCallback = cb; }

private:
	/// Not thread safe, but in loop
	void newConnection(SOCKET sockfd, const InetAddress& peerAddr);
	/// Thread safe.
	void closeConnection(const TcpConnectionPtr& conn);
	/// Not thread safe, but in loop
	void closeConnectionInLoop(const TcpConnectionPtr& conn);

	void deleteConnectionInLoop(TcpConnectionPtr& conn, std::function<void()> cb);
private:
	typedef std::map<int64_t, TcpConnectionPtr> ConnectionMap;

	EventLoop* _loop;
	const std::string _hostport;

	std::unique_ptr<TcpAcceptor> _acceptor;
	std::unique_ptr<EventLoopThreadPool> _threadPool;

	ConnectionCallback _connectEstablishedCallback;
	ConnectionCallback _connectDestroyedCallback;
	MessageCallback _messageCallback;
	WriteCompleteCallback _writeCompleteCallback;
	
	bool _started;

	// always in loop thread
	int64_t _nextConnId;
	ConnectionMap _connections;

};


}

#endif
