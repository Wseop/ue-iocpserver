#pragma once

class Room;
class Session;

class Player
{
public:
	Player(uint32 playerId, weak_ptr<Session> session);
	~Player();

public:
	uint32 GetPlayerId() { return _playerId; }

	float GetX() { return _x; }
	void SetX(float x) { _x = x; }

	float GetY() { return _y; }
	void SetY(float y) { _y = y; }

	float GetZ() { return _z; }
	void SetZ(float z) { _z = z; }

	float GetYaw() { return _yaw; }
	void SetYaw(float yaw) { _yaw = yaw; }

	shared_ptr<Room> GetRoom() { return _room.lock(); }
	void SetRoom(weak_ptr<Room> room) { _room = room; }

	shared_ptr<Session> GetSession() { return _session.lock(); }

private:
	uint32 _playerId = 0;
	
	float _x = 0.f;
	float _y = 0.f;
	float _z = 0.f;
	float _yaw = 0.f;

	weak_ptr<Room> _room;
	weak_ptr<Session> _session;
};

