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
	static shared_ptr<SendBuffer> MakePing();
	static shared_ptr<SendBuffer> MakeS_Enter(bool result, uint32 enterId, Protocol::PlayerInfo* playerInfo);
	static shared_ptr<SendBuffer> MakeS_Exit(bool result, uint32 enterId);
	static shared_ptr<SendBuffer> MakeS_Spawn(vector<shared_ptr<Player>> players);
	static shared_ptr<SendBuffer> MakeS_Despawn(vector<uint32> playerIds);
	static shared_ptr<SendBuffer> MakeS_Move(Protocol::PlayerInfo* playerInfo);
};

