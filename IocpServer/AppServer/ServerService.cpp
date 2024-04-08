#include "pch.h"
#include "ServerService.h"
#include "Listener.h"
#include "JobTimer.h"
#include "Room.h"
#include "Job.h"
#include "ServerPacketHandler.h"

ServerService::ServerService(NetAddress netAddress, SessionFactory sessionFactory) :
    Service(netAddress, sessionFactory),
    _listener(make_shared<Listener>())
{
    ServerPacketHandler::Init();
}

ServerService::~ServerService()
{
}

bool ServerService::Start(uint32 acceptCount)
{
    _listener->SetService(shared_from_this());
    if (_listener->Start(acceptCount) == false)
        return false;

    spdlog::info("ServerService : Start Listeners[{}]", acceptCount);

    gJobTimer->ReserveJob(gRoom->GetCleanupTick(), dynamic_pointer_cast<JobQueue>(gRoom), make_shared<Job>(gRoom, &Room::Cleanup));

    return true;
}
