#include "pch.h"
#include "Player.h"

Player::Player(uint32 objectId, weak_ptr<Session> owner) :
	Object(objectId, Protocol::OBJECT_TYPE_PLAYER),
	_owner(owner)
{}

Player::~Player()
{}