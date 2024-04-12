#include "pch.h"
#include "GameInstance.h"
#include "ClientPacketHandler.h"
#include "PacketSession.h"
#include "Player.h"

shared_ptr<GameInstance> gGameInstance = make_shared<GameInstance>();

GameInstance::GameInstance()
{
}

GameInstance::~GameInstance()
{
}

void GameInstance::EnterGameRoom(shared_ptr<Session> session)
{
	Protocol::C_Enter payload;
	session->Send(ClientPacketHandler::MakeC_Enter(&payload));
}

void GameInstance::HandleEnterGameRoom(Protocol::S_Enter payload)
{
	if (payload.result())
	{
		uint32 playerCount = 0;
		for (const Protocol::ObjectInfo& playerInfo : payload.other_object_infos())
		{
			shared_ptr<Player> player = SpawnPlayer(playerInfo);
			if (player == nullptr)
				continue;
			_otherPlayers[player->GetObjectId()] = player;
			playerCount++;
		}

		spdlog::info("GameInstance : EnterGameRoom Success. [{}] Players Exist.", playerCount);
	}
	else
	{
		spdlog::error("GameInstance : EnterGameRoom Fail.");
	}
}

void GameInstance::ExitGameRoom(shared_ptr<Session> session)
{
	Protocol::C_Exit payload;
	session->Send(ClientPacketHandler::MakeC_Exit(&payload));
}

void GameInstance::HandleExitGameRoom(Protocol::S_Exit payload)
{
	if (payload.result())
	{
		// 플레이어 정보 삭제
		_myPlayers.clear();
		_otherPlayers.clear();

		spdlog::info("GameInstance : ExitGameRoom Success.");
	}
	else
	{
		spdlog::error("GameInstance : ExitGameRoom Fail.");
	}
}

void GameInstance::SpawnMyPlayer(shared_ptr<Session> session)
{
	Protocol::C_Spawn payload;
	session->Send(ClientPacketHandler::MakeC_Spawn(&payload));
}

void GameInstance::HandleDespawnPlayer(Protocol::S_Despawn payload)
{
	for (const uint32 playerId : payload.object_ids())
		DespawnPlayer(playerId);
}

void GameInstance::MoveMyPlayersToOther(shared_ptr<Session> session)
{
	auto otherIt = _otherPlayers.begin();
	if (otherIt == _otherPlayers.end())
		return;

	shared_ptr<Player> other = otherIt->second;
	if (other == nullptr)
		return;

	Protocol::PosInfo targetPos = *other->GetPosInfo();

	for (auto playerIt = _myPlayers.begin(); playerIt != _myPlayers.end(); playerIt++)
	{
		shared_ptr<Player> player = playerIt->second;
		if (player == nullptr)
			continue;

		targetPos.set_object_id(player->GetObjectId());
		player->SetPosInfo(targetPos);

		Protocol::C_Move payload;
		payload.mutable_pos_info()->CopyFrom(targetPos);
		session->Send(ClientPacketHandler::MakeC_Move(&payload));
	}

	spdlog::info("GameInstance : Move Players to [{}, {}, {}]", targetPos.x(), targetPos.y(), targetPos.z());
}

void GameInstance::HandleMovePlayer(Protocol::S_Move payload)
{
	const Protocol::PosInfo& posInfo = payload.pos_info();
	auto playerIt = _otherPlayers.find(posInfo.object_id());
	if (playerIt == _otherPlayers.end())
		return;

	shared_ptr<Player> player = playerIt->second;
	if (player == nullptr)
		return;

	player->SetPosInfo(posInfo);
}

void GameInstance::HandleSpawnPlayer(Protocol::S_Spawn payload)
{
	shared_ptr<Player> player = SpawnPlayer(payload.object_info());
	if (player == nullptr)
		return;

	const uint32 playerId = player->GetObjectId();

	if (payload.is_mine())
	{
		_myPlayers[playerId] = player;
		//spdlog::info("GameInstance : Spawn My Player. ID[{}].", playerId);
	}
	else
	{
		_otherPlayers[playerId] = player;
		spdlog::info("GameInstance : Spawn Other Player. ID[{}].", playerId);
	}
}

shared_ptr<Player> GameInstance::SpawnPlayer(const Protocol::ObjectInfo& playerInfo)
{
	const uint32 playerId = playerInfo.object_id();

	if (_myPlayers.find(playerId) != _myPlayers.end() || _otherPlayers.find(playerId) != _otherPlayers.end())
	{
		spdlog::error("GameInstance : Player ID Duplicated.");
		return nullptr;
	}

	shared_ptr<Player> player = make_shared<Player>(playerId);
	player->SetPosInfo(playerInfo.pos_info());
	return player;
}

void GameInstance::DespawnPlayer(uint32 playerId)
{
	size_t eraseCount = 0;

	eraseCount = _myPlayers.erase(playerId);
	if (eraseCount == 0)
		eraseCount = _otherPlayers.erase(playerId);

	if (eraseCount != 0)
		spdlog::info("GameInstance : Despawn Player. ID[{}].", playerId);
}