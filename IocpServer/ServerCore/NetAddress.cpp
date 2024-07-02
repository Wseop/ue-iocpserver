#include "pch.h"
#include "NetAddress.h"

NetAddress::NetAddress()
{
}

NetAddress::NetAddress(SOCKADDR_IN sockAddr) :
	_sockAddr(sockAddr)
{
}

NetAddress::NetAddress(string ip, uint16 port)
{
	::memset(&_sockAddr, 0, sizeof(_sockAddr));
	_sockAddr.sin_family = AF_INET;
	_sockAddr.sin_addr = ip2Address(ip.c_str());
	_sockAddr.sin_port = ::htons(port);
}

string NetAddress::getIpAddress()
{
	char buffer[100];
	::InetNtopA(AF_INET, &_sockAddr.sin_addr, buffer, sizeof(buffer) / sizeof(WCHAR));
	return string(buffer);
}

IN_ADDR NetAddress::ip2Address(const char* ip)
{
	IN_ADDR address;
	::InetPtonA(AF_INET, ip, &address);
	return address;
}
