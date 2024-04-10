#pragma once

#include "Object.h"

class Session;

class Player : public Object
{
public:
	Player(uint32 objectId);
	virtual ~Player();
};

