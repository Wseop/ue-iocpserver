#pragma once

class Player;

class Room : public enable_shared_from_this<Room>
{
public:
	Room();
	virtual ~Room();

public:
	void EnterPlayer(shared_ptr<Player> player);
	void SpawnPlayer(shared_ptr<Player> player);

private:
	mutex _mutex;
	map<uint64, shared_ptr<Player>> _players;
};

extern shared_ptr<Room> GRoom;