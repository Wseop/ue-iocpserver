#pragma once

#include "Protocol.pb.h"
#include "JobQueue.h"

class Player;
class PacketSession;
class SendBuffer;

class Room : public JobQueue
{
public:
	Room();
	virtual ~Room();

public:
	uint64 getCleanupTick() const { return CLEANUP_TICK; }

public:
	void enter(shared_ptr<PacketSession> session, Protocol::C_Enter payload);
	void exit(shared_ptr<PacketSession> session, Protocol::C_Exit payload);
	
	void spawnPlayer(shared_ptr<PacketSession> session);
	void despawnPlayer(uint32 playerId);
	
	void movePlayer(shared_ptr<PacketSession> session, Protocol::C_Move payload);

private:
	void broadcast(shared_ptr<SendBuffer> sendBuffer);
	void broadcast(shared_ptr<SendBuffer> sendBuffer, uint32 exceptId);

public:
	void cleanup();

private:
	const uint64 CLEANUP_TICK = 1000 * 60 * 10;

	map<uint32, weak_ptr<PacketSession>> _sessions;
	map<uint32, shared_ptr<Player>> _players;
};

extern shared_ptr<Room> gRoom;