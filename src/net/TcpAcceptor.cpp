#include "..\common.h"
#include "../../include/net/TcpAcceptor.h"

namespace muma
{

TcpAcceptor::TcpAcceptor(EventLoop* loop, const InetAddress& listenAddr, bool reuseport)
  : _loop(loop),
    _isListenning(false)
{
	_acceptChannel = _loop->NewChannel(_acceptSocket.fd(), NULL, Channel::FD_OF_SOCKET);
	_acceptSocket.setNonblocking(true);
	_acceptSocket.setReuseAddr(reuseport);
	_acceptSocket.bindAddress(listenAddr);
	
	_acceptChannel->setAcceptCallback(
		std::bind(&TcpAcceptor::handleAccept, this));
}

TcpAcceptor::~TcpAcceptor()
{
	assert(_loop->isInLoopThread());
	_loop->DeleteChannel(_acceptChannel);
}

void TcpAcceptor::listen()
{
	assert(_loop->isInLoopThread());

	_isListenning = true;
	_acceptChannel->enableAccepting();
	_acceptSocket.listen();

}

void TcpAcceptor::handleAccept()
{
	InetAddress peerAddr(0);
	
	SOCKET connfd = _acceptSocket.accept(&peerAddr);
	if (connfd != INVALID_SOCKET)
	{
		if (_newConnectionCallback)
		{
			_newConnectionCallback(connfd, peerAddr);
		}
		else
		{
			::closesocket(connfd);
		}
	}
	else
	{
		//error
	}
}


}
