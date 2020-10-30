#include "../common.h"
#include "../../include/net/TcpConnection.h"
#include "../../include/net/SocketsOps.h"
#include "../../include/net/Buffer.h"
#include "../../include/net/Channel.h"
#include "../../include/net/EventLoop.h"
#include "../../include/net/TcpSocket.h"



namespace muma
{


TcpConnection::TcpConnection(EventLoop* loop,
							 int64_t id,
							SOCKET sockfd,
							 const InetAddress& peerAddr)
							 : _loop(loop),
							 _id(id),
							 _state(kConnecting),
							_socket(new TcpSocket(sockfd)),
							 _localAddr(new InetAddress(sockets::getLocalAddr(_socket->fd()))),
							 _peerAddr(new InetAddress(peerAddr)),
							 _highWaterMark(64*1024*1024),
							 _userdata(NULL),
							_inputBuffer(new Buffer(64*1024)),
							_outputBuffer(new Buffer(64 * 1024))
{
	_channel = _loop->NewChannel(_socket->fd(), NULL, Channel::FD_OF_SOCKET);

	_socket->setKeepAlive(true);
	_channel->setReadCallback(
		std::bind(&TcpConnection::handleRead, this));
	_channel->setWriteCallback(
		std::bind(&TcpConnection::handleWrite, this));
	_channel->setCloseCallback(
		std::bind(&TcpConnection::handleClose, this));
	_channel->setErrorCallback(
		std::bind(&TcpConnection::handleError, this));
}

TcpConnection::~TcpConnection(void)
{
	assert(_loop->isInLoopThread());

	if (_state != kDisconnected)
	{
		_channel->remove();
		_socket->shutdownWrite();
	}

	_loop->DeleteChannel(_channel);
}


void TcpConnection::sendInLoop(const void* data, size_t len, std::function<void(bool)> cb)
{
  if (_state == kConnected)
  {
	  if(_loop->isInLoopThread())
	  {
		  sendInLoopFunc(data, len, cb);
	  }
	  else
	  {
		  _loop->queueInLoop(
			  std::bind(&TcpConnection::sendInLoopFunc,
			  this,data, len, cb));     //fix me
	  }
  }
}

//void TcpConnection::sendStringInLoop(std::string data)
//{
//	sendInLoopFunc(data.data(), data.size());
//}

void TcpConnection::sendInLoopFunc(const void* data, size_t len, std::function<void(bool)> cb)
{
	assert(_loop->isInLoopThread());
	if (kConnected != _state)
	{
		if (cb) {
			cb(false);
		}
		return;
	}

	int32_t nwrote = 0;
	size_t remaining = len;
	bool error = false;

	// if no thing in output queue, try writing directly
	if (!_channel->isEnableWriteEvent() && _outputBuffer->readableBytes() == 0)
	{
		nwrote = sockets::write(_socket->fd(), data, len);
		if (nwrote >= 0)
		{
			remaining = len - nwrote;
			if (remaining == 0 && _writeCompleteCallback)
			{
				_loop->queueInLoop(std::bind(_writeCompleteCallback, this));
			}
		}
		else // nwrote < 0
		{
			nwrote = 0;
			if (::WSAGetLastError() != WSAEWOULDBLOCK)
			{
				error = true; 
			}
		}
	}

	assert(remaining <= len);
	if (!error && remaining > 0)
	{
		size_t oldLen = _outputBuffer->readableBytes();
		size_t totalSize = oldLen + remaining;
		if (totalSize >= _highWaterMark
			&& oldLen < _highWaterMark     
			&& _highWaterMarkCallback)
		{
			_loop->queueInLoop(std::bind(_highWaterMarkCallback, this, totalSize));
		}

		_outputBuffer->append(static_cast<const char*>(data)+nwrote, remaining);
		if (!_channel->isEnableWriteEvent())
		{
			_channel->enableWriting();
		}
	}

	if (cb) {
		cb(!error);
	}
}

void TcpConnection::close()
{
	_loop->runInLoop(std::bind(&TcpConnection::handleClose, this));
}

void TcpConnection::setTcpNoDelay(bool on)
{
	_socket->setTcpNoDelay(on);
}


void TcpConnection::connectEstablished()
{
	assert(_loop->isInLoopThread());
	assert(_state == kConnecting);

	_state = kConnected;
	_channel->enableClosing();
	_channel->enableReading();

	if (_connectEstablishedCallback)
		_connectEstablishedCallback(this);
}

void TcpConnection::connectDestroyed()
{
	assert(_loop->isInLoopThread());

	if (_state != kDisconnected)
	{
		_state = kDisconnected;
		_channel->remove();
		_socket->shutdownWrite();
	}

	if(_connectDestroyedCallback)
		_connectDestroyedCallback(this);
}

void TcpConnection::handleRead()
{
	assert(_loop->isInLoopThread());
	if (kConnected != _state)
		return;

	int savedErrno = 0;
	int32_t n = _inputBuffer->readFd(_channel->socketHandle(), &savedErrno);
	if (n > 0)
	{
		_messageCallback(this, _inputBuffer.get());
	}
	else if (n == 0)
	{
		handleClose();
	}
	else
	{
		handleError();
	}
}


void TcpConnection::handleWrite()
{
	assert(_loop->isInLoopThread());
	if (kConnected != _state)
		return;

	if (_channel->isEnableWriteEvent())
	{
		assert(_outputBuffer->readableBytes() > 0);
		int32_t n = sockets::write(_channel->socketHandle(), _outputBuffer->peek(), _outputBuffer->readableBytes());
		if (n > 0)
		{
			_outputBuffer->retrieve(n);
			if (_outputBuffer->readableBytes() == 0)
			{
				_channel->disableWriting();
				if (_writeCompleteCallback)
				{
					_loop->queueInLoop(std::bind(_writeCompleteCallback, this));
				}
			}
			else
			{
			}
		}
		else
		{
			handleError();
		}
	}
}


void TcpConnection::handleClose()
{
	assert(_loop->isInLoopThread());

	if (_state == kConnected)
	{
		_channel->disableAll();
	}

	_state = kDisconnecting;
	 // must be the last line
	_closeCallback(this);
}


void TcpConnection::handleError()
{
	handleClose();
}


}
