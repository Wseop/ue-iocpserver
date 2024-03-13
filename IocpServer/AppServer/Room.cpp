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

	// �ߺ� üũ
	if (_players.find(player->GetPlayerId()) != _players.end())
		return false;
	
	// �÷��̾� ����
	if (SpawnPlayer(player) == false)
		return false;

	// �濡 �÷��̾� ���� �߰�
	_players[player->GetPlayerId()] = player;
	player->SetRoom(shared_from_this());

	return true;
}

bool Room::ExitPlayer(uint64 playerId)
{
	lock_guard<mutex> lock(_mutex);

	// �����ϴ� �÷��̾ �ƴϸ� ����
	if (_players.find(playerId) == _players.end())
		return false;

	// �濡�� �÷��̾� ���� ����
	_players.erase(playerId);

	// �÷��̾� ����
	if (DespawnPlayer(playerId) == false)
		return false;

	return true;
}

bool Room::SpawnPlayer(shared_ptr<Player> player)
{
	// ���� ��ġ ����
	player->SetX(Utils::GetRandom(-1000.f, 1000.f));
	player->SetY(Utils::GetRandom(-1000.f, 1000.f));
	player->SetZ(Utils::GetRandom(50.f, 200.f));
	player->SetYaw(0.f);

	vector<shared_ptr<Player>> players;
	vector<uint64> removeIds;

	// �濡 �ִ� �ٸ� �÷��̾�鿡�� ���� ������ �÷��̾� ���� ���� (Broadcasting)
	for (auto iter = _players.begin(); iter != _players.end(); iter++)
	{
		shared_ptr<Player> other = iter->second;

		// ����ó��) ���ŵ� �÷��̾��� ���
		if (other == nullptr)
		{
			removeIds.push_back(iter->first);
			continue;
		}

		shared_ptr<PacketSession> otherSession = other->GetPacketSession();
		
		// ����ó��) ������ ���� �÷��̾��� ���
		if (otherSession == nullptr)
		{
			removeIds.push_back(iter->first);
			continue;
		}

		otherSession->Send(ServerPacketHandler::MakeS_SpawnPlayer({ player }));
		players.push_back(other);
	}

	players.push_back(player);
	
	// ���� ������ �÷��̾�� �濡 �ִ� ��� �÷��̾���� ���� ����
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

	// �濡 ���� �÷��̾�鿡�� ���� �÷��̾��� ���� ����
	for (auto iter = _players.begin(); iter != _players.end(); iter++)
	{
		// ����ó��) ���ŵ� �÷��̾��� ���
		if (iter->second == nullptr)
		{
			removeIds.push_back(iter->first);
			continue;
		}

		shared_ptr<PacketSession> playerSession = iter->second->GetPacketSession();

		// ����ó��) ������ ���� �÷��̾��� ���
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
