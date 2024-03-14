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

Room::Room() :
	_jobQueue(make_shared<JobQueue>())
{
}

Room::~Room()
{
}

bool Room::Enter(shared_ptr<Session> session)
{
	lock_guard<mutex> lock(_mutex);

	const uint32 sessionId = session->GetSessionId();

	// 중복 체크
	if (_sessions.find(sessionId) != _sessions.end())
		return false;

	_sessions[sessionId] = session;

	// 방에 스폰되어 있는 플레이어들 전송
	vector<shared_ptr<Player>> players;

	for (auto iter = _players.begin(); iter != _players.end(); iter++)
	{
		shared_ptr<Player> player = iter->second;

		if (player == nullptr)
			continue;

		players.push_back(player);
	}

	session->Send(ServerPacketHandler::MakeS_Spawn(true, players));

	cout << format("[Room] Session {} Enter", sessionId) << endl;

	return true;
}

bool Room::Exit(uint32 sessionId)
{
	lock_guard<mutex> lock(_mutex);

	if (_sessions.find(sessionId) == _sessions.end())
		return false;

	_sessions.erase(sessionId);
	
	// 해당 session이 생성시킨 player들의 id 수집 및 제거
	vector<uint32> playerIds;

	for (auto iter = _players.begin(); iter != _players.end(); iter++)
	{
		shared_ptr<Player> player = iter->second;

		if (player == nullptr)
			continue;

		shared_ptr<Session> playerSession = player->GetSession();

		if (playerSession == nullptr)
			continue;

		if (playerSession->GetSessionId() == sessionId)
			playerIds.push_back(player->GetPlayerId());
	}

	for (uint32 id : playerIds)
	{
		_players.erase(id);
	}

	// 해당하는 player들을 despawn시키는 메세지를 broadcasting
	shared_ptr<SendBuffer> sendBuffer = ServerPacketHandler::MakeS_Despawn(playerIds);

	for (auto iter = _sessions.begin(); iter != _sessions.end(); iter++)
	{
		shared_ptr<Session> session = iter->second.lock();

		if (session == nullptr)
			continue;

		session->Send(sendBuffer);
	}

	cout << format("[Room] Session {} Exit", sessionId) << endl;

	return true;
}

bool Room::Spawn(uint32 sessionId, uint32 spawnCount)
{
	lock_guard<mutex> lock(_mutex);

	// 세션이 유효하지 않으면 종료
	if (_sessions.find(sessionId) == _sessions.end())
		return false;

	shared_ptr<Session> session = _sessions[sessionId].lock();

	if (session == nullptr)
		return false;

	// Player 생성
	vector<shared_ptr<Player>> spawnedPlayers;

	for (uint32 i = 0; i < spawnCount; i++)
	{
		shared_ptr<Player> player = ObjectManager::CreatePlayer(session);

		if (player == nullptr)
			continue;

		if (_players.find(player->GetPlayerId()) != _players.end())
			continue;

		// 랜덤 위치 지정
		player->SetX(Utils::GetRandom(-1000.f, 1000.f));
		player->SetY(Utils::GetRandom(-1000.f, 1000.f));
		player->SetZ(Utils::GetRandom(100.f, 1000.f));

		spawnedPlayers.push_back(player);

		_players[player->GetPlayerId()] = player;
	}

	// 방에 있는 세션들에게 스폰 정보 Broadcasting
	shared_ptr<SendBuffer> sendBuffer = ServerPacketHandler::MakeS_Spawn(true, spawnedPlayers);

	for (auto iter = _sessions.begin(); iter != _sessions.end(); iter++)
	{
		session = iter->second.lock();

		if (session == nullptr)
			continue;

		session->Send(sendBuffer);
	}

	return true;
}