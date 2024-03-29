#include "pch.h"
#include "GameInstance.h"
#include "PacketSession.h"
#include "ClientPacketHandler.h"
#include "Player.h"
#include "Utils.h"

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
			spdlog::error("Session[{}] : Player Duplicated", session->GetSessionId());
			return;
		}

		const Protocol::ObjectInfo& objectInfo = enterPacket.my_object_info();
		shared_ptr<Player> player = make_shared<Player>(objectInfo.object_id(), session);
		player->SetPosInfo(objectInfo.pos_info());
		_players[session->GetSessionId()] = player;
		spdlog::info("Session[{}] : Player Spawn : {}", session->GetSessionId(), player->GetObjectId());
	}
	else
	{
		_sessionIds.erase(session->GetSessionId());
		spdlog::warn("Session[{}] : Enter Fail", session->GetSessionId());
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
		spdlog::info("Session[{}] : Player Despawn : {}", session->GetSessionId(), player->GetObjectId());
	}
	else
	{
		spdlog::warn("Session[{}] : Exit Fail", session->GetSessionId());
	}
}

void GameInstance::Move(shared_ptr<Session> session)
{
	if (_sessionIds.find(session->GetSessionId()) == _sessionIds.end())
		return;
	
	auto findIt = _players.find(session->GetSessionId());
	if (findIt == _players.end())
		return;

	shared_ptr<Player> player = findIt->second;
	if (player == nullptr)
		return;

	Protocol::PosInfo* posInfo = player->GetPosInfo();
	if (posInfo->move_state() == Protocol::MOVE_STATE_IDLE)
	{
		posInfo->set_move_state(Protocol::MOVE_STATE_RUN);
		posInfo->set_x(Utils::GetRandom(-3500.f, 3500.f));
		posInfo->set_y(Utils::GetRandom(-3500.f, 3500.f));
	}
	else if (posInfo->move_state() == Protocol::MOVE_STATE_RUN)
	{
		posInfo->set_move_state(Protocol::MOVE_STATE_IDLE);
	}

	Protocol::C_Move payload;
	payload.mutable_pos_info()->CopyFrom(*posInfo);
	session->Send(ClientPacketHandler::MakeC_Move(&payload));

	spdlog::info("Session[{}] : Player[{}] : Move[{}]", session->GetSessionId(), player->GetObjectId(), static_cast<int32>(posInfo->move_state()));
}
