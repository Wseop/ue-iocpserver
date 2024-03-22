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

void Room::Enter(shared_ptr<Session> session)
{
	lock_guard<mutex> lock(_mutex);

	const uint32 sessionId = session->GetSessionId();

	// ���� �ߺ� üũ
	if (_sessions.find(sessionId) != _sessions.end())
		return;

	// �÷��̾� ����
	shared_ptr<Player> player = SpawnPlayer(session);

	if (player == nullptr)
		return;

	// ���� ��ġ ����
	session->Send(ServerPacketHandler::MakeS_Enter(true, sessionId, &player->GetPlayerInfo()));

	// �濡 �����Ǿ� �ִ� �÷��̾�� ����
	vector<shared_ptr<Player>> others;

	for (auto iter = _players.begin(); iter != _players.end(); iter++)
	{
		shared_ptr<Player> other = iter->second;

		if (player == nullptr || other->GetPlayerId() == player->GetPlayerId())
			continue;

		others.push_back(other);
	}

	session->Send(ServerPacketHandler::MakeS_Spawn(others));

	// �濡 �ִ� �ٸ� ���ǵ鿡�� Broadcast
	Broadcast(ServerPacketHandler::MakeS_Spawn({ player }));

	_sessions[sessionId] = session;

	cout << format("[Room] Session {} Enter", sessionId) << endl;
}

bool Room::Exit(uint32 sessionId)
{
	lock_guard<mutex> lock(_mutex);

	// �濡 �����ϴ� �������� üũ
	if (_sessions.find(sessionId) == _sessions.end())
		return false;

	_sessions.erase(sessionId);
	
	// �ش� ������ ������ �÷��̾� �˻�
	shared_ptr<Player> player = nullptr;

	for (auto iter = _players.begin(); iter != _players.end(); iter++)
	{
		player = iter->second;

		if (player == nullptr)
			continue;

		shared_ptr<Session> session = player->GetSession();

		if (session == nullptr)
			continue;

		if (session->GetSessionId() == sessionId)
			break;
	}

	// �ش� ������ ������ �÷��̾ ������ ����
	if (player == nullptr)
		return true;

	// �÷��̾� Despawn
	DespawnPlayer(player->GetPlayerId());

	// �ٸ� ���ǵ鿡�� Broadcast
	shared_ptr<SendBuffer> sendBuffer = ServerPacketHandler::MakeS_Despawn({ player->GetPlayerId()});
	Broadcast(sendBuffer);

	cout << format("[Room] Session {} Exit", sessionId) << endl;

	return true;
}

void Room::MovePlayer(shared_ptr<Session> playerOwner, Protocol::PlayerInfo& playerInfo)
{
	lock_guard<mutex> lock(_mutex);

	// ����) ����, �÷��̾ �濡 �ִ��� üũ
	if (_sessions.find(playerOwner->GetSessionId()) == _sessions.end())
		return;

	const uint32 playerId = playerInfo.player_id();
	auto findResult = _players.find(playerId);

	if (findResult == _players.end())
		return;

	shared_ptr<Player> player = findResult->second;

	// ����) �̵���ų �÷��̾��� Owner�� ������ ��ġ�ϴ��� üũ
	if (playerOwner->GetSessionId() != player->GetSession()->GetSessionId())
		return;

	// �÷��̾� �̵� & Broadcast
	if (player->Move(playerInfo))
		Broadcast(ServerPacketHandler::MakeS_Move(&playerInfo));
}

shared_ptr<Player> Room::SpawnPlayer(weak_ptr<Session> session)
{
	shared_ptr<Player> player = ObjectManager::CreatePlayer(session);

	if (player == nullptr)
		return nullptr;

	const uint32 playerId = player->GetPlayerId();

	// �ߺ� üũ
	if (_players.find(playerId) != _players.end())
		return nullptr;

	// ���� ��ġ ����
	player->SetX(Utils::GetRandom(-1000.f, 1000.f));
	player->SetY(Utils::GetRandom(-1000.f, 1000.f));
	player->SetZ(100.f);

	// �÷��̾� �߰�
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
