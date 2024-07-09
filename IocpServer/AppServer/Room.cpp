#include "pch.h"
#include "Room.h"
#include "Player.h"
#include "ObjectManager.h"
#include "Utils.h"
#include "Session.h"
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

void Room::enter(shared_ptr<PacketSession> session, Protocol::C_Enter payload)
{
	const uint32 sessionId = session->getSessionId();
	
	Protocol::S_Enter enterPayload;
	enterPayload.set_session_id(sessionId);

	// 중복 체크
	if (_sessions.find(sessionId) != _sessions.end())
	{
		enterPayload.set_result(false);
		session->send(ServerPacketHandler::makeS_Enter(&enterPayload));
		spdlog::warn("Room : Session[{}] Duplicated. Enter Fail.", sessionId);
		return;
	}

	// 세션 추가
	_sessions[sessionId] = session;

	// 다른 세션들의 Player 정보를 취합하여 입장한 세션으로 전송
	for (auto playerIt = _players.begin(); playerIt != _players.end(); playerIt++)
	{
		shared_ptr<Player> player = playerIt->second;
		
		if (player == nullptr || player->GetSession()->getSessionId() == sessionId)
			continue;

		enterPayload.add_other_object_infos()->CopyFrom(*player->GetObjectInfo());
	}
	enterPayload.set_result(true);
	session->send(ServerPacketHandler::makeS_Enter(&enterPayload));

	spdlog::info("Room : Session[{}] Enter.", sessionId);
}

void Room::exit(shared_ptr<PacketSession> session, Protocol::C_Exit payload)
{
	const uint32 sessionId = session->getSessionId();

	Protocol::S_Exit exitPacket;
	exitPacket.set_session_id(sessionId);

	// 세션 유무 확인
	if (_sessions.find(sessionId) == _sessions.end())
	{
		exitPacket.set_result(false);
		session->send(ServerPacketHandler::makeS_Exit(&exitPacket));
		return;
	}
	
	// 세션 제거 및 Exit 패킷 전송
	_sessions.erase(sessionId);
	exitPacket.set_result(true);
	session->send(ServerPacketHandler::makeS_Exit(&exitPacket));

	// 나간 세션의 플레이어들 Despawn
	Protocol::S_Despawn despawnPayload;
	for (auto playerIt = _players.begin(); playerIt != _players.end(); playerIt++)
	{
		shared_ptr<Player> player = playerIt->second;

		if (player == nullptr || player->GetSession()->getSessionId() != sessionId)
			continue;

		despawnPayload.add_object_ids(player->GetObjectId());
	}
	broadcast(ServerPacketHandler::makeS_Despawn(&despawnPayload));

	for (uint32 playerId : despawnPayload.object_ids())
		despawnPlayer(playerId);

	spdlog::info("Room : Session[{}] Exit.", sessionId);
}

void Room::spawnPlayer(shared_ptr<PacketSession> session)
{
	const uint32 sessionId = session->getSessionId();

	// 방에 있는 세션인지 체크
	if (_sessions.find(sessionId) == _sessions.end())
		return;

	// 플레이어 스폰
	shared_ptr<Player> player = ObjectManager::CreatePlayer(session);
	player->SetX(Utils::getRandom(-500.f, 500.f));
	player->SetY(Utils::getRandom(-500.f, 500.f));
	player->SetZ(100.f);
	_players[player->GetObjectId()] = player;

	// 스폰 정보 전송
	Protocol::S_Spawn spawnPayload;
	spawnPayload.mutable_object_info()->CopyFrom(*player->GetObjectInfo());
	spawnPayload.set_is_mine(true);
	session->send(ServerPacketHandler::makeS_Spawn(&spawnPayload));

	// 다른 세션들에게 스폰 정보 전송
	spawnPayload.set_is_mine(false);
	broadcast(ServerPacketHandler::makeS_Spawn(&spawnPayload), sessionId);

	spdlog::info("Room : Spawn Player[{}]. Session[{}].", player->GetObjectId(), sessionId);
}

void Room::despawnPlayer(uint32 playerId)
{
	auto findIt = _players.find(playerId);
	if (findIt == _players.end())
		return;

	_players.erase(playerId);

	spdlog::info("Room : Despawn Player[{}].", playerId);
}

void Room::movePlayer(shared_ptr<PacketSession> session, Protocol::C_Move payload)
{
	// 세션 입장 여부 확인
	if (_sessions.find(session->getSessionId()) == _sessions.end())
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

	spdlog::info("Room : Move Player[{}] to [{}, {}, {}]", player->GetObjectId(), posInfo.x(), posInfo.y(), posInfo.z());

	// Move 패킷 Broadcast
	Protocol::S_Move movePacket;
	movePacket.mutable_pos_info()->CopyFrom(posInfo);
	broadcast(ServerPacketHandler::makeS_Move(&movePacket));
}

void Room::broadcast(shared_ptr<SendBuffer> sendBuffer)
{
	for (auto iter = _sessions.begin(); iter != _sessions.end(); iter++)
	{
		shared_ptr<Session> session = iter->second.lock();
		if (session == nullptr)
			continue;
		session->send(sendBuffer);
	}
}

void Room::broadcast(shared_ptr<SendBuffer> sendBuffer, uint32 exceptId)
{
	for (auto iter = _sessions.begin(); iter != _sessions.end(); iter++)
	{
		shared_ptr<Session> session = iter->second.lock();
		if (session == nullptr || session->getSessionId() == exceptId)
			continue;
		session->send(sendBuffer);
	}
}

void Room::cleanup()
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

	gJobTimer->reserveJob(CLEANUP_TICK, shared_from_this(), make_shared<Job>(dynamic_pointer_cast<Room>(shared_from_this()), &Room::cleanup));
}
