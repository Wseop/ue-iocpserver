#pragma once

#include "PacketHandler.h"
#include "Protocol.pb.h"

class Player;

class ServerPacketHandler : public PacketHandler
{
public:
	static void Init();

	// Packet Handlers
private:
	static void HandlePing(shared_ptr<Session> session, BYTE* payload, uint32 payloadSize);
	static void HandleC_Enter(shared_ptr<Session> session, BYTE* payload, uint32 payloadSize);
	static void HandleC_Exit(shared_ptr<Session> session, BYTE* payload, uint32 payloadSize);
	static void HandleC_Move(shared_ptr<Session> session, BYTE* payload, uint32 payloadSize);

	// Packet Makers
public:
	static inline shared_ptr<SendBuffer> MakePing(Protocol::Ping* payload) { return MakeSendBuffer(PacketType::Ping, payload); }
	static inline shared_ptr<SendBuffer> MakeS_Enter(Protocol::S_Enter* payload) { return MakeSendBuffer(PacketType::S_Enter, payload); }
	static inline shared_ptr<SendBuffer> MakeS_Exit(Protocol::S_Exit* payload) { return MakeSendBuffer(PacketType::S_Exit, payload); }
	static inline shared_ptr<SendBuffer> MakeS_Spawn(Protocol::S_Spawn* payload) { return MakeSendBuffer(PacketType::S_Spawn, payload); }
	static inline shared_ptr<SendBuffer> MakeS_Despawn(Protocol::S_Despawn* payload) { return MakeSendBuffer(PacketType::S_Despawn, payload); }
	static inline shared_ptr<SendBuffer> MakeS_Move(Protocol::S_Move* payload) { return MakeSendBuffer(PacketType::S_Move, payload); }
};

