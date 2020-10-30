#include "..\common.h"
#include "../../include/net/InetAddress.h"

namespace muma
{


InetAddress::InetAddress(uint16_t port)
{
  memset(&_addr, 0, sizeof(_addr));
  _addr.sin_family = AF_INET;
  _addr.sin_addr.s_addr = htonl(INADDR_ANY);
  _addr.sin_port = htons(port);
}


InetAddress::InetAddress(const char* ip, uint16_t port)
{
  memset(&_addr, 0, sizeof(_addr));
  _addr.sin_family = AF_INET;
  _addr.sin_addr.S_un.S_addr = inet_addr(ip);
  _addr.sin_port = htons(port);
  
}

std::string InetAddress::toIpPort() const
{
	char strTemp[64];
	::sprintf(strTemp, "%s:%d", ::inet_ntoa(_addr.sin_addr), ntohs(_addr.sin_port));

	return strTemp;
}

void InetAddress::toIpPort(char* ipPortBuf) const
{
	::sprintf(ipPortBuf, "%s:%d", ::inet_ntoa(_addr.sin_addr), ntohs(_addr.sin_port));
}

void InetAddress::toIpPort(char* ipBuf, unsigned short& port) const
{
	strcpy(ipBuf, ::inet_ntoa(_addr.sin_addr));
	port = ::ntohs(_addr.sin_port);
}

void InetAddress::ToIpv4(uint32_t ip, char* strIpBuf)
{
	in_addr addr;
	addr.S_un.S_addr = ip;
	strcpy(strIpBuf, ::inet_ntoa(addr));
}

void InetAddress::ToSockAddrInet(const char* ip, uint16_t port, sockaddr_in& saddr)
{
	memset(&saddr, 0, sizeof(sockaddr_in));
	saddr.sin_family = AF_INET;
	saddr.sin_addr.S_un.S_addr = inet_addr(ip);
	saddr.sin_port = htons(port);	
}

}