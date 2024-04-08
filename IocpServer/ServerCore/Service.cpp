#include "pch.h"
#include "Service.h"
#include "ThreadManager.h"
#include "IocpCore.h"
#include "Session.h"

Service::Service(NetAddress netAddress, SessionFactory sessionFactory) :
	_iocpCore(make_shared<IocpCore>()),
	_netAddress(netAddress),
	_sessionFactory(sessionFactory)
{
	for (uint32 i = 0; i < thread::hardware_concurrency(); i++)
	{
		gThreadManager->Launch([this]()
			{
				_iocpCore->Dispatch(10);
			});
	}
}

Service::~Service()
{
}

shared_ptr<Session> Service::CreateSession()
{
	static atomic<uint32> sSessionId = 1;
	shared_ptr<Session> session = _sessionFactory();

	if (session == nullptr)
		return nullptr;

	session->SetSessionId(sSessionId.fetch_add(1));
	session->SetService(shared_from_this());

	return session;
}

void Service::AddSession(shared_ptr<Session> session)
{
	if (session == nullptr)
		return;

	const uint32 sessionId = session->GetSessionId();

	lock_guard<mutex> lock(_mutex);

	// 중복이면 기존 세션 제거 후 새 세션을 추가
	if (_sessions.find(sessionId) != _sessions.end())
		_sessions.erase(sessionId);

	_sessions[sessionId] = session;
}

void Service::RemoveSession(shared_ptr<Session> session)
{
	if (session == nullptr)
		return;

	lock_guard<mutex> lock(_mutex);

	_sessions.erase(session->GetSessionId());
}

shared_ptr<Session> Service::GetSession(uint32 sessionId)
{
	lock_guard<mutex> lock(_mutex);

	auto findIt = _sessions.find(sessionId);
	if (findIt == _sessions.end())
		return nullptr;

	return findIt->second;
}
