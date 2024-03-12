#include "pch.h"
#include "Room.h"
#include "Player.h"
#include "ObjectManager.h"
#include "Utils.h"
#include "PacketSession.h"
#include "ServerPacketHandler.h"
#include "Job.h"

shared_ptr<Room> GRoom = make_shared<Room>();

Room::Room()
{
}

Room::~Room()
{
}

void Room::EnterPlayer(shared_ptr<Player> player)
{
	lock_guard<mutex> lock(_mutex);

	// �ߺ� üũ
	if (_players.find(player->GetPlayerId()) != _players.end())
		return;
	
	// Room�� Player ���� �߰�
	_players[player->GetPlayerId()] = player;
	player->SetRoom(shared_from_this());

	// ĳ���� ����
	SpawnPlayer(player);

	// ���� �Ϸ� �޼��� ����
	if (shared_ptr<PacketSession> packetSession = player->GetPacketSession())
	{
		packetSession->PushSendJob(
			make_shared<Job>([&packetSession, playerId = player->GetPlayerId()]()
				{
					packetSession->Send(ServerPacketHandler::MakeS_Enter(1, playerId));
				}),
			false
		);
	}
}

void Room::ExitPlayer(uint64 playerId)
{
	lock_guard<mutex> lock(_mutex);

	if (_players.find(playerId) == _players.end())
		return;

	// Despawn
	DespawnPlayer(playerId);

	// Exit �Ϸ� �޼��� ����
	if (shared_ptr<PacketSession> packetSession = _players[playerId]->GetPacketSession())
	{
		packetSession->PushSendJob(
			make_shared<Job>([&packetSession]()
				{
					packetSession->Send(ServerPacketHandler::MakeS_Exit(1));
				}),
			false
		);
	}

	// player ����
	_players.erase(playerId);
}

void Room::SpawnPlayer(shared_ptr<Player> player)
{
	// ���� ��ġ ����
	player->SetX(Utils::GetRandom(-1000.f, 1000.f));
	player->SetY(Utils::GetRandom(-1000.f, 1000.f));
	player->SetZ(Utils::GetRandom(50.f, 200.f));
	player->SetYaw(0.f);

	// ���� ������ player�� ������ ������ player��
	vector<shared_ptr<Player>> others;
	vector<uint64> removeIds;

	// �濡 �ִ� ��� player�鿡�� �� player ���� Broadcasting
	for (auto iter = _players.begin(); iter != _players.end(); iter++)
	{
		shared_ptr<Player> other = iter->second;

		if (shared_ptr<PacketSession> packetSession = other->GetPacketSession())
		{
			packetSession->PushSendJob(
				make_shared<Job>([&packetSession, player]()
					{
						packetSession->Send(ServerPacketHandler::MakeS_SpawnPlayer({ player }));
					}),
				false
			);

			if (other->GetPlayerId() != player->GetPlayerId())
			{
				others.push_back(iter->second);
			}
		}
		// PacketSession�� �������� �ʴ� ��� Disconnect�� ���·� �Ǵ��Ͽ� Player ����
		else
		{
			const uint64 removeId = other->GetPlayerId();
			if (_players.find(removeId) != _players.end())
				removeIds.push_back(removeId);
		}
	}
	
	// �� player���� ������ player���� ���� Send
	if (shared_ptr<PacketSession> packetSession = player->GetPacketSession())
	{
		packetSession->PushSendJob(
			make_shared<Job>([&packetSession, others]()
				{
					packetSession->Send(ServerPacketHandler::MakeS_SpawnPlayer(others));
				}), 
			false);
	}

	// Disconnect�� Player ����
	for (uint64 removeId : removeIds)
	{
		_players.erase(removeId);
	}
}

void Room::DespawnPlayer(uint64 playerId)
{
	// �����ִ� player�鿡�� Despawn �޼��� Broadcasting
	for (auto iter = _players.begin(); iter != _players.end(); iter++)
	{
		shared_ptr<Player> other = iter->second;

		if (shared_ptr<PacketSession> packetSession = other->GetPacketSession())
		{
			packetSession->PushSendJob(
				make_shared<Job>([&packetSession, playerId]()
					{
						packetSession->Send(ServerPacketHandler::MakeS_DespawnPlayer({ playerId }));
					}),
				false
			);
		}
	}
}
