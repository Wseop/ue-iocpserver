#include "pch.h"
#include "IocpEvent.h"

IocpEvent::IocpEvent(EventType eventType) :
	_eventType(eventType)
{
	init();
}

IocpEvent::~IocpEvent()
{
}

void IocpEvent::init()
{
	hEvent = 0;
	Offset = 0;
	OffsetHigh = 0;
	Internal = 0;
	InternalHigh = 0;
}

void IocpEvent::addSendBuffer(shared_ptr<SendBuffer> sendBuffer)
{
	_sendBuffers.push_back(sendBuffer);
}

void IocpEvent::clearSendBuffers()
{
	_sendBuffers.clear();
}
