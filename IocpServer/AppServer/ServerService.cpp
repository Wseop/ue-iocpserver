#include "pch.h"
#include "ServerService.h"
#include "Listener.h"

ServerService::ServerService(NetAddress netAddress, SessionFactory sessionFactory, uint32 acceptCount) :
    Service(netAddress, sessionFactory),
    _acceptCount(acceptCount)
{
}

ServerService::~ServerService()
{
}

bool ServerService::Start()
{
    _listener = make_shared<Listener>(_acceptCount);
    _listener->SetService(shared_from_this());
    return _listener->StartAccept();
}
