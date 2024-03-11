#include "pch.h"
#include "Player.h"
#include "Room.h"
#include "PacketSession.h"

Player::Player(uint64 playerId, weak_ptr<PacketSession> packetSession) :
	_playerId(playerId),
	_packetSession(packetSession)
{
}

Player::~Player()
{
}
