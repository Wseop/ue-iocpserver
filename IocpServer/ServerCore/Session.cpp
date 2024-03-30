﻿#include "pch.h"
#include "Session.h"
#include "IocpCore.h"
#include "Service.h"
#include "PacketHandler.h"
#include "Utils.h"

Session::Session() :
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
        spdlog::warn("Session[{}] : Invalid EventType", _sessionId);
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
    if (_bConnected == false)
        return;

    if (sendBuffer == nullptr)
        return;

    bool bSendRegister = false;
    {
        lock_guard<mutex> lock(_mutex);

        _sendQueue.push(sendBuffer);
        if (_bSendRegistered.exchange(true) == false)
            bSendRegister = true;
    }
    if (bSendRegister)
        RegisterSend();
}

void Session::OnAccept(NetAddress netAddress)
{
    // 세션 Connect 처리
    if (GetService()->GetIocpCore().Register(shared_from_this()) == false)
        return;

    _bConnected.store(true);
    SetNetAddress(netAddress);
    ProcessConnect();
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
            spdlog::error("Session[{}] : Connect Error[{}]", _sessionId, errorCode);

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

    if (GetService()->AddSession(dynamic_pointer_cast<Session>(shared_from_this())) == false)
    {
        Disconnect();
        return;
    }

    OnConnect();
    RegisterRecv();

    string ip = Utils::WStrToStr(_netAddress.GetIpAddress());
    spdlog::info("Session[{}] : Connected[{}({})]", _sessionId, ip, _netAddress.GetPort());
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
            spdlog::error("Session[{}] : Disconnect Error[{}]", _sessionId, errorCode);

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
    GetService()->RemoveSession(_sessionId);

    spdlog::info("Session[{}] : Disconnected", _sessionId);
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
            if (errorCode != WSAENOTCONN)
                spdlog::error("Session[{}] : Recv Error[{}]", _sessionId, errorCode);
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
        spdlog::error("Session[{}] : Recv Overflow", _sessionId);

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

void Session::RegisterSend()
{
    _sendEvent.Init();
    _sendEvent.SetOwner(shared_from_this());

    // 보낼 데이터들을 WSABUF에 할당
    vector<WSABUF> wsaBufs;
    {
        lock_guard<mutex> lock(_mutex);

        while (_sendQueue.empty() == false)
        {
            shared_ptr<SendBuffer> sendBuffer = _sendQueue.front();
            _sendQueue.pop();

            WSABUF wsaBuf;
            wsaBuf.buf = reinterpret_cast<char*>(sendBuffer->Buffer());
            wsaBuf.len = sendBuffer->GetBufferSize();
            wsaBufs.push_back(wsaBuf);

            _sendEvent.PushSendBuffer(sendBuffer);
        }
    }
    
    DWORD numOfBytes = 0;
    if (::WSASend(_socket, wsaBufs.data(), static_cast<DWORD>(wsaBufs.size()), &numOfBytes, 0, &_sendEvent, nullptr) == SOCKET_ERROR)
    {
        int32 errorCode = ::WSAGetLastError();
        if (errorCode != WSA_IO_PENDING)
        {
            _sendEvent.SetOwner(nullptr);
            _sendEvent.ClearSendBuffers();
            _bSendRegistered.store(false);

            if (errorCode != WSAENOTCONN)
                spdlog::error("Session[{}] : Send Error[{}]", _sessionId, errorCode);
        }
    }
}

void Session::ProcessSend(uint32 numOfBytes)
{
    OnSend(numOfBytes);

    _sendEvent.SetOwner(nullptr);
    _sendEvent.ClearSendBuffers();

    // 보낼 데이터가 남아있으면 추가로 전송
    bool bSendRegister = false;
    {
        lock_guard<mutex> lock(_mutex);

        if (_sendQueue.empty())
            _bSendRegistered.store(false);
        else
            bSendRegister = true;
    }
    if (bSendRegister)
        RegisterSend();
}