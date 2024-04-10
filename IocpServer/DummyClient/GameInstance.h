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
	void EnterGameRoom(shared_ptr<Session> session);
	void HandleEnterGameRoom(Protocol::S_Enter payload);

	void ExitGameRoom(shared_ptr<Session> session);
	void HandleExitGameRoom(Protocol::S_Exit payload);

	void SpawnMyPlayer(shared_ptr<Session> session);
	void HandleSpawnPlayer(Protocol::S_Spawn payload);

	void HandleDespawnPlayer(Protocol::S_Despawn payload);

	void MoveMyPlayersToOther(shared_ptr<Session> session);
	void HandleMovePlayer(Protocol::S_Move payload);

private:
	shared_ptr<Player> SpawnPlayer(const Protocol::ObjectInfo& playerInfo);
	void DespawnPlayer(uint32 playerId);

private:
	map<uint32, shared_ptr<Player>> _myPlayers;
	map<uint32, shared_ptr<Player>> _otherPlayers;
};

extern shared_ptr<GameInstance> gGameInstance;
