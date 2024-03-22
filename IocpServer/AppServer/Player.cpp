#include "pch.h"
#include "Player.h"
#include "Room.h"
#include "PacketSession.h"

Player::Player(uint32 playerId, weak_ptr<Session> session) :
	_playerInfo(new Protocol::PlayerInfo()),
	_session(session)
{
	_playerInfo->set_player_id(playerId);
	_playerInfo->set_x(0.f);
	_playerInfo->set_y(0.f);
	_playerInfo->set_z(0.f);
	_playerInfo->set_yaw(0.f);
	_playerInfo->set_move_state(Protocol::MOVE_STATE_IDLE);
}

Player::~Player()
{
	if (_playerInfo != nullptr)
	{
		delete _playerInfo;
		_playerInfo = nullptr;
	}
}

bool Player::Move(Protocol::PlayerInfo& info)
{
	if (GetPlayerId() != info.player_id())
		return false;

	_playerInfo->CopyFrom(info);
	return true;
}
