#pragma once

#include "Protocol.pb.h"

class PacketSession;
class SendBuffer;

class ServerPacketHandler
{
public:
	static void init();

	// Packet Handlers
private:
	static void handlePing(shared_ptr<PacketSession> session, BYTE* payload, uint32 payloadSize);
	static void handleC_Enter(shared_ptr<PacketSession> session, BYTE* payload, uint32 payloadSize);
	static void handleC_Exit(shared_ptr<PacketSession> session, BYTE* payload, uint32 payloadSize);
	static void handleC_Spawn(shared_ptr<PacketSession> session, BYTE* payload, uint32 payloadSize);
	static void handleC_Move(shared_ptr<PacketSession> session, BYTE* payload, uint32 payloadSize);

	// Packet Makers
public:
	static shared_ptr<SendBuffer> makePing(Protocol::Ping* payload);
	static shared_ptr<SendBuffer> makeS_Enter(Protocol::S_Enter* payload);
	static shared_ptr<SendBuffer> makeS_Exit(Protocol::S_Exit* payload);
	static shared_ptr<SendBuffer> makeS_Spawn(Protocol::S_Spawn* payload);
	static shared_ptr<SendBuffer> makeS_Despawn(Protocol::S_Despawn* payload);
	static shared_ptr<SendBuffer> makeS_Move(Protocol::S_Move* payload);
};

