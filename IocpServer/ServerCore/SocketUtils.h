#pragma once

#include "NetAddress.h"

class SocketUtils
{
public:
	static LPFN_CONNECTEX		ConnectEx;
	static LPFN_DISCONNECTEX	DisconnectEx;
	static LPFN_ACCEPTEX		AcceptEx;

public:
	static void startUp();
	static void cleanUp();
	
	static SOCKET createSocket();
	static void closeSocket(SOCKET& socket);

	static bool bind(SOCKET socket, NetAddress netAddr);
	static bool bind(SOCKET socket, SOCKADDR_IN netAddr);
	static bool bindAnyAddress(SOCKET socket, uint16 port);
	static bool listen(SOCKET socket, int32 backlog = SOMAXCONN);

	static bool setLinger(SOCKET socket, uint16 onoff, uint16 linger);
	static bool setReuseAddress(SOCKET socket, bool flag);
	static bool setUpdateAcceptSocket(SOCKET socket, SOCKET listenSocket);
	static bool setTcpNoDelay(SOCKET socket, bool flag);

private:
	static bool bindWindowsFunction(SOCKET socket, GUID guid, LPVOID* fn);
};

template<typename T>
static inline bool SetSockOpt(SOCKET socket, int32 level, int32 optName, T optVal)
{
	return SOCKET_ERROR != ::setsockopt(socket, level, optName, reinterpret_cast<char*>(&optVal), sizeof(T));
}