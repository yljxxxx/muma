#include "../common.h"
#include "../../include/net/TcpClient.h"
#include "../../include/net//SocketsOps.h"
#include "../../include/net/EventLoop.h"

#include <functional>

namespace muma
{


TcpClient::TcpClient(const char* name, EventLoop* loop,
            const InetAddress& serverAddr, bool retry
			, int retryIntervalMs, int timeout)
			: _name(name),
			_loop(loop),
			_connection(NULL),
			_connectionCallback(NULL),
			_messageCallback(NULL),
			_writeCompleteCallback(NULL),
			_connectExceptionCallback(NULL),
			_retry(retry),
			_connect(false),
			_nextConnId(1)
{
	_connector = new TcpConnector(loop, serverAddr, retry, retryIntervalMs, timeout
		, std::bind(&TcpClient::newConnection, this, std::placeholders::_1)
		, std::bind(&TcpClient::connectionException, this, std::placeholders::_1));
}

TcpClient::~TcpClient(void)
{
	assert(_loop->isInLoopThread());

	if (_connection)
	{
		delete _connection;
	}
	delete _connector;
}

void TcpClient::connect()
{
	if (_connect)
	{
		_loop->queueInLoop(
			std::bind(&TcpClient::reconnectInLoop, this));
	}
	else
	{
		_connect = true;
		_connector->start();
	}
}

void TcpClient::reconnectInLoop()
{
	disconnectInLoop();
	_connector->start();
}

void TcpClient::disconnect()
{
  _connect = false;
  
  _loop->queueInLoop(
	  std::bind(&TcpClient::disconnectInLoop, this));
}

void TcpClient::disconnectInLoop()
{
	if (_connection)
	{
		_connection->close();
	}
	else
	{
		_connector->stop();
	}
}

void TcpClient::newConnection(SOCKET sockfd)
{
	assert(_loop->isInLoopThread());

	InetAddress peerAddr(sockets::getPeerAddr(sockfd));
	//InetAddress localAddr(sockets::getLocalAddr(sockfd));
	
	//fixme
	++_nextConnId;
	
	TcpConnectionPtr conn = new TcpConnection(_loop, _nextConnId, sockfd, peerAddr);
	conn->setConnectEstablishedCallback(_connectionCallback);
	conn->setMessageCallback(_messageCallback);
	conn->setWriteCompleteCallback(_writeCompleteCallback);
	conn->setCloseCallback(
		std::bind(&TcpClient::removeConnection, this,  std::placeholders::_1)); // FIXME: unsafe
	
	_connection = conn;
	conn->connectEstablished();
}


void TcpClient::removeConnection(const TcpConnectionPtr& conn)
{
	assert(_loop->isInLoopThread());
	assert(_loop == conn->getLoop());

	_connection->connectDestroyed();
	if (_connection)
	{
		delete _connection;
		_connection = NULL;
	}

	//if (_retry && _connect)
	//{
	//	_connector->start();
	//}
}

void TcpClient::connectionException(int errorcode)
{
	if (_connectExceptionCallback)
	{
		_connectExceptionCallback(_name.c_str(), errorcode);
	}
}


}