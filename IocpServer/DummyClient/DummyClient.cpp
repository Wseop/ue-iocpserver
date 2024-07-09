#include "pch.h"
#include "NetworkService.h"
#include "ClientPacketHandler.h"
#include "Session.h"
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

    ClientPacketHandler::init();

    shared_ptr<NetworkService> service = make_shared<NetworkService>(NetAddress("127.0.0.1", 7777), []() { return make_shared<PacketSession>(); });
    shared_ptr<PacketSession> session = dynamic_pointer_cast<PacketSession>(service->CreateSession());

    // 서버 접속
    assert(session->connect());
    this_thread::sleep_for(100ms);

    while (true)
    {
        // 방에 입장
        {
            gGameInstance->push(make_shared<Job>(gGameInstance, &GameInstance::enterGameRoom, session));
            this_thread::sleep_for(10ms);
        }

        // 플레이어 스폰
        {
            uint32 spawnCount = 50;
            for (uint32 i = 0; i < spawnCount; i++)
                gGameInstance->push(make_shared<Job>(gGameInstance, &GameInstance::spawnMyPlayer, session));
            spdlog::info("Spawn {} Players", spawnCount);
            this_thread::sleep_for(3s);
        }

        uint64 start = ::GetTickCount64();
        while (true)
        {
            // 다른 세션의 플레이어를 찾아서 해당 위치로 이동
            {
                gGameInstance->push(make_shared<Job>(gGameInstance, &GameInstance::moveMyPlayersToOther, session));
                this_thread::sleep_for(500ms);
            }
            
            uint64 end = ::GetTickCount64();
            if (end - start > 1000 * 15)
                break;
        }

        // 방에서 퇴장
        gGameInstance->push(make_shared<Job>(gGameInstance, &GameInstance::exitGameRoom, session));
        this_thread::sleep_for(3s);
    }

    session->disconnect();
}