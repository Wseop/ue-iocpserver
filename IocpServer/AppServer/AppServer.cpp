#include "pch.h"
#include "ServerService.h"
#include "PacketSession.h"

int main()
{
#ifdef _DEBUG
    spdlog::set_level(spdlog::level::debug);
#else
    spdlog::set_level(spdlog::level::info);
#endif // _DEBUG

    shared_ptr<ServerService> service = make_shared<ServerService>(NetAddress(L"127.0.0.1", 7777), []() { return make_shared<PacketSession>(); });
    assert(service->Start(10));

    while (true)
    {}
}
