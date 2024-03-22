#pragma once

#include "Protocol.pb.h"

class Room;
class Session;

class Player
{
public:
	Player(uint32 playerId, weak_ptr<Session> session);
	~Player();

public:
	Protocol::PlayerInfo& GetPlayerInfo() { return *_playerInfo; }

	uint32 GetPlayerId() { return _playerInfo->player_id(); }

	float GetX() { return _playerInfo->x(); }
	void SetX(float x) { _playerInfo->set_x(x); }

	float GetY() { return _playerInfo->y(); }
	void SetY(float y) { _playerInfo->set_y(y); }

	float GetZ() { return _playerInfo->z(); }
	void SetZ(float z) { _playerInfo->set_z(z); }

	float GetYaw() { return _playerInfo->yaw(); }
	void SetYaw(float yaw) { _playerInfo->set_yaw(yaw); }

	shared_ptr<Room> GetRoom() { return _room.lock(); }
	void SetRoom(weak_ptr<Room> room) { _room = room; }

	shared_ptr<Session> GetSession() { return _session.lock(); }

public:
	bool Move(Protocol::PlayerInfo& info);

private:
	Protocol::PlayerInfo* _playerInfo = nullptr;

	weak_ptr<Room> _room;
	weak_ptr<Session> _session;
};

