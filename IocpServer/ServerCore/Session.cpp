#include "pch.h"
#include "Session.h"
#include "IocpCore.h"
#include "Service.h"
#include "PacketHandler.h"

Session::Session(uint32 sessionId) :
    _sessionId(sessionId),
    _recvBuffer(BUFFER_SIZE)
{
}

Session::~Session()
{
}

void Session::Dispatch(IocpEvent* iocpEvent, uint32 numOfBytes)
{
    switch (iocpEvent->GetEventType())
    {
    case EventType::Connect:
        ProcessConnect();
        break;
    case EventType::Disconnect:
        ProcessDisconnect();
        break;
    case EventType::Recv:
        ProcessRecv(numOfBytes);
        break;
    case EventType::Send:
        ProcessSend(numOfBytes);
        break;
    default:
        cout << "Invalid EventType" << endl;
        break;
    }
}

bool Session::Connect()
{
    if (_bConnected.exchange(true) == true)
        return false;

    return RegisterConnect();
}

bool Session::Disconnect()
{
    if (_bConnected.exchange(false) == false)
        return false;

    return RegisterDisconnect();
}

void Session::Send(shared_ptr<SendBuffer> sendBuffer)
{
    if (sendBuffer == nullptr)
        return;

    _sendQueue.Push(sendBuffer);

    // Send를 처리중인 Thread가 없으면 현재 Thread가 담당
    if (_bSendRegistered.exchange(true) == false)
    {
        vector<shared_ptr<SendBuffer>> sendBuffers;
        _sendQueue.PopAll(sendBuffers);

        RegisterSend(sendBuffers);
    }
}

bool Session::OnAccept(NetAddress netAddress)
{
    // 세션 Connect 처리
    if (GetService()->GetIocpCore().Register(shared_from_this()) == false)
        return false;

    _bConnected.store(true);
    SetNetAddress(netAddress);
    ProcessConnect();

    return true;
}

bool Session::RegisterConnect()
{
    if (GetService()->GetIocpCore().Register(shared_from_this()) == false ||
        SocketUtils::SetReuseAddress(_socket, true) == false ||
        SocketUtils::BindAnyAddress(_socket, 0) == false)
    {
        _bConnected.store(false);
        return false;
    }

    _connectEvent.Init();
    _connectEvent.SetOwner(shared_from_this());

    // Connect
    SOCKADDR_IN sockAddr = GetService()->GetNetAddress().GetSockAddr();
    int32 addrLen = sizeof(sockAddr);
    DWORD numOfBytes = 0;

    if (SocketUtils::ConnectEx(_socket, reinterpret_cast<const sockaddr*>(&sockAddr), addrLen, nullptr, 0, &numOfBytes, &_connectEvent) == false)
    {
        int32 errorCode = ::WSAGetLastError();

        if (errorCode != WSA_IO_PENDING)
        {
            cout << "Connect Error : " << errorCode << endl;

            _bConnected.store(false);
            _connectEvent.SetOwner(nullptr);
            return false;
        }
    }

    return true;
}

void Session::ProcessConnect()
{
    _connectEvent.SetOwner(nullptr);

    GetService()->AddSession(dynamic_pointer_cast<Session>(shared_from_this()));
    OnConnect();

    // Recv 시작
    RegisterRecv();
}

bool Session::RegisterDisconnect()
{
    _disconnectEvent.Init();
    _disconnectEvent.SetOwner(shared_from_this());

    // Disconnect
    if (SocketUtils::DisconnectEx(_socket, &_disconnectEvent, TF_REUSE_SOCKET, 0) == false)
    {
        int32 errorCode = ::WSAGetLastError();

        if (errorCode != WSA_IO_PENDING)
        {
            cout << "Disconnect Error : " << errorCode << endl;

            _bConnected.store(true);
            _disconnectEvent.SetOwner(nullptr);
            return false;
        }
    }

    return true;
}

void Session::ProcessDisconnect()
{
    _disconnectEvent.SetOwner(nullptr);

    OnDisconnect();
    GetService()->RemoveSession(dynamic_pointer_cast<Session>(shared_from_this()));

    wcout << format(L"Client Disconnect - {}({})", GetNetAddress().GetIpAddress(), GetNetAddress().GetPort()) << endl;;
}

void Session::RegisterRecv()
{
    _recvEvent.Init();
    _recvEvent.SetOwner(shared_from_this());

    // 수신 Buffer 할당
    WSABUF wsaBuf;
    wsaBuf.buf = reinterpret_cast<char*>(_recvBuffer.WritePos());
    wsaBuf.len = _recvBuffer.FreeSize();

    // Recv
    DWORD numOfBytes = 0;
    DWORD flags = 0;

    if (::WSARecv(_socket, &wsaBuf, 1, &numOfBytes, &flags, &_recvEvent, nullptr) == SOCKET_ERROR)
    {
        int32 errorCode = ::WSAGetLastError();

        if (errorCode != WSA_IO_PENDING)
        {
            cout << "Recv Error : " << errorCode << endl;

            _recvEvent.SetOwner(nullptr);
        }
    }
}

void Session::ProcessRecv(uint32 numOfBytes)
{
    _recvEvent.SetOwner(nullptr);

    // 수신한 데이터가 0이거나 Buffer Overflow인 경우 disconnect
    if (numOfBytes == 0 || _recvBuffer.OnWrite(numOfBytes) == false)
    {
        Disconnect();
        return;
    }

    // 수신한 데이터를 Packet 단위로 처리 후 처리한 데이터 크기를 계산
    uint32 processedSize = ProcessPacket(numOfBytes);

    // 수신한 전체 데이터 크기
    uint32 dataSize = _recvBuffer.DataSize();

    // Overflow 예외 처리
    if (processedSize > dataSize || _recvBuffer.OnRead(processedSize) == false)
    {
        cout << "Recv Overflow" << endl;

        Disconnect();
        return;
    }

    // 수신버퍼 커서 정리
    _recvBuffer.Clean();

    // 다시 데이터 수신 시작
    RegisterRecv();
}

uint32 Session::ProcessPacket(uint32 numOfBytes)
{
    uint32 processedSize = 0;
    BYTE* buffer = _recvBuffer.ReadPos();

    while (true)
    {
        // 모든 데이터 처리 시 종료
        if (processedSize >= numOfBytes)
            break;

        // 남은 데이터가 헤더보다 작은 경우 parsing 불가, 종료
        const uint32 dataSize = numOfBytes - processedSize;
        if (dataSize < sizeof(PacketHeader))
            break;

        // 헤더 parsing, packet 크기를 구함
        PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);
        const uint32 packetSize = header->packetSize;

        // 남은 데이터가 packet 크기보다 작은 경우 parsing 불가, 종료
        if (dataSize < packetSize)
            break;

        // packet 처리
        OnRecv(buffer);

        // 처리한 packet 크기 계산, buffer offset 이동
        processedSize += packetSize;
        buffer += packetSize;
    }

    return processedSize;
}

void Session::RegisterSend(vector<shared_ptr<SendBuffer>>& sendBuffers)
{
    if (sendBuffers.size() == 0)
    {
        _bSendRegistered.store(false);
        return;
    }

    _sendEvent.Init();
    _sendEvent.SetOwner(shared_from_this());

    // 보낼 데이터들을 WSABUF에 할당
    vector<WSABUF> wsaBufs;
    
    for (shared_ptr<SendBuffer>& sendBuffer : sendBuffers)
    {
        WSABUF wsaBuf;
        wsaBuf.buf = reinterpret_cast<char*>(sendBuffer->Buffer());
        wsaBuf.len = sendBuffer->GetBufferSize();
        wsaBufs.push_back(wsaBuf);

        // SendBuffer가 전송 완료 전에 소멸되지 않도록 Event에 추가 (use_count++)
        _sendEvent.PushSendBuffer(sendBuffer);
    }

    // Send
    DWORD numOfBytes = 0;

    if (::WSASend(_socket, wsaBufs.data(), static_cast<DWORD>(wsaBufs.size()), &numOfBytes, 0, &_sendEvent, nullptr) == SOCKET_ERROR)
    {
        int32 errorCode = ::WSAGetLastError();

        if (errorCode != WSA_IO_PENDING)
        {
            cout << "Send Error : " << errorCode << endl;

            _sendEvent.SetOwner(nullptr);
            _sendEvent.ClearSendBuffers();
            _bSendRegistered.store(false);
        }
    }
}

void Session::ProcessSend(uint32 numOfBytes)
{
    _sendEvent.SetOwner(nullptr);

    OnSend(numOfBytes);

    _sendEvent.ClearSendBuffers();

    // 보낼 데이터가 남아있으면 추가로 전송
    vector<shared_ptr<SendBuffer>> sendBuffers;
    _sendQueue.PopAll(sendBuffers);

    RegisterSend(sendBuffers);
}