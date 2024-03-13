#include "pch.h"
#include "Room.h"
#include "Player.h"
#include "ObjectManager.h"
#include "Utils.h"
#include "PacketSession.h"
#include "ServerPacketHandler.h"
#include "Job.h"

shared_ptr<Room> gRoom = make_shared<Room>();

Room::Room()
{
}

Room::~Room()
{
}

bool Room::EnterPlayer(shared_ptr<Player> player)
{
	lock_guard<mutex> lock(_mutex);

	// 중복 체크
	if (_players.find(player->GetPlayerId()) != _players.end())
		return false;
	
	// 플레이어 스폰
	if (SpawnPlayer(player) == false)
		return false;

	// 방에 플레이어 정보 추가
	_players[player->GetPlayerId()] = player;
	player->SetRoom(shared_from_this());

	return true;
}

bool Room::ExitPlayer(uint64 playerId)
{
	lock_guard<mutex> lock(_mutex);

	// 존재하는 플레이어가 아니면 종료
	if (_players.find(playerId) == _players.end())
		return false;

	// 방에서 플레이어 정보 제거
	_players.erase(playerId);

	// 플레이어 디스폰
	if (DespawnPlayer(playerId) == false)
		return false;

	return true;
}

bool Room::SpawnPlayer(shared_ptr<Player> player)
{
	// 랜덤 위치 지정
	player->SetX(Utils::GetRandom(-1000.f, 1000.f));
	player->SetY(Utils::GetRandom(-1000.f, 1000.f));
	player->SetZ(Utils::GetRandom(50.f, 200.f));
	player->SetYaw(0.f);

	vector<shared_ptr<Player>> players;
	vector<uint64> removeIds;

	// 방에 있는 다른 플레이어들에게 새로 입장한 플레이어 정보 전송 (Broadcasting)
	for (auto iter = _players.begin(); iter != _players.end(); iter++)
	{
		shared_ptr<Player> other = iter->second;

		// 예외처리) 제거된 플레이어인 경우
		if (other == nullptr)
		{
			removeIds.push_back(iter->first);
			continue;
		}

		shared_ptr<PacketSession> otherSession = other->GetPacketSession();
		
		// 예외처리) 연결이 끊긴 플레이어인 경우
		if (otherSession == nullptr)
		{
			removeIds.push_back(iter->first);
			continue;
		}

		otherSession->Send(ServerPacketHandler::MakeS_SpawnPlayer({ player }));
		players.push_back(other);
	}

	players.push_back(player);
	
	// 새로 입장한 플레이어에게 방에 있는 모든 플레이어들의 정보 전송
	shared_ptr<PacketSession> playerSession = player->GetPacketSession();

	if (playerSession == nullptr)
		return false;
	
	playerSession->Send(ServerPacketHandler::MakeS_SpawnPlayer(players));

	RemovePlayer(removeIds);

	return true;
}

bool Room::DespawnPlayer(uint64 playerId)
{
	vector<uint64> removeIds;

	// 방에 남은 플레이어들에게 나간 플레이어의 정보 전송
	for (auto iter = _players.begin(); iter != _players.end(); iter++)
	{
		// 예외처리) 제거된 플레이어인 경우
		if (iter->second == nullptr)
		{
			removeIds.push_back(iter->first);
			continue;
		}

		shared_ptr<PacketSession> playerSession = iter->second->GetPacketSession();

		// 예외처리) 연결이 끊긴 플레이어인 경우
		if (playerSession == nullptr)
		{
			removeIds.push_back(iter->first);
			continue;
		}

		playerSession->Send(ServerPacketHandler::MakeS_DespawnPlayer({ playerId }));
	}

	RemovePlayer(removeIds);

	return true;
}

void Room::RemovePlayer(vector<uint64> removeIds)
{
	for (uint64 removeId : removeIds)
	{
		_players.erase(removeId);
	}
}
