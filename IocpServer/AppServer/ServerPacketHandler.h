#pragma once

#include "PacketHandler.h"

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

	// Packet Makers
public:
	static shared_ptr<SendBuffer> MakePing();
	static shared_ptr<SendBuffer> MakeS_Enter(int32 result, uint64 playerId);
	static shared_ptr<SendBuffer> MakeS_Exit(int32 result);
	static shared_ptr<SendBuffer> MakeS_SpawnPlayer(vector<shared_ptr<Player>> players);
	static shared_ptr<SendBuffer> MakeS_DespawnPlayer(vector<uint64> playerIds);
};

