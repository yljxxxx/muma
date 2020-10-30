#include "../common.h"
#include "../../include/net/Channel.h"
#include "../../include/net/EventLoop.h"

#include <functional>

namespace muma
{


Channel::Channel(EventLoop* loop, SOCKET socket, HANDLE handle, FD_TYPE fdtype)
  : _loop(loop),
	_socket(socket),
	_handle(handle),
	_fdtype(fdtype),
	_events(NONE_EVENT),
	_revents(NONE_EVENT),
	_indexForPoll(EventLoop::kInvalidChannelIndex),
	_indexForLoop(EventLoop::kInvalidChannelIndex),
	_readCallback(NULL),
	_writeCallback(NULL),
	_closeCallback(NULL),
	_acceptCallback(NULL),
	_errorCallback(NULL)
{
}

Channel::~Channel(void)
{
}

/*void Channel::handleEvent()
{
	if (_loop->isInLoopThread())
	{
		handleEventFunc();
	}
	else 
	{
		_loop->queueInLoop(std::bind(&Channel::handleEventFunc, this));
	}
}*/

void Channel::handleEvent() {

	assert(_loop->isInLoopThread());

	if (_revents & CLOSE_EVENT)
	{
		if (_closeCallback != NULL) _closeCallback();
	}

	if (_revents & READ_EVENT)
	{
		if (_readCallback != NULL) _readCallback();
	}

	if (_revents & WRITE_EVENT)
	{
		if (_writeCallback != NULL) _writeCallback();
	}

	if (_revents & ACCEPT_EVENT)
	{
		if (_acceptCallback != NULL) _acceptCallback();
	}

	if ((_revents & ERROR_EVENT))
	{
		if (_errorCallback != NULL) _errorCallback();
	}
}

void Channel::setReadCallback(const EventCallback& cb) 
{
	if (_loop->isInLoopThread())
	{
		setReadCallbackFunc(cb);
	}
	else
	{
		_loop->queueInLoop(std::bind(&Channel::setReadCallbackFunc, this, cb));
	}
}
void Channel::setWriteCallback(const EventCallback& cb)
{
	if (_loop->isInLoopThread())
	{
		setWriteCallbackFunc(cb);
	}
	else
	{
		_loop->queueInLoop(std::bind(&Channel::setWriteCallbackFunc, this, cb));
	}
}
void Channel::setCloseCallback(const EventCallback& cb)
{
	if (_loop->isInLoopThread())
	{
		setCloseCallbackFunc(cb);
	}
	else
	{
		_loop->queueInLoop(std::bind(&Channel::setCloseCallbackFunc, this, cb));
	}
}
void Channel::setAcceptCallback(const EventCallback& cb)
{
	if (_loop->isInLoopThread())
	{
		setAcceptCallbackFunc(cb);
	}
	else
	{
		_loop->queueInLoop(std::bind(&Channel::setAcceptCallbackFunc, this, cb));
	}
}
void Channel::setErrorCallback(const EventCallback& cb)
{
	if (_loop->isInLoopThread())
	{
		setErrorCallbackFunc(cb);
	}
	else
	{
		_loop->queueInLoop(std::bind(&Channel::setErrorCallbackFunc, this, cb));
	}
}

void Channel::enableReading() 
{ 
	if (_loop->isInLoopThread())
	{
		enableReadingFunc();
	}
	else {
		_loop->queueInLoop(std::bind(&Channel::enableReadingFunc, this));
	}
}

void Channel::enableWriting() 
{
	if (_loop->isInLoopThread())
	{
		enableWritingFunc();
	}
	else {
		_loop->queueInLoop(std::bind(&Channel::enableWritingFunc, this));
	}
}
void Channel::enableAccepting()
{
	if (_loop->isInLoopThread())
	{
		enableAcceptingFunc();
	}
	else {
		_loop->queueInLoop(std::bind(&Channel::enableAcceptingFunc, this));
	}
}
void Channel::enableClosing()
{
	if (_loop->isInLoopThread())
	{
		enableClosingFunc();
	}
	else {
		_loop->queueInLoop(std::bind(&Channel::enableClosingFunc, this));
	}
}

void Channel::disableReading()
{
	if (_loop->isInLoopThread())
	{
		disableReadingFunc();
	}
	else {
		_loop->queueInLoop(std::bind(&Channel::disableReadingFunc, this));
	}
}
void Channel::disableWriting()
{
	if (_loop->isInLoopThread())
	{
		disableWritingFunc();
	}
	else {
		_loop->queueInLoop(std::bind(&Channel::disableWritingFunc, this));
	}
}
void Channel::disableAccepting()
{
	if (_loop->isInLoopThread())
	{
		disableAcceptingFunc();
	}
	else {
		_loop->queueInLoop(std::bind(&Channel::disableAcceptingFunc, this));
	}
}
void Channel::disableClosing()
{
	if (_loop->isInLoopThread())
	{
		disableClosingFunc();
	}
	else {
		_loop->queueInLoop(std::bind(&Channel::disableClosingFunc, this));
	}
}
void Channel::disableAll()
{
	if (_loop->isInLoopThread())
	{
		disableAllFunc();
	}
	else {
		_loop->queueInLoop(std::bind(&Channel::disableAllFunc, this));
	}
}

void Channel::remove()
{
	if (_loop->isInLoopThread())
	{
		removeFunc();
	}
	else
	{
		_loop->queueInLoop(std::bind(&Channel::removeFunc, this));
	}
}

void Channel::setReadCallbackFunc(const EventCallback& cb)
{
	_readCallback = cb;
}

void Channel::setWriteCallbackFunc(const EventCallback& cb)
{
	_writeCallback = cb;
}

void Channel::setCloseCallbackFunc(const EventCallback& cb)
{
	_closeCallback = cb;
}

void Channel::setAcceptCallbackFunc(const EventCallback& cb)
{
	_acceptCallback = cb;
}

void Channel::setErrorCallbackFunc(const EventCallback& cb)
{
	_errorCallback = cb;
}

void Channel::enableReadingFunc() 
{
	_events |= READ_EVENT;
	_loop->updateForChannel(this);
}

void Channel::enableWritingFunc() 
{ 
	_events |= WRITE_EVENT; 
	_loop->updateForChannel(this);
}

void Channel::enableAcceptingFunc()
{
	_events |= ACCEPT_EVENT;
	_loop->updateForChannel(this);
}
	
void Channel::enableClosingFunc()
{
	_events |= CLOSE_EVENT; 
	_loop->updateForChannel(this);
}

void Channel::disableReadingFunc()
{ 
	_events &= ~READ_EVENT; 
	_loop->updateForChannel(this);
}
		
void Channel::disableWritingFunc()
{ 
	_events &= ~WRITE_EVENT; 
	_loop->updateForChannel(this);
}

void Channel::disableAcceptingFunc()
{
	_events &= ~ACCEPT_EVENT; 
	_loop->updateForChannel(this);
}

void Channel::disableClosingFunc()
{
	_events &= ~CLOSE_EVENT; 
	_loop->updateForChannel(this);
}

void Channel::disableAllFunc()
{ 
	_events = NONE_EVENT; 
	_loop->updateForChannel(this);
}

void Channel::removeFunc()
{
	_loop->removeForChannel(this);
}

}