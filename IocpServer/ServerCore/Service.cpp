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
    static atomic<uint32> sSessionId = 1;

    shared_ptr<Session> session = _sessionFactory(sSessionId.fetch_add(1));

    if (session == nullptr)
        return nullptr;

    session->SetService(shared_from_this());

    return session;
}

void Service::AddSession(shared_ptr<Session> session)
{
    if (session == nullptr)
        return;

    lock_guard<mutex> lock(_mutex);

    _sessions.insert(session);
}

void Service::RemoveSession(shared_ptr<Session> session)
{
    if (session == nullptr)
        return;

    lock_guard<mutex> lock(_mutex);

    _sessions.erase(session);
}
