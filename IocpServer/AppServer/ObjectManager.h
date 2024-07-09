#pragma once

class Player;
class Room;
class PacketSession;

class ObjectManager
{
public:
	static shared_ptr<Player> CreatePlayer(shared_ptr<PacketSession> session);

private:
	static atomic<uint32> _sObjectId;
};

