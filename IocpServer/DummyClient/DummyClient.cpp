#include "pch.h"
#include "ClientService.h"
#include "PacketSession.h"
#include "IocpCore.h"
#include "Job.h"
#include "GameInstance.h"

int main()
{
#ifdef _DEBUG
    spdlog::set_level(spdlog::level::debug);
#else
    spdlog::set_level(spdlog::level::info);
#endif // _DEBUG

    shared_ptr<ClientService> service = make_shared<ClientService>(NetAddress(L"127.0.0.1", 7777), []() { return make_shared<PacketSession>(); });

    while(true)
    {}
}