#pragma once

#include "Object.h"

class Room;
class Session;

class Player : public Object
{
public:
	Player(uint32 objectId, weak_ptr<Session> session);
	virtual ~Player();

public:
	shared_ptr<Room> GetRoom() const { return _room.lock(); }
	void SetRoom(weak_ptr<Room> room) { _room = room; }

	shared_ptr<Session> GetSession() const { return _session.lock(); }

private:
	weak_ptr<Room> _room;
	weak_ptr<Session> _session;
};

