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

    // Send�� ó������ Thread�� ������ ���� Thread�� ���
    if (_bSendRegistered.exchange(true) == false)
    {
        vector<shared_ptr<SendBuffer>> sendBuffers;
        _sendQueue.PopAll(sendBuffers);

        RegisterSend(sendBuffers);
    }
}

bool Session::OnAccept(NetAddress netAddress)
{
    // ���� Connect ó��
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

    // Recv ����
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

    // ���� Buffer �Ҵ�
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

    // ������ �����Ͱ� 0�̰ų� Buffer Overflow�� ��� disconnect
    if (numOfBytes == 0 || _recvBuffer.OnWrite(numOfBytes) == false)
    {
        Disconnect();
        return;
    }

    // ������ �����͸� Packet ������ ó�� �� ó���� ������ ũ�⸦ ���
    uint32 processedSize = ProcessPacket(numOfBytes);

    // ������ ��ü ������ ũ��
    uint32 dataSize = _recvBuffer.DataSize();

    // Overflow ���� ó��
    if (processedSize > dataSize || _recvBuffer.OnRead(processedSize) == false)
    {
        cout << "Recv Overflow" << endl;

        Disconnect();
        return;
    }

    // ���Ź��� Ŀ�� ����
    _recvBuffer.Clean();

    // �ٽ� ������ ���� ����
    RegisterRecv();
}

uint32 Session::ProcessPacket(uint32 numOfBytes)
{
    uint32 processedSize = 0;
    BYTE* buffer = _recvBuffer.ReadPos();

    while (true)
    {
        // ��� ������ ó�� �� ����
        if (processedSize >= numOfBytes)
            break;

        // ���� �����Ͱ� ������� ���� ��� parsing �Ұ�, ����
        const uint32 dataSize = numOfBytes - processedSize;
        if (dataSize < sizeof(PacketHeader))
            break;

        // ��� parsing, packet ũ�⸦ ����
        PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);
        const uint32 packetSize = header->packetSize;

        // ���� �����Ͱ� packet ũ�⺸�� ���� ��� parsing �Ұ�, ����
        if (dataSize < packetSize)
            break;

        // packet ó��
        OnRecv(buffer);

        // ó���� packet ũ�� ���, buffer offset �̵�
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

    // ���� �����͵��� WSABUF�� �Ҵ�
    vector<WSABUF> wsaBufs;
    
    for (shared_ptr<SendBuffer>& sendBuffer : sendBuffers)
    {
        WSABUF wsaBuf;
        wsaBuf.buf = reinterpret_cast<char*>(sendBuffer->Buffer());
        wsaBuf.len = sendBuffer->GetBufferSize();
        wsaBufs.push_back(wsaBuf);

        // SendBuffer�� ���� �Ϸ� ���� �Ҹ���� �ʵ��� Event�� �߰� (use_count++)
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

    // ���� �����Ͱ� ���������� �߰��� ����
    vector<shared_ptr<SendBuffer>> sendBuffers;
    _sendQueue.PopAll(sendBuffers);

    RegisterSend(sendBuffers);
}