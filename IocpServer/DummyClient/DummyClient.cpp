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

    while (true)
    {
        shared_ptr<Session> session = service->CreateSession();
        this_thread::sleep_for(100ms);

        // 서버 접속
        {
            assert(session->Connect());
            this_thread::sleep_for(10ms);
        }

        // 방에 입장
        {
            gGameInstance->Push(make_shared<Job>(gGameInstance, &GameInstance::EnterGameRoom, session));
            this_thread::sleep_for(10ms);
        }

        // 플레이어 스폰
        {
            for (uint32 i = 0; i < 10; i++)
                gGameInstance->Push(make_shared<Job>(gGameInstance, &GameInstance::SpawnMyPlayer, session));
            this_thread::sleep_for(3s);
        }

        uint64 start = ::GetTickCount64();
        while (true)
        {
            // 다른 세션의 플레이어를 찾아서 해당 위치로 이동
            {
                gGameInstance->Push(make_shared<Job>(gGameInstance, &GameInstance::MoveMyPlayersToOther, session));
                this_thread::sleep_for(500ms);
            }
            
            uint64 end = ::GetTickCount64();
            if (end - start > 1000 * 10)
                break;
        }

        // 방에서 퇴장
        gGameInstance->Push(make_shared<Job>(gGameInstance, &GameInstance::ExitGameRoom, session));
        this_thread::sleep_for(10ms);

        // 접속 종료
        session->Disconnect();
        this_thread::sleep_for(1s);
    }
}