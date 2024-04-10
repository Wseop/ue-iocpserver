#pragma once

#include "Protocol.pb.h"
#include "JobQueue.h"

class Player;
class Session;
class SendBuffer;

class Room : public JobQueue
{
public:
	Room();
	virtual ~Room();

public:
	uint64 GetCleanupTick() const { return CLEANUP_TICK; }

public:
	void Enter(shared_ptr<Session> session, Protocol::C_Enter payload);
	void Exit(shared_ptr<Session> session, Protocol::C_Exit payload);
	
	void SpawnPlayer(shared_ptr<Session> session);
	void DespawnPlayer(uint32 playerId);
	
	void MovePlayer(shared_ptr<Session> session, Protocol::C_Move payload);

private:
	void Broadcast(shared_ptr<SendBuffer> sendBuffer);
	void Broadcast(shared_ptr<SendBuffer> sendBuffer, uint32 exceptId);

public:
	void Cleanup();

private:
	const uint64 CLEANUP_TICK = 1000 * 60 * 10;

	map<uint32, weak_ptr<Session>> _sessions;
	map<uint32, shared_ptr<Player>> _players;
};

extern shared_ptr<Room> gRoom;