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
					packetSession->Send(ServerPacketHandler::MakeS_Enter(true, playerId));
				}),
			false
		);
	}
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

	// �濡 �ִ� ��� player�鿡�� �� player ���� Broadcasting
	for (auto iter = _players.begin(); iter != _players.end(); iter++)
	{
		shared_ptr<Player> other = iter->second;

		if (other->GetPlayerId() != player->GetPlayerId())
		{
			others.push_back(iter->second);
		}

		if (shared_ptr<PacketSession> packetSession = other->GetPacketSession())
		{
			packetSession->PushSendJob(
				make_shared<Job>([&packetSession, player]()
					{
						packetSession->Send(ServerPacketHandler::MakeS_SpawnPlayer({ player }));
					}),
				false
			);
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
}
