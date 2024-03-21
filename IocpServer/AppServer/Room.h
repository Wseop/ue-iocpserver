#pragma once

#include "Protocol.pb.h"

class Player;
class Session;
class JobQueue;
class SendBuffer;

class Room : public enable_shared_from_this<Room>
{
public:
	Room();
	virtual ~Room();

public:
	void Enter(shared_ptr<Session> session);
	bool Exit(uint32 sessionId);

	void MovePlayer(shared_ptr<Session> playerOwner, Protocol::PlayerInfo& playerInfo);

private:
	shared_ptr<Player> SpawnPlayer(weak_ptr<Session> session);
	void DespawnPlayer(uint32 playerId);

	void Broadcast(shared_ptr<SendBuffer> sendBuffer);

private:
	mutex _mutex;
	unordered_map<uint32, weak_ptr<Session>> _sessions;
	unordered_map<uint32, shared_ptr<Player>> _players;

	shared_ptr<JobQueue> _jobQueue = nullptr;
};

extern shared_ptr<Room> gRoom;