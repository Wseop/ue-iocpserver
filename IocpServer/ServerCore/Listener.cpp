#include "pch.h"
#include "Listener.h"
#include "IocpEvent.h"
#include "Service.h"
#include "IocpCore.h"
#include "Session.h"
#include "Utils.h"
#include "JobQueue.h"
#include "Job.h"

Listener::Listener() :
    _jobQueue(make_shared<JobQueue>())
{
}

Listener::~Listener()
{
}

void Listener::Dispatch(IocpEvent* iocpEvent, uint32 numOfBytes)
{
    assert(iocpEvent->GetEventType() == EventType::Accept);
    ProcessAccept(iocpEvent);
}

bool Listener::Start(uint32 acceptCount)
{
    if (GetService()->GetIocpCore()->Register(shared_from_this()) == false ||
        SocketUtils::SetLinger(_socket, 0, 0) == false ||
        SocketUtils::SetReuseAddress(_socket, true) == false ||
        SocketUtils::Bind(_socket, GetService()->GetNetAddress().GetSockAddr()) == false ||
        SocketUtils::Listen(_socket) == false)
    {
        spdlog::error("Listener : Initialize Fail");
        return false;
    }

    for (uint32 i = 0; i < acceptCount; i++)
    {
        IocpEvent* acceptEvent = new IocpEvent(EventType::Accept);
        _acceptEvents.push_back(acceptEvent);
        RegisterAccept(acceptEvent);
    }

    return true;
}

void Listener::RegisterAccept(IocpEvent* acceptEvent)
{
    acceptEvent->Init();
    acceptEvent->SetOwner(shared_from_this());

    shared_ptr<Session> session = GetService()->CreateSession();
    acceptEvent->SetSession(session);

    DWORD numOfBytes = 0;

    if (SocketUtils::AcceptEx(_socket, session->GetSocket(), session->Buffer(), 0, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, &numOfBytes, acceptEvent) == false)
    {
        int32 errorCode = ::WSAGetLastError();
        if (errorCode != WSA_IO_PENDING)
        {
            acceptEvent->SetOwner(nullptr);
            acceptEvent->SetSession(nullptr);

            // RegisterAccept 다시 걸어줌. JobQueue에 PushOnly로 등록하여 재귀호출 방지
            _jobQueue->Push(make_shared<Job>(dynamic_pointer_cast<Listener>(shared_from_this()), &Listener::RegisterAccept, acceptEvent), true);
            spdlog::error("Listener : Accept Fail");
        }
    }
}

void Listener::ProcessAccept(IocpEvent* acceptEvent)
{
    shared_ptr<Session> session = acceptEvent->GetSession();
    acceptEvent->SetSession(nullptr);
    acceptEvent->SetOwner(nullptr);

    if (SocketUtils::SetUpdateAcceptSocket(session->GetSocket(), _socket) == false)
    {
        spdlog::error("Listener : Socket Update Fail");
        RegisterAccept(acceptEvent);
        return;
    }

    SOCKADDR_IN clientAddr;
    ::memset(&clientAddr, 0, sizeof(SOCKADDR_IN));
    int32 addrLen = sizeof(clientAddr);

    if (::getpeername(session->GetSocket(), reinterpret_cast<sockaddr*>(&clientAddr), &addrLen) == SOCKET_ERROR)
    {
        spdlog::error("Listener : Get Client Address Fail");
        RegisterAccept(acceptEvent);
        return;
    }

    NetAddress netAddress(clientAddr);
    if (session->ProcessAccept(netAddress))
        spdlog::info("Listener : Client Connected[{}({})] : SessionId[{}]", Utils::WStrToStr(netAddress.GetIpAddress()), netAddress.GetPort(), session->GetSessionId());

    RegisterAccept(acceptEvent);
}
