
#include "../common.h"
#include "../../include/net/TcpConnector.h"
#include "../../include/net/EventLoop.h"
#include "../../include/net/SocketsOps.h"

using namespace muma;

TcpConnector::TcpConnector(EventLoop* loop, const InetAddress& serverAddr
	, bool isretry, int retryIntervalMs, int timeout
	, const NewConnectionCallback& newconnCb
	, const ConnectExceptionMsgCallback& connExcptCb)
	: loop_(loop),
	serverAddr_(serverAddr),
	state_(kDisconnected),
	isretry(false),
    retryDelayMs_(retryIntervalMs),
	timeout_(timeout),
	newConnectionCallback_(newconnCb),
	connectExcptMsgCallback_(connExcptCb),
	channel_(NULL)
{
	if (timeout_ == 0)
		timeout_ = 10000; //默认超时10s
}

TcpConnector::~TcpConnector()
{
	//必须在Loop中delete
	assert(loop_->isInLoopThread());
	stopInLoop();
}

void TcpConnector::start()
{
  loop_->runInLoop(std::bind(&TcpConnector::startInLoop, this)); 
}

void TcpConnector::startInLoop()
{
	assert(loop_->isInLoopThread());
	//assert(state_ == kDisconnected);
	if (state_ == kDisconnected)
	{
		connect();
	}
	else if (connectExcptMsgCallback_)
	{
		ConnectionErrorcode errorcode = connect_error_unknown;
		switch (state_)
		{
		case kConnecting:
			errorcode = connect_connecting;
			break;
		case kConnected:
			errorcode = connect_connected;
			break;
		}

		connectExcptMsgCallback_(errorcode);
			
	}
}

void TcpConnector::stop()
{
  loop_->queueInLoop(std::bind(&TcpConnector::stopInLoop, this)); 
}

void TcpConnector::stopInLoop()
{
  assert(loop_->isInLoopThread());
  
  if (timeid_.IsValid())
  {
	  loop_->cancel(timeid_);
	  timeid_.SetInvalid();
  }
  cancelCheckTimeoutInLoop();

  if (channel_)
  {
	  SOCKET s = channel_->socketHandle();
	  loop_->DeleteChannel(channel_);

	  if(!isSocketTransfer_)  //socket未移交所有权
		sockets::close(s);

	  channel_ = NULL;
  }
  setState(kDisconnected);
}

void TcpConnector::restart()
{
	loop_->queueInLoop(std::bind(&TcpConnector::restartInLoop, this));
}

void TcpConnector::restartInLoop()
{
	stopInLoop();
	startInLoop();
}

void TcpConnector::connect()
{
  SOCKET sockfd = sockets::createNonblockingTcpSocket();
  int ret = sockets::connect(sockfd, serverAddr_.getSockAddrInet());
  if (ret == 0 || sockets::getLastError() == WSAEWOULDBLOCK)
  {
	  setState(kConnecting);
	  assert(!channel_);
	  channel_ = loop_->NewChannel(sockfd, NULL, Channel::FD_OF_SOCKET);
	  isSocketTransfer_ = false;

	  channel_->setWriteCallback(
		  std::bind(&TcpConnector::handleWrite, this));
	  channel_->setErrorCallback(
		  std::bind(&TcpConnector::handleError, this));

	  channel_->enableWriting();

	  timeidForTimeout_ = loop_->runAfter(timeout_,
		  std::bind(&TcpConnector::checkTimeoutInLoop, this));

  }
  else if (isretry)
  {
	  if (connectExcptMsgCallback_)
	  {
		  connectExcptMsgCallback_(connect_failed_and_willRetry);
	  }
	  retry(sockfd);
  }
  else
  {
	  sockets::close(sockfd);
	  if (connectExcptMsgCallback_)
	  {
		  connectExcptMsgCallback_(connect_failed);
	  }
  }
}

SOCKET TcpConnector::removeAndResetChannel()
{
  channel_->disableAll();
  channel_->remove();
  SOCKET sockfd = channel_->socketHandle();
  isSocketTransfer_ = true;
  // Can't reset channel_ here, because we are inside Channel::handleEvent
  loop_->queueInLoop(std::bind(&TcpConnector::resetChannel, this)); // FIXME: unsafe
  return sockfd;
}

void TcpConnector::resetChannel()
{
	loop_->DeleteChannel(channel_);
	channel_ = NULL;
}

void TcpConnector::handleWrite()
{
	SOCKET sockfd = removeAndResetChannel();
	setState(kConnected);
	newConnectionCallback_(sockfd);
	cancelCheckTimeoutInLoop();
}

void TcpConnector::handleError()
{
	int sockfd = removeAndResetChannel();
	setState(kDisconnected);
	if (isretry)
	{
		if (connectExcptMsgCallback_)
		{
			connectExcptMsgCallback_(connect_failed_and_willRetry);
		}
		retry(sockfd);
	}
	else
	{
		sockets::close(sockfd);
		if (connectExcptMsgCallback_)
		{
			connectExcptMsgCallback_(connect_failed);
		}
	}
}

void TcpConnector::retry(int sockfd)
{
  sockets::close(sockfd);
  assert(state_ == kDisconnected);
  setState(kDisconnected);
  timeid_ = loop_->runAfter(retryDelayMs_,
	  std::bind(&TcpConnector::retryInLoop, this));
}

void TcpConnector::retryInLoop()
{
	startInLoop();
	timeid_.SetInvalid();
}

void TcpConnector::checkTimeoutInLoop()
{
	if (state_ != kConnected)
	{
		if (connectExcptMsgCallback_)
		{
			connectExcptMsgCallback_(connect_failed_timeout);
		}
	}
	timeidForTimeout_.SetInvalid();
}

void TcpConnector::cancelCheckTimeoutInLoop()
{
	if (timeidForTimeout_.IsValid())
	{
		loop_->cancel(timeidForTimeout_);
		timeidForTimeout_.SetInvalid();
	}
}