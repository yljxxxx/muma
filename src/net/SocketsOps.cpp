#include "..\common.h"
#include "../../include/net/SocketsOps.h"

namespace muma
{
	static bool kInitSockLib = false;

	void sockets::InitSockLib()
	{
		if(!kInitSockLib)
		{
			WORD wVersionRequested = MAKEWORD(2, 2);
			WSADATA wsaData;
			WSAStartup(wVersionRequested, &wsaData);
			kInitSockLib = true;
		}
	}

	void sockets::ReleaseSockLib()
	{
		if(kInitSockLib)
		{
			WSACleanup();
			kInitSockLib = false;
		}
	}
	
	SOCKET sockets::createNonblockingTcpSocket()
	{
		SOCKET tcpSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if(tcpSocket == INVALID_SOCKET)
			return INVALID_SOCKET;

		unsigned long bIsFIONBIO = 1;
		ioctlsocket(tcpSocket, FIONBIO, &bIsFIONBIO);		//ÖÃÎª·Ç×èÈû

		return tcpSocket;
	}

	SOCKET sockets::createNonblockingUdpSocket()
	{
		SOCKET udpSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		if(udpSocket == INVALID_SOCKET)
			return INVALID_SOCKET;

		unsigned long bIsFIONBIO = 1;
		ioctlsocket(udpSocket, FIONBIO, &bIsFIONBIO);		

		return udpSocket;
	}

	int sockets::bindAddress(SOCKET sockfd, const char* ip, uint16_t port)
	{
		sockaddr_in addr;
		memset(&addr, 0, sizeof(addr));
		addr.sin_family = AF_INET;

		if(NULL == ip || strlen(ip) == 0)
			addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
		else
			addr.sin_addr.S_un.S_addr = ::inet_addr(ip);

		addr.sin_port = htons(port);

		return ::bind(sockfd,  (sockaddr*)&addr, sizeof(addr));
	}

	int  sockets::connect(SOCKET sockfd, const struct sockaddr_in& addr)
	{
		return ::connect(sockfd, (sockaddr*)&addr, sizeof(sockaddr_in));
	}

	void sockets::enableBlocking(SOCKET sockfd)
	{
		unsigned long bIsFIONBIO = 0;
		ioctlsocket(sockfd, FIONBIO, &bIsFIONBIO);
	}

	void sockets::setKeepAlive(SOCKET sockfd, bool on)
	{
		BOOL optval = on ? TRUE : FALSE;
		::setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE,
			(char*)(&optval), sizeof(optval));
	}

	void sockets::setTcpNoDelay(SOCKET sockfd, bool on)
	{
		int optval = on ? 1 : 0;
		::setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY,
			(char*)(&optval), sizeof(optval));
	}

	void sockets::setRecvBufSize(SOCKET sockfd, int size)
	{
		setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF , (char*)&size, sizeof(int));
	}

	int32_t sockets::read(SOCKET sockfd, char *buf, size_t count)
	{
		return ::recv(sockfd, buf, count, 0);
	}


	int32_t sockets::readv(SOCKET sockfd, WSABUF* bufs, int bufCount)
	{
		DWORD recvdBytes = 0;
		DWORD flag = 0;
		int ret = ::WSARecv(sockfd, bufs, bufCount, &recvdBytes, &flag, NULL, NULL);
		if (ret == 0) 
		{
			return recvdBytes;
		}
		else 
		{
			return -1;
		}
	}

	int32_t sockets::readfromv(SOCKET sockfd, WSABUF* bufs, int bufCount, sockaddr_in* addr)
	{
		DWORD recvBytes = 0;
		DWORD flag = 0;

		sockaddr* saddr = NULL;
		int addrlen = 0;
		if(addr != NULL)
		{
			saddr = (sockaddr*)addr;
			addrlen = sizeof(addr);
		}

		::WSARecvFrom(sockfd, bufs, bufCount, &recvBytes, &flag, saddr, &addrlen, NULL, NULL); 
		return recvBytes;
	}

	int32_t sockets::write(SOCKET sockfd, const void *buf, size_t count)
	{
		return ::send(sockfd, (const char*)buf, count, 0);
	}

	int32_t sockets::writeto(SOCKET sockfd, const void *buf, size_t count, const struct sockaddr_in& addr)
	{
		return ::sendto(sockfd, (char*)buf, count, 0, (sockaddr*)&addr, sizeof(sockaddr_in));
	}

	void sockets::shutdownWrite(SOCKET sockfd)
	{
		::shutdown(sockfd, SD_SEND);
	}

	struct sockaddr_in sockets::getLocalAddr(SOCKET sockfd)
	{
		struct sockaddr_in localaddr;
		int addrlen = sizeof(localaddr);
		memset(&localaddr, 0, addrlen);
		
		::getsockname(sockfd, (sockaddr*)(&localaddr), &addrlen);
		return localaddr;
	}

	struct sockaddr_in sockets::getPeerAddr(SOCKET sockfd)
	{
		struct sockaddr_in peeraddr;
		int addrlen = sizeof(peeraddr);
		memset(&peeraddr, 0, addrlen);
		
		::getpeername(sockfd, (sockaddr*)(&peeraddr), &addrlen);

		return peeraddr;
	}

	int sockets::selectWrite(SOCKET sockfd, uint32_t millisecs)
	{
		fd_set set;
		FD_ZERO(&set);
		FD_SET(sockfd, &set);

		struct timeval timeout;
		timeout.tv_sec = millisecs/1000;
		timeout.tv_usec = (millisecs%1000) * 1000;

		int ret = ::select(0, NULL, &set, NULL, &timeout);

		return (ret > 0) ? 0 : -1;
	}

	int sockets::selectRead(SOCKET sockfd, uint32_t millisecs)
	{
		fd_set set;
		FD_ZERO(&set);
		FD_SET(sockfd, &set);

		struct timeval timeout;
		timeout.tv_sec = millisecs/1000;
		timeout.tv_usec = (millisecs%1000) * 1000;

		int ret = ::select(0, &set, NULL, NULL, &timeout);

		return (ret > 0) ? 0 : -1;
	}

}
