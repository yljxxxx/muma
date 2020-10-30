#include <muma/Common.h>
#include <muma/net/TcpClient.h>
#include <muma/net/SocketsOps.h>

#include <boost/bind.hpp>

namespace muma
{

namespace detail
{

void removeConnection(EventLoop* loop, const TcpConnectionPtr& conn)
{
  loop->queueInLoop(boost::bind(&TcpConnection::connectDestroyed, conn));
}


}


TcpClient::TcpClient(EventLoop* loop,
            const InetAddress& serverAddr)
			: _loop(loop),
			_connector(new TcpConnector(loop, serverAddr)),
			_connectionCallback(NULL),
			_messageCallback(NULL),
			_retry(false),
			_connect(true),
			_nextConnId(1)
{
	_connector->setNewConnectionCallback(
		boost::bind(&TcpClient::newConnection, this, _1));
}

TcpClient::~TcpClient(void)
{
	TcpConnectionPtr conn;
	{
		MutexLockGuard lock(_mutex);
		conn = _connection;
	}
	if (conn)
	{
		// FIXME: not 100% safe, if we are in different thread
		CloseCallback cb = boost::bind(&detail::removeConnection, _loop, _1);
		_loop->runInLoop(
			boost::bind(&TcpConnection::setCloseCallback, conn, cb));
	}
	else
	{
		_connector->stop();
	}
}

void TcpClient::connect()
{
  // FIXME: check state
  _connect = true;
  _connector->start();
}


void TcpClient::disconnect()
{
  _connect = false;

  {
    MutexLockGuard lock(_mutex);
    if (_connection)
    {
      _connection->shutdown();
    }
  }
}


void TcpClient::stop()
{
  _connect = false;
  _connector->stop();
}


void TcpClient::newConnection(SOCKET sockfd)
{
	assert(_loop->isInLoopThread());

	InetAddress peerAddr(sockets::getPeerAddr(sockfd));

	++_nextConnId;

	InetAddress localAddr(sockets::getLocalAddr(sockfd));
	// FIXME poll with zero timeout to double confirm the new connection
	// FIXME use make_shared if necessary
	TcpConnectionPtr conn(new TcpConnection(_loop,
		_nextConnId,
		sockfd,
		localAddr,
		peerAddr));

	conn->setConnectionCallback(_connectionCallback);
	conn->setMessageCallback(_messageCallback);
	conn->setWriteCompleteCallback(_writeCompleteCallback);
	conn->setCloseCallback(
		boost::bind(&TcpClient::removeConnection, this, _1)); // FIXME: unsafe
	{
		MutexLockGuard lock(_mutex);
		_connection = conn;
	}
	conn->connectEstablished();
}


void TcpClient::removeConnection(const TcpConnectionPtr& conn)
{
	assert(_loop->isInLoopThread());
	assert(_loop == conn->getLoop());

	{
		MutexLockGuard lock(_mutex);
		assert(_connection == conn);
		_connection.reset();
	}

	_loop->queueInLoop(boost::bind(&TcpConnection::connectDestroyed, conn));
	if (_retry && _connect)
	{
		_connector->restart();
	}
}


}