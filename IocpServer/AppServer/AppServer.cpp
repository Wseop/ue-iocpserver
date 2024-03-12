#include "pch.h"
#include "ServerPacketHandler.h"
#include "ThreadManager.h"
#include "ServerService.h"
#include "PacketSession.h"
#include "IocpCore.h"

int main()
{
    ServerPacketHandler::Init();

    shared_ptr<ServerService> service = make_shared<ServerService>(
        NetAddress(L"127.0.0.1", 7777), 
        [](){ return make_shared<PacketSession>(); }, 
        5);
    assert(service->Start());

    // Worker 실행 - Dispatch & ExecuteJobQueue
    for (uint32 i = 0; i < 10; i++)
    {
        gThreadManager->Launch([&service]()
            {
                while (true)
                {
                    service->GetIocpCore().Dispatch(10);
                    ThreadManager::ExecuteJobQueue();
                }
            });
    }

    gThreadManager->Join();
}
