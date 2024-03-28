#pragma once

#include "PacketHandler.h"
#include "Protocol.pb.h"

class ClientPacketHandler : public PacketHandler
{
public:
	static void Init();

private:
	// PacketHandlers
	static void HandlePing(shared_ptr<Session> session, BYTE* payload, uint32 payloadSize);
	static void HandleS_Enter(shared_ptr<Session> session, BYTE* payload, uint32 payloadSize);
	static void HandleS_Exit(shared_ptr<Session> session, BYTE* payload, uint32 payloadSize);
	static void HandleS_Spawn(shared_ptr<Session> session, BYTE* payload, uint32 payloadSize);
	static void HandleS_Despawn(shared_ptr<Session> session, BYTE* payload, uint32 payloadSize);
	static void HandleS_Move(shared_ptr<Session> session, BYTE* payload, uint32 payloadSize);

public:
	// Packet Makers
	static inline shared_ptr<SendBuffer> MakePing(Protocol::Ping* payload) { return MakeSendBuffer(PacketType::Ping, payload); }
	static inline shared_ptr<SendBuffer> MakeC_Enter(Protocol::C_Enter* payload) { return MakeSendBuffer(PacketType::C_Enter, payload); }
	static inline shared_ptr<SendBuffer> MakeC_Exit(Protocol::C_Exit* payload) { return MakeSendBuffer(PacketType::C_Exit, payload); }
	static inline shared_ptr<SendBuffer> MakeC_Move(Protocol::C_Move* payload) { return MakeSendBuffer(PacketType::C_Move, payload); }
};

