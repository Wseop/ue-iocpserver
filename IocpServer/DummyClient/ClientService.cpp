#include "pch.h"
#include "ClientService.h"
#include "ClientPacketHandler.h"

ClientService::ClientService(NetAddress netAddress, SessionFactory sessionFactory) :
    Service(netAddress, sessionFactory)
{
    ClientPacketHandler::Init();
}

ClientService::~ClientService()
{
}
