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

	// 중복 체크
	if (_players.find(player->GetPlayerId()) != _players.end())
		return;
	
	// Room에 Player 정보 추가
	_players[player->GetPlayerId()] = player;
	player->SetRoom(shared_from_this());

	// 캐릭터 스폰
	SpawnPlayer(player);

	// 입장 완료 메세지 전송
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

void Room::SpawnPlayer(shared_ptr<Player> player)
{
	// 랜덤 위치 지정
	player->SetX(Utils::GetRandom(-1000.f, 1000.f));
	player->SetY(Utils::GetRandom(-1000.f, 1000.f));
	player->SetZ(Utils::GetRandom(50.f, 200.f));
	player->SetYaw(0.f);

	// 새로 입장한 player를 제외한 나머지 player들
	vector<shared_ptr<Player>> others;

	// 방에 있는 모든 player들에게 새 player 정보 Broadcasting
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
	
	// 새 player에게 나머지 player들의 정보 Send
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

void Room::ExitPlayer(uint64 playerId)
{
	lock_guard<mutex> lock(_mutex);

	if (_players.find(playerId) == _players.end())
		return;

	// Despawn
	DespawnPlayer(playerId);

	// Exit 완료 메세지 전송
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

	// player 제거
	_players.erase(playerId);
}

void Room::DespawnPlayer(uint64 playerId)
{
	// 남아있는 player들에게 Despawn 메세지 Broadcasting
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
