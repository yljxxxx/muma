#ifndef __T__MUMA_INETADDRESS_H__
#define __T__MUMA_INETADDRESS_H__

#include <string>


namespace muma
{


class InetAddress
{
public:
	explicit InetAddress(uint16_t port);
	InetAddress(const char* ip, uint16_t port);

	 InetAddress(const struct sockaddr_in& addr)
    : _addr(addr)
	{ }

	 std::string toIpPort() const;
	 void toIpPort(char* ipPortBuf) const;
	 void toIpPort(char* ipBuf, unsigned short& port) const;

	 uint32_t ipNetEndian() const { return _addr.sin_addr.s_addr; }
	 uint16_t portNetEndian() const { return _addr.sin_port; }

	 const struct sockaddr_in& getSockAddrInet() const { return _addr; }
	 void setSockAddrInet(const struct sockaddr_in& addr) { _addr = addr; }

	 static void ToIpv4(uint32_t ip, char* strIpBuf);
	 static void ToSockAddrInet(const char* ip, uint16_t port, sockaddr_in& saddr);

private:
	struct sockaddr_in _addr;
};


}


#endif
