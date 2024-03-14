#include "pch.h"
#include "Player.h"
#include "Room.h"
#include "PacketSession.h"

Player::Player(uint32 playerId, weak_ptr<Session> session) :
	_playerId(playerId),
	_session(session)
{
}

Player::~Player()
{
}
