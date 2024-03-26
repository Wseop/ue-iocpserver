#include "pch.h"
#include "ServerService.h"
#include "Listener.h"
#include "JobTimer.h"
#include "Room.h"
#include "Job.h"

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
    gJobTimer->Reserve(gRoom->GetCleanupTick(), dynamic_pointer_cast<JobQueue>(gRoom), make_shared<Job>(gRoom, &Room::Cleanup));

    _listener = make_shared<Listener>(_acceptCount);
    _listener->SetService(shared_from_this());
    return _listener->StartAccept();
}
