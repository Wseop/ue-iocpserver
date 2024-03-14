#pragma once

class Player;
class Session;
class JobQueue;

class Room : public enable_shared_from_this<Room>
{
public:
	Room();
	virtual ~Room();

public:
	bool Enter(shared_ptr<Session> session);
	bool Exit(uint32 sessionId);

private:
	// Exit없이 종료된 세션 정보 정리
	void RemoveInvalidSessions();

private:
	mutex _mutex;
	unordered_map<uint32, weak_ptr<Session>> _sessions;

	shared_ptr<JobQueue> _jobQueue = nullptr;
};

extern shared_ptr<Room> gRoom;