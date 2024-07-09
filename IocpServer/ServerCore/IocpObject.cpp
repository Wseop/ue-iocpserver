#include "pch.h"
#include "IocpObject.h"

IocpObject::IocpObject()
{
	_socket = SocketUtils::createSocket();
}

IocpObject::~IocpObject()
{
	SocketUtils::closeSocket(_socket);
}
