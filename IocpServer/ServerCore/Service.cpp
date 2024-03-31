#include "pch.h"
#include "Service.h"
#include "Session.h"

Service::Service(NetAddress netAddress, SessionFactory sessionFactory) :
    _netAddress(netAddress),
    _sessionFactory(sessionFactory)
{
}

Service::~Service()
{
}

shared_ptr<Session> Service::CreateSession()
{
    static atomic<uint32> sSessionId = 0;

    shared_ptr<Session> session = _sessionFactory();
    session->SetSessionId(sSessionId.fetch_add(1));
    session->SetService(shared_from_this());

    return session;
}

shared_ptr<Session> Service::GetSession(uint32 sessionId)
{
    lock_guard<mutex> lock(_mutex);

    if (_sessions.find(sessionId) == _sessions.end())
        return nullptr;

    return _sessions[sessionId];
}

void Service::AddSession(shared_ptr<Session> session)
{
    if (session == nullptr)
        return;

    lock_guard<mutex> lock(_mutex);
    
    auto findIt = _sessions.find(session->GetSessionId());
    if (findIt != _sessions.end())
    {
        // 技记 ID 吝汗 惯积. 扁粮 技记 Disconnect 贸府
        shared_ptr<Session> prevSession = findIt->second;
        if (prevSession)
            prevSession->Disconnect();
    }
    _sessions[session->GetSessionId()] = session;
}

void Service::RemoveSession(uint32 sessionId)
{
    lock_guard<mutex> lock(_mutex);

    _sessions.erase(sessionId);
}
