#include "pch.h"
#include "ClientPacketHandler.h"
#include "ClientService.h"
#include "PacketSession.h"
#include "ThreadManager.h"
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

    const uint32 sessionCount = 50;

    ClientPacketHandler::Init();
    shared_ptr<ClientService> service = make_shared<ClientService>(NetAddress(L"127.0.0.1", 7777), []() { return make_shared<PacketSession>(); }, sessionCount);
    assert(service->Start());

    for (uint32 i = 0; i < 5; i++)
    {
        gThreadManager->Launch([&service]()
            {
                while (true)
                {
                    service->GetIocpCore().Dispatch(10);
                    ThreadManager::ExecuteJobQueue();
                    ThreadManager::DistributeReservedJob();
                }
            });
    }

    gThreadManager->Launch([&service, sessionCount]()
        {
            while (true)
            {
                this_thread::sleep_for(5s);

                // Enter
                for (uint32 i = 1; i <= sessionCount; i++)
                {
                    shared_ptr<Session> session = service->GetSession(i);
                    if (session == nullptr)
                        continue;
                    
                    gGameInstance->Push(make_shared<Job>(gGameInstance, &GameInstance::Enter, session));
                }

                this_thread::sleep_for(3s);

                // Move - RUN
                for (uint32 i = 1; i <= sessionCount; i++)
                {
                    shared_ptr<Session> session = service->GetSession(i);
                    if (session == nullptr)
                        continue;

                    gGameInstance->Push(make_shared<Job>(gGameInstance, &GameInstance::Move, session));
                }

                this_thread::sleep_for(10s);

                // Move - IDLE
                for (uint32 i = 1; i <= sessionCount; i++)
                {
                    shared_ptr<Session> session = service->GetSession(i);
                    if (session == nullptr)
                        continue;

                    gGameInstance->Push(make_shared<Job>(gGameInstance, &GameInstance::Move, session));
                }

                this_thread::sleep_for(3s);
                
                // Exit
                for (uint32 i = 1; i <= sessionCount; i++)
                {
                    shared_ptr<Session> session = service->GetSession(i);
                    if (session == nullptr)
                        continue;

                    gGameInstance->Push(make_shared<Job>(gGameInstance, &GameInstance::Exit, session));
                }
            }
        });

    gThreadManager->Join();
}