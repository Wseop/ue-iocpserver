#include "pch.h"
#include "Room.h"
#include "Player.h"
#include "ObjectManager.h"
#include "Utils.h"
#include "PacketSession.h"
#include "ServerPacketHandler.h"
#include "JobQueue.h"
#include "Job.h"

shared_ptr<Room> gRoom = make_shared<Room>();

Room::Room()
{
}

Room::~Room()
{
}

void Room::Enter(shared_ptr<Session> session, const string key)
{
	const uint32 sessionId = session->GetSessionId();

	// key 체크
	if (key != "123")
	{
		session->Send(ServerPacketHandler::MakeS_Enter(false, sessionId, nullptr));
		return;
	}

	// 세션 중복 체크
	if (_sessions.find(sessionId) != _sessions.end())
		return;

	// 플레이어 스폰
	shared_ptr<Player> player = SpawnPlayer(session);
	if (player == nullptr)
		return;

	// 스폰 위치 전송
	session->Send(ServerPacketHandler::MakeS_Enter(true, sessionId, &player->GetPlayerInfo()));

	// 방에 스폰되어 있는 플레이어들을 새로 입장한 세션으로 전송
	vector<shared_ptr<Player>> others;
	for (auto iter = _players.begin(); iter != _players.end(); iter++)
	{
		shared_ptr<Player> other = iter->second;
		if (other->GetPlayerId() == player->GetPlayerId())
			continue;
		others.push_back(other);
	}
	session->Send(ServerPacketHandler::MakeS_Spawn(others));

	// 새로 스폰된 플레이어 정보를 방에 있는 다른 세션들에게 Broadcast
	Broadcast(ServerPacketHandler::MakeS_Spawn({ player }));

	// 방에 세션 추가
	_sessions[sessionId] = session;

	cout << format("[Room] Session {} Enter", sessionId) << endl;
}

void Room::Exit(shared_ptr<Session> session, const uint32 enterId)
{
	const uint32 sessionId = session->GetSessionId();

	// SessionId와 EnterId는 동일해야함
	if (sessionId != enterId)
	{
		session->Send(ServerPacketHandler::MakeS_Exit(false, sessionId));
		return;
	}
	else
	{
		session->Send(ServerPacketHandler::MakeS_Exit(true, sessionId));
	}

	// 방에서 세션 제거
	if (_sessions.find(sessionId) == _sessions.end())
		return;
	_sessions.erase(sessionId);
	
	// 퇴장하는 세션이 생성한 플레이어 검색
	shared_ptr<Player> player = nullptr;
	for (auto iter = _players.begin(); iter != _players.end(); iter++)
	{
		player = iter->second;
		if (player == nullptr)
			continue;

		shared_ptr<Session> playerSession = player->GetSession();
		if (playerSession == nullptr)
			continue;
		if (playerSession == session)
			break;
	}
	if (player == nullptr)
		return;

	// 플레이어 Despawn
	DespawnPlayer(player->GetPlayerId());

	// 다른 세션들에게 퇴장알림 - Broadcast
	shared_ptr<SendBuffer> sendBuffer = ServerPacketHandler::MakeS_Despawn({ player->GetPlayerId()});
	Broadcast(sendBuffer);

	cout << format("[Room] Session {} Exit", sessionId) << endl;
}

void Room::MovePlayer(shared_ptr<Session> playerOwner, Protocol::PlayerInfo playerInfo)
{
	// 검증) 세션이 방에 있는지 체크
	if (_sessions.find(playerOwner->GetSessionId()) == _sessions.end())
		return;

	// 검증) 플레이어가 방에 있는지 체크
	auto findResult = _players.find(playerInfo.player_id());
	if (findResult == _players.end() || findResult->second == nullptr)
		return;

	shared_ptr<Player> player = findResult->second;
	// 검증) 이동시킬 플레이어의 Owner와 세션이 일치하는지 체크
	if (playerOwner->GetSessionId() != player->GetSession()->GetSessionId())
		return;

	// 플레이어 이동 & Broadcast
	if (player->Move(playerInfo))
		Broadcast(ServerPacketHandler::MakeS_Move(&playerInfo));
}

shared_ptr<Player> Room::SpawnPlayer(weak_ptr<Session> session)
{
	shared_ptr<Player> player = ObjectManager::CreatePlayer(session);
	if (player == nullptr)
		return nullptr;

	const uint32 playerId = player->GetPlayerId();
	// 중복 체크
	if (_players.find(playerId) != _players.end())
		return nullptr;

	// 스폰 위치 지정
	player->SetX(Utils::GetRandom(-1000.f, 1000.f));
	player->SetY(Utils::GetRandom(-1000.f, 1000.f));
	player->SetZ(100.f);
	// 플레이어 추가
	_players[playerId] = player;

	return player;
}

void Room::DespawnPlayer(uint32 playerId)
{
	if (_players.find(playerId) == _players.end())
		return;

	_players.erase(playerId);
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
