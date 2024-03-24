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

	// key üũ
	if (key != "123")
	{
		session->Send(ServerPacketHandler::MakeS_Enter(false, sessionId, nullptr));
		return;
	}

	// ���� �ߺ� üũ
	if (_sessions.find(sessionId) != _sessions.end())
		return;

	// �÷��̾� ����
	shared_ptr<Player> player = SpawnPlayer(session);
	if (player == nullptr)
		return;

	// ���� ��ġ ����
	session->Send(ServerPacketHandler::MakeS_Enter(true, sessionId, &player->GetPlayerInfo()));

	// �濡 �����Ǿ� �ִ� �÷��̾���� ���� ������ �������� ����
	vector<shared_ptr<Player>> others;
	for (auto iter = _players.begin(); iter != _players.end(); iter++)
	{
		shared_ptr<Player> other = iter->second;
		if (other->GetPlayerId() == player->GetPlayerId())
			continue;
		others.push_back(other);
	}
	session->Send(ServerPacketHandler::MakeS_Spawn(others));

	// ���� ������ �÷��̾� ������ �濡 �ִ� �ٸ� ���ǵ鿡�� Broadcast
	Broadcast(ServerPacketHandler::MakeS_Spawn({ player }));

	// �濡 ���� �߰�
	_sessions[sessionId] = session;

	cout << format("[Room] Session {} Enter", sessionId) << endl;
}

void Room::Exit(shared_ptr<Session> session, const uint32 enterId)
{
	const uint32 sessionId = session->GetSessionId();

	// SessionId�� EnterId�� �����ؾ���
	if (sessionId != enterId)
	{
		session->Send(ServerPacketHandler::MakeS_Exit(false, sessionId));
		return;
	}
	else
	{
		session->Send(ServerPacketHandler::MakeS_Exit(true, sessionId));
	}

	// �濡�� ���� ����
	if (_sessions.find(sessionId) == _sessions.end())
		return;
	_sessions.erase(sessionId);
	
	// �����ϴ� ������ ������ �÷��̾� �˻�
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

	// �÷��̾� Despawn
	DespawnPlayer(player->GetPlayerId());

	// �ٸ� ���ǵ鿡�� ����˸� - Broadcast
	shared_ptr<SendBuffer> sendBuffer = ServerPacketHandler::MakeS_Despawn({ player->GetPlayerId()});
	Broadcast(sendBuffer);

	cout << format("[Room] Session {} Exit", sessionId) << endl;
}

void Room::MovePlayer(shared_ptr<Session> playerOwner, Protocol::PlayerInfo playerInfo)
{
	// ����) ������ �濡 �ִ��� üũ
	if (_sessions.find(playerOwner->GetSessionId()) == _sessions.end())
		return;

	// ����) �÷��̾ �濡 �ִ��� üũ
	auto findResult = _players.find(playerInfo.player_id());
	if (findResult == _players.end() || findResult->second == nullptr)
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
