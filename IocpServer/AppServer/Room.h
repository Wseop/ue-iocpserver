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
	void Enter(shared_ptr<Session> session, const string key);
	void Exit(shared_ptr<Session> session, const uint32 enterId);

	void MovePlayer(shared_ptr<Session> playerOwner, Protocol::PlayerInfo playerInfo);

private:
	shared_ptr<Player> SpawnPlayer(weak_ptr<Session> session);
	void DespawnPlayer(uint32 playerId);

	void Broadcast(shared_ptr<SendBuffer> sendBuffer);

private:
	unordered_map<uint32, weak_ptr<Session>> _sessions;
	unordered_map<uint32, shared_ptr<Player>> _players;
};

extern shared_ptr<Room> gRoom;