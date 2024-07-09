#include "pch.h"
#include "NetworkService.h"
#include "ThreadManager.h"
#include "IocpCore.h"
#include "Session.h"
#include "Listener.h"

NetworkService::NetworkService(NetAddress netAddress, SessionFactory sessionFactory) :
	_netAddress(netAddress),
	_sessionFactory(sessionFactory),
	_listener(nullptr)
{
	// Run Worker
	for (uint32 i = 0; i < thread::hardware_concurrency(); i++)
	{
		ThreadManager::instance()->launch([]()
			{
				while (true)
				{
					gIocpCore->dispatchEvent(10);
					ThreadManager::instance()->executeJobQueue();
					ThreadManager::instance()->distributeReservedJob();
				}
			});
	}
}

NetworkService::~NetworkService()
{
}

bool NetworkService::listen(uint32 acceptCount)
{
	if (_listener != nullptr)
		return true;

	_listener = make_shared<Listener>();
	_listener->setService(shared_from_this());
	return _listener->start(acceptCount);
}

shared_ptr<Session> NetworkService::createSession()
{
	static atomic<uint32> sSessionId = 1;
	shared_ptr<Session> session = _sessionFactory();

	if (session == nullptr)
		return nullptr;

	session->setSessionId(sSessionId.fetch_add(1));
	session->setService(shared_from_this());

	return session;
}

void NetworkService::addSession(shared_ptr<Session> session)
{
	if (session == nullptr)
		return;

	const uint32 sessionId = session->getSessionId();

	lock_guard<mutex> lock(_mutex);

	// 중복이면 기존 세션 제거 후 새 세션을 추가
	if (_sessions.find(sessionId) != _sessions.end())
		_sessions.erase(sessionId);

	_sessions[sessionId] = session;
}

void NetworkService::removeSession(shared_ptr<Session> session)
{
	if (session == nullptr)
		return;

	lock_guard<mutex> lock(_mutex);

	_sessions.erase(session->getSessionId());
}

shared_ptr<Session> NetworkService::getSession(uint32 sessionId)
{
	lock_guard<mutex> lock(_mutex);

	auto findIt = _sessions.find(sessionId);
	if (findIt == _sessions.end())
		return nullptr;

	return findIt->second;
}
