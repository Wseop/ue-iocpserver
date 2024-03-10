#pragma once

#include "PacketHandler.h"

class ServerPacketHandler : public PacketHandler
{
public:
	static void Init();

	// Packet Handlers
private:
	static void HandlePing(shared_ptr<Session> session, BYTE* payload, uint32 payloadSize);
	static void HandleC_Enter(shared_ptr<Session> session, BYTE* payload, uint32 payloadSize);

	// Packet Makers
public:
	static shared_ptr<SendBuffer> MakePing();
	static shared_ptr<SendBuffer> MakeS_Enter(bool bResult);
};

