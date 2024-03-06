#include "pch.h"
#include "IocpObject.h"

IocpObject::IocpObject()
{
	_socket = SocketUtils::CreateSocket();
}

IocpObject::~IocpObject()
{
	SocketUtils::Close(_socket);
}
