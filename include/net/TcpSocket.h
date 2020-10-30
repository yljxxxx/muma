#ifndef __T__MUMA_SOCKET_H__
#define __T__MUMA_SOCKET_H__

#include "../../include/net/InetAddress.h"

namespace muma
{


class TcpSocket
{
public:
	TcpSocket();
	TcpSocket(SOCKET fd);

	~TcpSocket();

	static int post(const InetAddress& severAddr, const char* data, size_t dataLen
		, char* recvBuf, size_t recvBufSize, size_t* pRecvDataLen, bool waitall
		, uint32_t connMillsecs, uint32_t sendMillsecs, uint32_t recvMillsecs);

	SOCKET fd(){ return _sockfd; }

	int bindAddress(const InetAddress& localaddr);
	
	int listen();

	SOCKET accept(InetAddress* peeraddr);

	void shutdownWrite();

	void setNonblocking(bool on);

	void setTcpNoDelay(bool on);

	void setReuseAddr(bool on);

	void setKeepAlive(bool on);

	int connect(const InetAddress& serveraddr, uint32_t millisecs);

	int send(const char* data, size_t len, uint32_t millisecs = 0);

	int recv(char* buf, size_t len, uint32_t millisecs, bool waitall);
	
private:
	int selectWrite(uint32_t millisecs);
	int selectRead(uint32_t millisecs);

private:
	SOCKET _sockfd;
};


}


#endif