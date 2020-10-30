#ifndef __T__MUMA_SOCKETS_H__
#define __T__MUMA_SOCKETS_H__


namespace muma
{


namespace sockets
{
	void InitSockLib();
	void ReleaseSockLib();

	SOCKET createNonblockingTcpSocket();
	SOCKET createNonblockingUdpSocket();
	
	int bindAddress(SOCKET sockfd, const char* ip, uint16_t port);

	int connect(SOCKET sockfd, const struct sockaddr_in& addr);
	inline void close(SOCKET sockfd){ 
		::closesocket(sockfd);
	}
	
	void enableBlocking(SOCKET sockfd);
	void setKeepAlive(SOCKET sockfd, bool on);
	void setTcpNoDelay(SOCKET sockfd, bool on);
	void setRecvBufSize(SOCKET sockfd, int size);

	int32_t read(SOCKET sockfd, char *buf, size_t count);
	int32_t readv(SOCKET sockfd, WSABUF* bufs, int bufCount);
	int32_t readfromv(SOCKET sockfd, WSABUF* bufs, int bufCount, sockaddr_in* addr);
	
	int32_t write(SOCKET sockfd, const void *buf, size_t count);
	int32_t writeto(SOCKET sockfd, const void *buf, size_t count, const struct sockaddr_in& addr);

	void shutdownWrite(SOCKET sockfd);

	inline int getLastError(){ return ::WSAGetLastError(); }

	struct sockaddr_in getLocalAddr(SOCKET sockfd);
	struct sockaddr_in getPeerAddr(SOCKET sockfd);

	int selectWrite(SOCKET sockfd, uint32_t millisecs);
	int selectRead(SOCKET sockfd, uint32_t millisecs);

	
}


}


#endif