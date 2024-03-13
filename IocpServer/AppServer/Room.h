#pragma once

class Player;

class Room : public enable_shared_from_this<Room>
{
public:
	Room();
	virtual ~Room();

public:
	bool EnterPlayer(shared_ptr<Player> player);
	bool ExitPlayer(uint64 playerId);

private:
	bool SpawnPlayer(shared_ptr<Player> player);
	bool DespawnPlayer(uint64 playerId);
	void RemovePlayer(vector<uint64> removeIds);

private:
	mutex _mutex;
	map<uint64, shared_ptr<Player>> _players;
};

extern shared_ptr<Room> gRoom;