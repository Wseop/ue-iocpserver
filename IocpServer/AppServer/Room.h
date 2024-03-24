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
	void Enter(shared_ptr<Session> session, Protocol::C_Enter payload);
	void Exit(shared_ptr<Session> session, Protocol::C_Exit payload);
	void MovePlayer(shared_ptr<Session> session, Protocol::C_Move payload);

private:
	shared_ptr<Player> SpawnPlayer(weak_ptr<Session> session);
	void DespawnPlayer(uint32 playerId);
	
	void Broadcast(shared_ptr<SendBuffer> sendBuffer);

private:
	unordered_map<uint32, weak_ptr<Session>> _sessions;
	unordered_map<uint32, shared_ptr<Player>> _players;
};

extern shared_ptr<Room> gRoom;