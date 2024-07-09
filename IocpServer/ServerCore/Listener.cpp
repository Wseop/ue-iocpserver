#include "pch.h"
#include "Listener.h"
#include "IocpEvent.h"
#include "NetworkService.h"
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

void Listener::processEvent(IocpEvent* iocpEvent, uint32 numOfBytes)
{
    assert(iocpEvent->getEventType() == EventType::Accept);
    processAccept(iocpEvent);
}

bool Listener::start(uint32 acceptCount)
{
    if (gIocpCore->registerObject(shared_from_this()) == false ||
        SocketUtils::setLinger(_socket, 0, 0) == false ||
        SocketUtils::setReuseAddress(_socket, true) == false ||
        SocketUtils::bind(_socket, getService()->GetNetAddress().getSockAddr()) == false ||
        SocketUtils::listen(_socket) == false)
    {
        spdlog::error("[Listener] Initialize Fail");
        return false;
    }

    for (uint32 i = 0; i < acceptCount; i++)
    {
        IocpEvent* acceptEvent = new IocpEvent(EventType::Accept);
        _acceptEvents.push_back(acceptEvent);
        registerAccept(acceptEvent);
    }

    spdlog::info("[Listener] Start");

    return true;
}

void Listener::registerAccept(IocpEvent* acceptEvent)
{
    acceptEvent->init();
    acceptEvent->setOwner(shared_from_this());

    shared_ptr<Session> session = getService()->CreateSession();
    acceptEvent->setSession(session);

    DWORD numOfBytes = 0;

    if (SocketUtils::AcceptEx(_socket, session->getSocket(), session->buffer(), 0, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, &numOfBytes, acceptEvent) == false)
    {
        int32 errorCode = ::WSAGetLastError();
        if (errorCode != WSA_IO_PENDING)
        {
            acceptEvent->setOwner(nullptr);
            acceptEvent->setSession(nullptr);

            // RegisterAccept 다시 걸어줌. JobQueue에 PushOnly로 등록하여 재귀호출 방지
            _jobQueue->push(make_shared<Job>(dynamic_pointer_cast<Listener>(shared_from_this()), &Listener::registerAccept, acceptEvent));
            spdlog::error("[Listener] Accept Fail");
        }
    }
}

void Listener::processAccept(IocpEvent* acceptEvent)
{
    shared_ptr<Session> session = acceptEvent->getSession();
    acceptEvent->setSession(nullptr);
    acceptEvent->setOwner(nullptr);

    if (SocketUtils::setUpdateAcceptSocket(session->getSocket(), _socket) == false)
    {
        spdlog::error("[Listener] Socket Update Fail");
        registerAccept(acceptEvent);
        return;
    }

    SOCKADDR_IN clientAddr;
    ::memset(&clientAddr, 0, sizeof(SOCKADDR_IN));
    int32 addrLen = sizeof(clientAddr);

    if (::getpeername(session->getSocket(), reinterpret_cast<sockaddr*>(&clientAddr), &addrLen) == SOCKET_ERROR)
    {
        spdlog::error("[Listener] Get Client Address Fail");
        registerAccept(acceptEvent);
        return;
    }

    NetAddress netAddress(clientAddr);
    if (session->processAccept(netAddress))
        spdlog::info("[Listener] Client Connected[{}({})] : SessionId[{}]", netAddress.getIpAddress(), netAddress.getPort(), session->getSessionId());

    registerAccept(acceptEvent);
}
