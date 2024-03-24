#pragma once

class Player;
class Room;
class Session;

class ObjectManager
{
public:
	static shared_ptr<Player> CreatePlayer(weak_ptr<Session> session);

private:
	static atomic<uint32> _sObjectId;
};

