#pragma once

#include "JobQueue.h"
#include "Protocol.pb.h"

class Player;
class Session;

class GameInstance : public JobQueue
{
public:
	GameInstance();
	~GameInstance();

public:
	void Enter(shared_ptr<Session> session);
	void HandleEnter(shared_ptr<Session> session, const Protocol::S_Enter enterPacket);

	void Exit(shared_ptr<Session> session);
	void HandleExit(shared_ptr<Session> session, const Protocol::S_Exit exitPacket);

	void Move(shared_ptr<Session> session);

private:
	set<uint32> _sessionIds;
	unordered_map<uint32, shared_ptr<Player>> _players;
};

extern shared_ptr<GameInstance> gGameInstance;
