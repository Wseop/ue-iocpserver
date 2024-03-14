#include "pch.h"
#include "Room.h"
#include "Player.h"
#include "ObjectManager.h"
#include "Utils.h"
#include "PacketSession.h"
#include "ServerPacketHandler.h"
#include "Job.h"

shared_ptr<Room> gRoom = make_shared<Room>();

Room::Room()
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

	cout << "Session Enter - " << sessionId << endl;

	return true;
}

bool Room::Exit(uint32 sessionId)
{
	lock_guard<mutex> lock(_mutex);

	if (_sessions.find(sessionId) == _sessions.end())
		return false;

	_sessions.erase(sessionId);

	cout << "Session Exit - " << sessionId << endl;

	return true;
}
