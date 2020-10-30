#include "..\common.h"
#include "../../include/net/TcpServer.h"
#include "../../include/net/SocketsOps.h"

#include "include\base\Event.h"


namespace muma
{


TcpServer::TcpServer(EventLoop* loop,
                     const InetAddress& listenAddr, bool isReusePort)
  : _loop(loop),
    _hostport(listenAddr.toIpPort()),
    _acceptor(new TcpAcceptor(loop, listenAddr, isReusePort)),
    _threadPool(new EventLoopThreadPool()),
	_started(false),
	_nextConnId(1),
	_connectEstablishedCallback(NULL),
	_connectDestroyedCallback(NULL),
	_messageCallback(NULL),
	_writeCompleteCallback(NULL)
{
  _acceptor->setNewConnectionCallback(
	  std::bind(&TcpServer::newConnection, this, std::placeholders::_1, std::placeholders::_2));
}

TcpServer::~TcpServer()
{
	assert(_loop->isInLoopThread());

	//先关闭接收器，防止新连接
	_acceptor.release();

	muma::Event tempEvent;
	
	for (ConnectionMap::iterator iter(_connections.begin());
		iter != _connections.end(); ++iter)
	{
		TcpConnectionPtr conn = iter->second;

		tempEvent.Reset();

		auto delFunc = [](TcpConnectionPtr& conn, muma::Event& tEvent) {
			delete conn;
			tEvent.Set();
		};

		EventLoop* ioLoop = conn->getLoop();
		ioLoop->queueInLoop(
			std::bind(delFunc, conn, tempEvent));
		
		tempEvent.Wait();
	}

	_connections.clear();
}

void TcpServer::start(int numThreads, const ThreadInitCallback& cb)
{
  if (!_started)
  {
		_started = true;
		_threadPool->run(numThreads, cb);
  }

  if (!_acceptor->isListenning())
  {
	  _loop->queueInLoop(
		  std::bind(&TcpAcceptor::listen, _acceptor.get()));
  }
}

void TcpServer::newConnection(SOCKET sockfd, const InetAddress& peerAddr)
{
	assert(_loop->isInLoopThread());

	EventLoop* ioLoop = _threadPool->getNextLoop();
	if(NULL == ioLoop)
		ioLoop = _loop;

	int64_t connId = ++_nextConnId;

	// FIXME poll with zero timeout to double confirm the new connection
	// FIXME use make_shared if necessary
	TcpConnectionPtr conn = new TcpConnection(ioLoop,
		connId, sockfd, peerAddr);
	
	_connections[connId] = conn;
	conn->setConnectEstablishedCallback(_connectEstablishedCallback);
	conn->setConnectDestroyedCallback(_connectDestroyedCallback);
	conn->setMessageCallback(_messageCallback);
	conn->setWriteCompleteCallback(_writeCompleteCallback);
	conn->setCloseCallback(
		std::bind(&TcpServer::closeConnection, this, std::placeholders::_1));
	
	ioLoop->queueInLoop(std::bind(&TcpConnection::connectEstablished, conn));
}

void TcpServer::deleteConnection(TcpConnectionPtr& conn, std::function<void()> cb)
{
	_loop->queueInLoop(std::bind(&TcpServer::deleteConnectionInLoop, this, conn, cb));
}

void TcpServer::deleteConnectionInLoop(TcpConnectionPtr& conn, std::function<void()> cb)
{
	assert(_loop->isInLoopThread());

	auto delFunc = [](TcpConnectionPtr& conn, std::function<void()> cb) {
		delete conn;
		if (cb)
			cb();
	};

	ConnectionMap::iterator iter = _connections.find(conn->id());
	if (iter != _connections.end())
	{
		assert(conn == iter->second);
		_connections.erase(iter);

		EventLoop* ioLoop = conn->getLoop();
		ioLoop->queueInLoop(
			std::bind(delFunc, conn, cb));
	}
}



void TcpServer::closeConnection(const TcpConnectionPtr& conn)
{
	_loop->queueInLoop(std::bind(&TcpServer::closeConnectionInLoop, this, conn));
}

void TcpServer::closeConnectionInLoop(const TcpConnectionPtr& conn)
{
	assert(_loop->isInLoopThread());

	EventLoop* ioLoop = conn->getLoop();
	ioLoop->queueInLoop(
		std::bind(&TcpConnection::connectDestroyed, conn));
}



}