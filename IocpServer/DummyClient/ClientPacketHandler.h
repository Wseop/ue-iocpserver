#pragma once

#include "Protocol.pb.h"

class PacketSession;
class SendBuffer;

class ClientPacketHandler
{
public:
	static void init();

private:
	// PacketHandlers
	static void handlePing(shared_ptr<PacketSession> session, BYTE* payload, uint32 payloadSize);
	static void handleS_Enter(shared_ptr<PacketSession> session, BYTE* payload, uint32 payloadSize);
	static void handleS_Exit(shared_ptr<PacketSession> session, BYTE* payload, uint32 payloadSize);
	static void handleS_Spawn(shared_ptr<PacketSession> session, BYTE* payload, uint32 payloadSize);
	static void handleS_Despawn(shared_ptr<PacketSession> session, BYTE* payload, uint32 payloadSize);
	static void handleS_Move(shared_ptr<PacketSession> session, BYTE* payload, uint32 payloadSize);

public:
	// Packet Makers
	static shared_ptr<SendBuffer> makePing(Protocol::Ping* payload);
	static shared_ptr<SendBuffer> makeC_Enter(Protocol::C_Enter* payload);
	static shared_ptr<SendBuffer> makeC_Exit(Protocol::C_Exit* payload);
	static shared_ptr<SendBuffer> makeC_Spawn(Protocol::C_Spawn* payload);
	static shared_ptr<SendBuffer> makeC_Move(Protocol::C_Move* payload);
};

