#include "pch.h"
#include "GameInstance.h"
#include "PacketSession.h"
#include "ClientPacketHandler.h"
#include "Player.h"

shared_ptr<GameInstance> gGameInstance = make_shared<GameInstance>();

GameInstance::GameInstance()
{
}

GameInstance::~GameInstance()
{
}

void GameInstance::Enter(shared_ptr<Session> session)
{
	if (_sessionIds.find(session->GetSessionId()) != _sessionIds.end())
		return;
	_sessionIds.insert(session->GetSessionId());

	Protocol::C_Enter payload;
	session->Send(ClientPacketHandler::MakeC_Enter(&payload));
}

void GameInstance::HandleEnter(shared_ptr<Session> session, const Protocol::S_Enter enterPacket)
{
	if (enterPacket.result())
	{
		if (_players.find(session->GetSessionId()) != _players.end())
		{
			spdlog::error("[{}] : Player Duplicated", session->GetSessionId());
			return;
		}

		const Protocol::ObjectInfo& objectInfo = enterPacket.my_object_info();
		shared_ptr<Player> player = make_shared<Player>(objectInfo.object_id(), session);
		player->SetPosInfo(objectInfo.pos_info());
		_players[session->GetSessionId()] = player;
		spdlog::info("[{}] : Player Spawn : {}", session->GetSessionId(), player->GetObjectId());
	}
	else
	{
		_sessionIds.erase(session->GetSessionId());
		spdlog::warn("[{}] : Enter Fail", session->GetSessionId());
	}
}

void GameInstance::Exit(shared_ptr<Session> session)
{
	if (_sessionIds.find(session->GetSessionId()) == _sessionIds.end())
		return;
	_sessionIds.erase(session->GetSessionId());

	Protocol::C_Exit payload;
	session->Send(ClientPacketHandler::MakeC_Exit(&payload));
}

void GameInstance::HandleExit(shared_ptr<Session> session, const Protocol::S_Exit exitPacket)
{
	if (exitPacket.result())
	{
		auto findIt = _players.find(session->GetSessionId());
		if (findIt == _players.end())
			return;
		
		shared_ptr<Player> player = findIt->second;
		if (player == nullptr)
			return;

		_players.erase(session->GetSessionId());
		spdlog::info("[{}] : Player Despawn : {}", session->GetSessionId(), player->GetObjectId());
	}
	else
	{
		spdlog::warn("[{}] : Exit Fail", session->GetSessionId());
	}
}
