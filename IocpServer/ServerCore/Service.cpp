#include "pch.h"
#include "Service.h"
#include "Session.h"

atomic<uint32> Service::sSessionId = 1;

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
    shared_ptr<Session> session = _sessionFactory();
    if (session == nullptr)
        return nullptr;

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

bool Service::AddSession(shared_ptr<Session> session)
{
    if (session == nullptr)
        return false;

    lock_guard<mutex> lock(_mutex);
    
    const uint32 sessionId = sSessionId.fetch_add(1);

    if (_sessions.find(sessionId) != _sessions.end())
        return false;

    session->SetSessionId(sessionId);
    _sessions[sessionId] = session;

    return true;
}

void Service::RemoveSession(uint32 sessionId)
{
    lock_guard<mutex> lock(_mutex);

    _sessions.erase(sessionId);
}
