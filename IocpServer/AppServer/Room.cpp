#include "pch.h"
#include "Room.h"
#include "Player.h"
#include "ObjectManager.h"
#include "Utils.h"
#include "PacketSession.h"
#include "ServerPacketHandler.h"
#include "JobQueue.h"
#include "Job.h"
#include "JobTimer.h"

shared_ptr<Room> gRoom = make_shared<Room>();

Room::Room()
{
}

Room::~Room()
{
}

void Room::Enter(shared_ptr<Session> session, Protocol::C_Enter payload)
{
	const uint32 sessionId = session->GetSessionId();
	Protocol::S_Enter enterPacket;

	// 세션 중복 체크
	if (_sessions.find(sessionId) != _sessions.end())
	{
		enterPacket.set_result(false);
		enterPacket.set_session_id(0);
		session->Send(ServerPacketHandler::MakeS_Enter(&enterPacket));
		return;
	}

	// 플레이어 스폰
	shared_ptr<Player> newPlayer = SpawnPlayer(session);
	if (newPlayer == nullptr)
	{
		enterPacket.set_result(false);
		enterPacket.set_session_id(0);
		session->Send(ServerPacketHandler::MakeS_Enter(&enterPacket));
		return;
	}

	// S_Enter 패킷 전송
	enterPacket.set_result(true);
	enterPacket.set_session_id(sessionId);
	enterPacket.mutable_my_object_info()->CopyFrom(*newPlayer->GetObjectInfo());
	for (auto iter = _players.begin(); iter != _players.end(); iter++)
	{
		shared_ptr<Player> other = iter->second;
		if (other == nullptr || other->GetObjectId() == newPlayer->GetObjectId())
			continue;
		enterPacket.add_other_object_infos()->CopyFrom(*other->GetObjectInfo());
	}
	session->Send(ServerPacketHandler::MakeS_Enter(&enterPacket));

	// 다른 세션들에게 새 플레이어 정보를 알림
	Protocol::S_Spawn spawnPacket;
	spawnPacket.add_object_infos()->CopyFrom(*newPlayer->GetObjectInfo());
	Broadcast(ServerPacketHandler::MakeS_Spawn(&spawnPacket));

	// 세션 목록에 추가
	_sessions[sessionId] = session;

	spdlog::info("Enter : SessionId : {}", sessionId);
}

void Room::Exit(shared_ptr<Session> session, Protocol::C_Exit payload)
{
	const uint32 sessionId = session->GetSessionId();
	Protocol::S_Exit exitPacket;
	exitPacket.set_session_id(sessionId);

	// 세션 유무 확인
	if (_sessions.find(sessionId) == _sessions.end())
	{
		exitPacket.set_result(false);
		session->Send(ServerPacketHandler::MakeS_Exit(&exitPacket));
		return;
	}
	
	// 세션 목록에서 제거
	_sessions.erase(sessionId);
	// Exit 패킷 전송
	exitPacket.set_result(true);
	session->Send(ServerPacketHandler::MakeS_Exit(&exitPacket));

	spdlog::info("Exit : SessionId : {}", sessionId);

	// 제거할 플레이어 검색
	Protocol::S_Despawn despawnPacket;
	for (auto iter = _players.begin(); iter != _players.end(); iter++)
	{
		shared_ptr<Player> player = iter->second;
		if (player == nullptr)
			continue;
		
		shared_ptr<Session> playerSession = player->GetSession();
		if (playerSession == nullptr || playerSession != session)
			continue;

		despawnPacket.add_object_ids(player->GetObjectId());
	}
	// 다른 세션들에게 Despawn 패킷 전송
	Broadcast(ServerPacketHandler::MakeS_Despawn(&despawnPacket));

	// 플레이어 목록에서 제거
	for (uint32 playerId : despawnPacket.object_ids())
		DespawnPlayer(playerId);
}

void Room::MovePlayer(shared_ptr<Session> session, Protocol::C_Move payload)
{
	// 세션 입장 여부 확인
	if (_sessions.find(session->GetSessionId()) == _sessions.end())
		return;

	// 이동 시킬 플레이어 검색
	const Protocol::PosInfo& posInfo = payload.pos_info();
	auto playerIter = _players.find(posInfo.object_id());
	if (playerIter == _players.end())
		return;

	shared_ptr<Player> player = playerIter->second;
	if (player == nullptr || player->GetSession() != session)
		return;

	// 플레이어 이동
	player->SetPosInfo(posInfo);

	spdlog::info("MovePlayer : {} : [{}, {}, {}, {}]", player->GetObjectId(), static_cast<int32>(posInfo.move_state()), posInfo.x(), posInfo.y(), posInfo.z());

	// Move 패킷 Broadcast
	Protocol::S_Move movePacket;
	movePacket.mutable_pos_info()->CopyFrom(posInfo);
	Broadcast(ServerPacketHandler::MakeS_Move(&movePacket));
}

shared_ptr<Player> Room::SpawnPlayer(weak_ptr<Session> session)
{
	shared_ptr<Player> player = ObjectManager::CreatePlayer(session);
	if (player == nullptr)
		return nullptr;

	const uint32 playerId = player->GetObjectId();
	// 중복 체크
	if (_players.find(playerId) != _players.end())
		return nullptr;

	// 스폰 위치 랜덤 지정
	player->SetX(Utils::GetRandom(-1000.f, 1000.f));
	player->SetY(Utils::GetRandom(-1000.f, 1000.f));
	player->SetZ(100.f);

	_players[playerId] = player;

	spdlog::info("SpawnPlayer : {}", playerId);

	return player;
}

void Room::DespawnPlayer(uint32 playerId)
{
	if (_players.find(playerId) == _players.end())
		return;

	_players.erase(playerId);

	spdlog::info("DespawnPlayer : {}", playerId);
}

void Room::Broadcast(shared_ptr<SendBuffer> sendBuffer)
{
	for (auto iter = _sessions.begin(); iter != _sessions.end(); iter++)
	{
		shared_ptr<Session> session = iter->second.lock();
		if (session == nullptr)
			continue;
		session->Send(sendBuffer);
	}
}

void Room::Cleanup()
{
	spdlog::debug("Room : Cleanup");

	{
		auto iter = _sessions.begin();
		while (iter != _sessions.end())
		{
			if (iter->second.lock() == nullptr)
			{
				iter = _sessions.erase(iter);
				spdlog::debug("Room : Null Session Removed");
			}
			else
				iter++;
		}
	}

	{
		auto iter = _players.begin();
		while (iter != _players.end())
		{
			if (iter->second == nullptr || iter->second->GetSession() == nullptr)
			{
				iter = _players.erase(iter);
				spdlog::debug("Room : Null Player Removed");
			}
			else
				iter++;
		}
	}

	gJobTimer->Reserve(CLEANUP_TICK, shared_from_this(), make_shared<Job>(dynamic_pointer_cast<Room>(shared_from_this()), &Room::Cleanup));
}
