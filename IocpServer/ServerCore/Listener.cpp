#include "pch.h"
#include "Listener.h"
#include "IocpEvent.h"
#include "Service.h"
#include "IocpCore.h"
#include "Session.h"

Listener::Listener(uint32 acceptCount) :
    _acceptCount(acceptCount)
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

bool Listener::StartAccept()
{
    // Iocp�� ��� �� Socket �ʱ�ȭ
    if (GetService()->GetIocpCore().Register(shared_from_this()) == false ||
        SocketUtils::SetLinger(_socket, 0, 0) == false ||
        SocketUtils::SetReuseAddress(_socket, true) == false ||
        SocketUtils::Bind(_socket, GetService()->GetNetAddress().GetSockAddr()) == false ||
        SocketUtils::Listen(_socket) == false)
    {
        return false;
    }

    for (uint32 i = 0; i < _acceptCount; i++)
    {
        IocpEvent* acceptEvent = new IocpEvent(EventType::Accept);
        _acceptEvents.push_back(acceptEvent);
        RegisterAccept(acceptEvent);
    }

    return true;
}

void Listener::RegisterAccept(IocpEvent* acceptEvent)
{
    // ������ Client�� ���� ���� ����
    shared_ptr<Session> session = GetService()->CreateSession();

    if (session == nullptr)
    {
        cout << "[Listener] Failed to create session" << endl;
        // TODO. Retry?
        return;
    }

    // IocpEvent �ʱ�ȭ
    acceptEvent->Init();
    acceptEvent->SetOwner(shared_from_this());
    acceptEvent->SetSession(session);

    // Accept
    DWORD numOfBytes = 0;

    if (SocketUtils::AcceptEx(_socket, session->GetSocket(), session->Buffer(), 0, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, &numOfBytes, acceptEvent) == false)
    {
        int32 errorCode = ::WSAGetLastError();

        if (errorCode != WSA_IO_PENDING)
        {
            cout << "[Listener] Accept Error : " << errorCode << endl;

            // use_count release
            acceptEvent->SetOwner(nullptr);
            acceptEvent->SetSession(nullptr);
            // TODO. Retry?
        }
    }
}

void Listener::ProcessAccept(IocpEvent* acceptEvent)
{
    // Client ���� �Ϸ�, Session ����
    shared_ptr<Session> session = acceptEvent->GetSession();

    // use_count release
    acceptEvent->SetOwner(nullptr);
    acceptEvent->SetSession(nullptr);

    if (SocketUtils::SetUpdateAcceptSocket(session->GetSocket(), _socket) == false)
    {
        cout << "[Listener] Failed to set accept socket" << endl;

        RegisterAccept(acceptEvent);
        return;
    }

    // Client �ּ� ��������
    SOCKADDR_IN sockAddr;
    int32 addrLen = sizeof(sockAddr);

    if (::getpeername(session->GetSocket(), reinterpret_cast<sockaddr*>(&sockAddr), &addrLen) == SOCKET_ERROR)
    {
        cout << "[Listener] Failed to get client's address" << endl;

        RegisterAccept(acceptEvent);
        return;
    }

    // Session ����
    if (session->OnAccept(NetAddress(sockAddr)))
    {
        wcout << format(L"[Listener] Client Connected - {}({})", session->GetNetAddress().GetIpAddress(), session->GetNetAddress().GetPort()) << endl;
    }

    // Event ����, �ٸ� Client ���� ���
    RegisterAccept(acceptEvent);
}
