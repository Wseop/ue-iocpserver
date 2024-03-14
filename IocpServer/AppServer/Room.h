#pragma once

class Player;
class Session;

class Room : public enable_shared_from_this<Room>
{
public:
	Room();
	virtual ~Room();

public:
	bool Enter(shared_ptr<Session> session);

private:
	mutex _mutex;
	unordered_map<uint32, weak_ptr<Session>> _sessions;
};

extern shared_ptr<Room> gRoom;