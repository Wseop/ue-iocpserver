#include "pch.h"
#include "ClientService.h"
#include "PacketSession.h"

ClientService::ClientService(NetAddress netAddress, SessionFactory sessionFactory, uint32 sessionCount) :
    Service(netAddress, sessionFactory),
    _sessionCount(sessionCount)
{
}

ClientService::~ClientService()
{
}

bool ClientService::Start()
{
    for (uint32 i = 0; i < _sessionCount; i++)
    {
        shared_ptr<Session> session = CreateSession();
        if (session->Connect() == false)
        {
            spdlog::error("Failed to connect server");
            return false;
        }
    }
    spdlog::info("{} Sessions Connected to server", _sessionCount);

    return true;
}