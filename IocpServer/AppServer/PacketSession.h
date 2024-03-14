#pragma once

#include "Session.h"

class JobQueue;
class Job;

class PacketSession : public Session
{
public:
	PacketSession(uint32 sessionId);
	virtual ~PacketSession();

protected:
	virtual void OnConnect() override;
	virtual void OnDisconnect() override;
	virtual void OnRecv(BYTE* packet) override;
	virtual void OnSend(uint32 numOfBytes) override;
};

