#pragma once

#include "Object.h"

class Session;

class Player : public Object
{
public:
	Player(uint32 objectId, weak_ptr<Session> owner);
	virtual ~Player();

public:
	shared_ptr<Session> GetSession() const { return _owner.lock(); }

private:
	weak_ptr<Session> _owner;
};

