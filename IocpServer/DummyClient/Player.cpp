#include "pch.h"
#include "Player.h"

Player::Player(uint32 objectId) :
	Object(objectId, Protocol::OBJECT_TYPE_PLAYER)
{}

Player::~Player()
{}