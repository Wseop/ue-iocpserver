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

	cout << "Session Enter - " << sessionId << endl;

	_jobQueue->Push(make_shared<Job>(shared_from_this(), &Room::RemoveInvalidSessions), true);

	return true;
}

bool Room::Exit(uint32 sessionId)
{
	lock_guard<mutex> lock(_mutex);

	if (_sessions.find(sessionId) == _sessions.end())
		return false;

	_sessions.erase(sessionId);

	cout << "Session Exit - " << sessionId << endl;

	_jobQueue->Push(make_shared<Job>(shared_from_this(), &Room::RemoveInvalidSessions), true);

	return true;
}

void Room::RemoveInvalidSessions()
{
	lock_guard<mutex> lock(_mutex);

	vector<uint32> removeIds;

	for (auto iter = _sessions.begin(); iter != _sessions.end(); iter++)
	{
		if ((iter->second).lock() == nullptr)
		{
			removeIds.push_back(iter->first);
		}
	}

	for (uint32 id : removeIds)
	{
		_sessions.erase(id);
	}

	cout << "[ROOM] 세션 정리 완료, 남은 세션 수 : " << _sessions.size() << endl;
}
