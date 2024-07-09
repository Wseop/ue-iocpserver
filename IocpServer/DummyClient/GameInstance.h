#pragma once

#include "JobQueue.h"
#include "Protocol.pb.h"

class Player;
class PacketSession;

class GameInstance : public JobQueue
{
public:
	GameInstance();
	~GameInstance();

public:
	void enterGameRoom(shared_ptr<PacketSession> session);
	void handleEnterGameRoom(Protocol::S_Enter payload);

	void exitGameRoom(shared_ptr<PacketSession> session);
	void handleExitGameRoom(Protocol::S_Exit payload);

	void spawnMyPlayer(shared_ptr<PacketSession> session);
	void handleSpawnPlayer(Protocol::S_Spawn payload);

	void handleDespawnPlayer(Protocol::S_Despawn payload);

	void moveMyPlayersToOther(shared_ptr<PacketSession> session);
	void handleMovePlayer(Protocol::S_Move payload);

private:
	shared_ptr<Player> spawnPlayer(const Protocol::ObjectInfo& playerInfo);
	void despawnPlayer(uint32 playerId);

private:
	map<uint32, shared_ptr<Player>> _myPlayers;
	map<uint32, shared_ptr<Player>> _otherPlayers;
};

extern shared_ptr<GameInstance> gGameInstance;
