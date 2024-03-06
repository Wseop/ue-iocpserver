#include "pch.h"
#include "IocpEvent.h"

IocpEvent::IocpEvent(EventType eventType) :
	_eventType(eventType)
{
}

IocpEvent::~IocpEvent()
{
}

void IocpEvent::Init()
{
	hEvent = 0;
	Offset = 0;
	OffsetHigh = 0;
	Internal = 0;
	InternalHigh = 0;
}

void IocpEvent::PushSendBuffer(shared_ptr<SendBuffer> sendBuffer)
{
	if (sendBuffer)
		_sendBuffers.push_back(sendBuffer);
}

void IocpEvent::ClearSendBuffers()
{
	_sendBuffers.clear();
}
