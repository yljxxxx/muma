#ifndef __T__MUMA_TCPCONNECTION_H__
#define __T__MUMA_TCPCONNECTION_H__

#include <memory>
#include <string>

#include "Callbacks.h"

namespace muma
{
	class EventLoop;
	class InetAddress;
	class TcpSocket;
	class Channel;

class TcpConnection 
{
public:
	/// Constructs a TcpConnection with a connected sockfd
	/// User should not create this object.
	TcpConnection(EventLoop* loop,
		int64_t id,
		SOCKET sockfd,
		const InetAddress& peerAddr);

	~TcpConnection(void);

	EventLoop* getLoop() const { return _loop; }
	const int64_t id() const { return _id; }

	const InetAddress& localAddress() { return *_localAddr; }
	const InetAddress& peerAddress() { return *_peerAddr; }
	bool connected() const { return _state == kConnected; }


	void sendInLoop(const void* data, size_t len, std::function<void(bool)> cb);
	
	void close(); // NOT thread safe, no simultaneous calling
	void setTcpNoDelay(bool on);

	void setConnectEstablishedCallback(const ConnectionCallback& cb)
	{ _connectEstablishedCallback = cb; }

	void setConnectDestroyedCallback(const ConnectionCallback& cb)
	{ _connectDestroyedCallback = cb; }

	void setMessageCallback(const MessageCallback& cb)
	{ _messageCallback = cb; }

	void setWriteCompleteCallback(const WriteCompleteCallback& cb)
	{ _writeCompleteCallback = cb; }

	void setHighWaterMarkCallback(const HighWaterMarkCallback& cb, size_t highWaterMark)
	{ _highWaterMarkCallback = cb; _highWaterMark = highWaterMark; }

	void setUserData(void* userdata){ _userdata = userdata; }
	void* getUserData(){ return _userdata; }

	/// Internal use only.
	void setCloseCallback(const CloseCallback& cb)
	{ _closeCallback = cb; }

	 // called when TcpServer accepts a new connection
	void connectEstablished();   // should be called only once
	// called when TcpServer has removed me from its map
	void connectDestroyed();  // should be called only once

private:
	void handleRead();
	void handleWrite();
	void handleClose();
	void handleError();

	void sendInLoopFunc(const void* data, size_t len, std::function<void(bool)> cb);
	//void sendStringInLoop(std::string data);    //fix me
	
private:
	enum ConnState { kDisconnected, kConnecting, kConnected, kDisconnecting};
	
	EventLoop* _loop;
	int64_t _id;
	ConnState _state;    // FIXME: use atomic variable

	std::unique_ptr<TcpSocket> _socket;
	Channel* _channel;

	std::unique_ptr<InetAddress> _localAddr;
	std::unique_ptr<InetAddress> _peerAddr;

	ConnectionCallback _connectEstablishedCallback;
	ConnectionCallback _connectDestroyedCallback;
	MessageCallback _messageCallback;
	WriteCompleteCallback _writeCompleteCallback;
	HighWaterMarkCallback _highWaterMarkCallback;
	CloseCallback _closeCallback;

	size_t _highWaterMark;
	std::unique_ptr<Buffer> _inputBuffer;
	std::unique_ptr<Buffer> _outputBuffer; // FIXME: use list<Buffer> as output buffer.

	//std::vector<Buffer*> _freeBuflist;
	//std::vector<Buffer*> _outputBufferList;

	void* _userdata;

	
};


}


#endif