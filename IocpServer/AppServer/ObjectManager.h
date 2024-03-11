#pragma once

class Player;
class Room;
class PacketSession;

class ObjectManager
{
public:
	static shared_ptr<Player> CreatePlayer(weak_ptr<PacketSession> packetSession);
};

