#ifndef __T__MUMA_ACCEPTOR_H__
#define __T__MUMA_ACCEPTOR_H__

#include <functional>

#include "../../include/net/InetAddress.h"
#include "../../include/net/Channel.h"
#include "../../include/net/EventLoop.h"
#include "../../include/net/TcpSocket.h"

namespace muma
{


class TcpAcceptor
{
public:
	typedef std::function<void (SOCKET sockfd,
		const InetAddress&)> NewConnectionCallback;

	TcpAcceptor(EventLoop* loop, const InetAddress& listenAddr, bool reuseport);
	~TcpAcceptor();

	void setNewConnectionCallback(const NewConnectionCallback& cb)
	{ _newConnectionCallback = cb; }

	bool isListenning() const { return _isListenning; }
	void listen();

private:
	void handleAccept();

private:
	EventLoop* _loop;
	TcpSocket _acceptSocket;
	Channel* _acceptChannel;

	bool _isListenning;
	NewConnectionCallback _newConnectionCallback;
	
};


}

#endif