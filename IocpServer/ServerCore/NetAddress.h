#pragma once

class NetAddress
{
public:
	NetAddress();
	NetAddress(SOCKADDR_IN sockAddr);
	NetAddress(string ip, uint16 port);

	SOCKADDR_IN&	getSockAddr() { return _sockAddr; }
	string			getIpAddress();
	uint16			getPort() { return ::ntohs(_sockAddr.sin_port); }

public:
	static IN_ADDR ip2Address(const char* ip);

private:
	SOCKADDR_IN _sockAddr;
};

