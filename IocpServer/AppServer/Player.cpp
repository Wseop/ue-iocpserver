#include "pch.h"
#include "Player.h"
#include "Room.h"
#include "PacketSession.h"

Player::Player(uint32 objectId, weak_ptr<Session> session) :
	Object(objectId, Protocol::OBJECT_TYPE_PLAYER),
	_session(session)
{
}

Player::~Player()
{
}