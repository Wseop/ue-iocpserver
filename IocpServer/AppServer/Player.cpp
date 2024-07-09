#include "pch.h"
#include "Player.h"
#include "Room.h"
#include "Session.h"

Player::Player(uint32 objectId, weak_ptr<PacketSession> session) :
	Object(objectId, Protocol::OBJECT_TYPE_PLAYER),
	_session(session)
{
}

Player::~Player()
{
}