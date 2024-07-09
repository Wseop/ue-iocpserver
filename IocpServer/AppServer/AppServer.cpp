#include "pch.h"
#include "NetworkService.h"
#include "Session.h"
#include "ServerPacketHandler.h"
#include "JobTimer.h"
#include "Room.h"
#include "Job.h"

int main()
{
#ifdef _DEBUG
    spdlog::set_level(spdlog::level::debug);
#else
    spdlog::set_level(spdlog::level::info);
#endif // _DEBUG

    ServerPacketHandler::init();

    shared_ptr<NetworkService> service = make_shared<NetworkService>(NetAddress("127.0.0.1", 7777), []() { return make_shared<PacketSession>(); });
    assert(service->listen(10));

    gJobTimer->reserveJob(gRoom->getCleanupTick(), dynamic_pointer_cast<JobQueue>(gRoom), make_shared<Job>(gRoom, &Room::cleanup));

    while (true)
    {}
}
