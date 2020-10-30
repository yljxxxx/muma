#include "..\common.h"
#include "../../include/net/TcpSocket.h"

namespace muma
{

TcpSocket::TcpSocket()
    : _sockfd(socket(AF_INET, SOCK_STREAM, IPPROTO_TCP))
{ 
}

TcpSocket::TcpSocket(SOCKET fd)
	: _sockfd(fd)
{
}

TcpSocket::~TcpSocket(void)
{
	if(_sockfd != INVALID_SOCKET)
	{
		closesocket(_sockfd);
	}
}

int TcpSocket::bindAddress(const InetAddress& addr)
{
	int ret = ::bind(_sockfd, (sockaddr*)(&addr.getSockAddrInet()), sizeof(addr.getSockAddrInet()));
	if(ret != 0)
		return -1;

	return 0;
}


int TcpSocket::listen()
{
	int ret = ::listen(_sockfd, SOMAXCONN);
	if (ret != 0)
		return -1;

	return 0;
}


SOCKET TcpSocket::accept(InetAddress* peeraddr)
{
	SOCKET sockfd = INVALID_SOCKET;

	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));

	int addrlen=sizeof(addr);
	sockfd = ::accept(_sockfd, (sockaddr*)(&addr), &addrlen);
	if(sockfd != INVALID_SOCKET)
		peeraddr->setSockAddrInet(addr);

	return sockfd;
}

void TcpSocket::shutdownWrite()
{
	::shutdown(_sockfd, SD_SEND);
}

void TcpSocket::setNonblocking(bool on)
{
	unsigned long bIsFIONBIO = on ? 1 : 0;
	::ioctlsocket(_sockfd, FIONBIO, &bIsFIONBIO);	
}

void TcpSocket::setTcpNoDelay(bool on)
{
  int optval = on ? 1 : 0;
  ::setsockopt(_sockfd, IPPROTO_TCP, TCP_NODELAY,
               (char*)(&optval), sizeof(optval));
}

void TcpSocket::setReuseAddr(bool on)
{
  BOOL optval = on ? TRUE : FALSE;
  ::setsockopt(_sockfd, SOL_SOCKET, SO_REUSEADDR,
               (char*)(&optval), sizeof(optval));
}

void TcpSocket::setKeepAlive(bool on)
{
  BOOL optval = on ? TRUE : FALSE;
  ::setsockopt(_sockfd, SOL_SOCKET, SO_KEEPALIVE,
               (char*)(&optval), sizeof(optval));
}

int TcpSocket::connect(const InetAddress& serveraddr, uint32_t millisecs)
{
	const sockaddr_in& add = serveraddr.getSockAddrInet();
	int ret = ::connect(_sockfd, (sockaddr*)&add, sizeof(sockaddr_in));
	if(ret == 0)
		return 0;

	if(::WSAGetLastError() != WSAEWOULDBLOCK)
		return -1;

	return selectWrite(millisecs);
}



int TcpSocket::send(const char* data, size_t len, uint32_t millisecs)
{
	int totalSendBytes = 0;
	int sendbytes = 0;

	if(0 == millisecs)
	{
		sendbytes = ::send(_sockfd, data, len, 0);
		if(sendbytes > 0)
		{
			totalSendBytes = sendbytes;
		}
	}
	else
	{
		while(totalSendBytes < len)
		{
			if(selectWrite(millisecs) != 0)
				break;

			sendbytes = ::send(_sockfd, data+totalSendBytes, len-totalSendBytes, 0);
			if(sendbytes <= 0)
				break;

			totalSendBytes += sendbytes;
		}
	}
	
	return totalSendBytes;
}


int TcpSocket::recv(char* buf, size_t len, uint32_t millisecs, bool waitall)
{
	int totalrecvbytes = 0;
	do
	{
		if(selectRead(millisecs) != 0)
			return -1;

		int recvbytes = ::recv(_sockfd, buf+totalrecvbytes, len-totalrecvbytes, 0);
		if(recvbytes <= 0)
			return -1;

		totalrecvbytes += recvbytes;
	}while(waitall && totalrecvbytes < len);

	return totalrecvbytes;
}


int TcpSocket::selectWrite(uint32_t millisecs)
{
	fd_set set;
	FD_ZERO(&set);
	FD_SET(_sockfd, &set);

	struct timeval timeout;
	timeout.tv_sec = millisecs/1000;
	timeout.tv_usec = (millisecs%1000) * 1000;

	int ret = ::select(0, NULL, &set, NULL, &timeout);
	
	return (ret > 0) ? 0 : -1;
}

int TcpSocket::selectRead(uint32_t millisecs)
{
	fd_set set;
	FD_ZERO(&set);
	FD_SET(_sockfd, &set);

	struct timeval timeout;
	timeout.tv_sec = millisecs/1000;
	timeout.tv_usec = (millisecs%1000) * 1000;

	int ret = ::select(0, &set, NULL, NULL, &timeout);
	
	return (ret > 0) ? 0 : -1;
}

int TcpSocket::post(const InetAddress& severAddr, const char* data, size_t dataLen
		, char* recvBuf, size_t recvBufSize, size_t* pRecvDataLen, bool waitall
		, uint32_t connMillsecs, uint32_t sendMillsecs, uint32_t recvMillsecs)
{
	muma::TcpSocket tcpSocket;
	if(tcpSocket.connect(severAddr, connMillsecs) != 0)
		return -1;

	if(tcpSocket.send(data, dataLen, sendMillsecs) != dataLen)
		return -1;

	if(recvBuf != NULL && recvBufSize > 0)
	{
		int recvRet = tcpSocket.recv(recvBuf, recvBufSize, recvMillsecs, waitall);
		if(recvRet < 0)
			return -1;

		if(pRecvDataLen != NULL)
			*pRecvDataLen = recvRet;
	}

	tcpSocket.shutdownWrite();
	return 0;
}

}