#include "pch.h"
#include "Session.h"
#include "IocpEvent.h"
#include "Service.h"
#include "IocpCore.h"
#include "SendBuffer.h"
#include "RecvBuffer.h"
#include "PacketHandler.h"

Session::Session() :
	_recvEvent(new IocpEvent(EventType::Recv)),
	_sendEvent(new IocpEvent(EventType::Send)),
	_recvBuffer(make_shared<RecvBuffer>(BUFFER_SIZE))
{
}

Session::~Session()
{
	delete _recvEvent;
	delete _sendEvent;
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
		spdlog::error("Session[{}] : Invalid EventType[{}]", _sessionId, static_cast<uint8>(iocpEvent->GetEventType()));
		break;
	}
}

BYTE* Session::Buffer()
{
	return _recvBuffer->WritePos();
}

bool Session::Connect()
{
	if (_bConnected.exchange(true) == true)
		return false;

	if (RegisterConnect() == false)
	{
		_bConnected.store(false);
		return false;
	}

	return true;
}

void Session::Disconnect()
{
	if (_bConnected.exchange(false) == false)
		return;

	RegisterDisconnect();
}

void Session::Send(shared_ptr<SendBuffer> sendBuffer)
{
	if (IsConnected() == false)
		return;

	if (sendBuffer == nullptr)
		return;

	{
		lock_guard<mutex> lock(_mutex);

		_sendQueue.push(sendBuffer);
	}

	if (_bSendRegistered.exchange(true) == false)
		RegisterSend();
}

bool Session::ProcessAccept(const NetAddress& netAddress)
{
	if (GetService()->GetIocpCore()->Register(shared_from_this()) == false)
		return false;

	_bConnected.store(true);
	SetNetAddress(netAddress);
	GetService()->AddSession(dynamic_pointer_cast<Session>(shared_from_this()));
	RegisterRecv();

	return true;
}

bool Session::RegisterConnect()
{
	if (GetService()->GetIocpCore()->Register(shared_from_this()) == false ||
		SocketUtils::SetReuseAddress(_socket, true) == false ||
		SocketUtils::BindAnyAddress(_socket, 0) == false)
	{
		spdlog::error("Session[{}] : Connect initialize Fail", _sessionId);
		return false;
	}

	_connectEvent = new IocpEvent(EventType::Connect);
	_connectEvent->SetOwner(shared_from_this());

	SetNetAddress(GetService()->GetNetAddress());
	SOCKADDR_IN sockAddr = _netAddress.GetSockAddr();
	int32 addrLen = sizeof(sockAddr);
	DWORD numOfBytes = 0;

	if (SocketUtils::ConnectEx(_socket, reinterpret_cast<const sockaddr*>(&sockAddr), addrLen, nullptr, 0, &numOfBytes, _connectEvent) == false)
	{
		int32 errorCode = ::WSAGetLastError();
		if (errorCode != WSA_IO_PENDING)
		{
			spdlog::error("Session[{}] : Connect Fail", _sessionId);

			delete _connectEvent;
			return false;
		}
	}

	return true;
}

void Session::ProcessConnect()
{
	delete _connectEvent;

	GetService()->AddSession(dynamic_pointer_cast<Session>(shared_from_this()));
	RegisterRecv();
	OnConnect();

	spdlog::info("Session[{}] : Connect Success", _sessionId);
}

void Session::RegisterDisconnect()
{
	_disconnectEvent = new IocpEvent(EventType::Disconnect);
	_disconnectEvent->SetOwner(shared_from_this());

	if (SocketUtils::DisconnectEx(_socket, _disconnectEvent, TF_REUSE_SOCKET, 0) == false)
	{
		int32 errorCode = ::WSAGetLastError();
		if (errorCode != WSA_IO_PENDING)
		{
			spdlog::error("Session[{}] : Disconnect Fail", _sessionId);

			delete _disconnectEvent;
		}
	}
}

void Session::ProcessDisconnect()
{
	delete _disconnectEvent;

	GetService()->RemoveSession(dynamic_pointer_cast<Session>(shared_from_this()));
	OnDisconnect();

	spdlog::info("Session[{}] : Disconnect Success", _sessionId);
}

void Session::RegisterRecv()
{
	_recvEvent->Init();
	_recvEvent->SetOwner(shared_from_this());

	WSABUF wsaBuf;
	wsaBuf.buf = reinterpret_cast<char*>(_recvBuffer->WritePos());
	wsaBuf.len = _recvBuffer->FreeSize();

	DWORD numOfBytes = 0;
	DWORD flags = 0;

	if (::WSARecv(_socket, &wsaBuf, 1, &numOfBytes, &flags, _recvEvent, nullptr) == SOCKET_ERROR)
	{
		int32 errorCode = ::WSAGetLastError();
		if (errorCode != WSA_IO_PENDING)
		{
			_recvEvent->SetOwner(nullptr);

			spdlog::error("Session[{}] : Recv Error[{}]", _sessionId, errorCode);
		}
	}
}

void Session::ProcessRecv(uint32 numOfBytes)
{
	_recvEvent->SetOwner(nullptr);

	if (numOfBytes == 0 || _recvBuffer->OnWrite(numOfBytes) == false)
	{
		Disconnect();
		return;
	}

	uint32 processedSize = ProcessRecvBuffer(numOfBytes);
	uint32 dataSize = _recvBuffer->DataSize();
	if (processedSize > dataSize || _recvBuffer->OnRead(numOfBytes) == false)
	{
		Disconnect();
		return;
	}

	_recvBuffer->Clean();
	RegisterRecv();
}

uint32 Session::ProcessRecvBuffer(uint32 numOfBytes)
{
	BYTE* buffer = _recvBuffer->ReadPos();
	uint32 processedSize = 0;

	while (true)
	{
		if (numOfBytes <= processedSize)
			break;

		// 받은 데이터 크기 체크
		uint32 remainBytes = numOfBytes - processedSize;
		if (remainBytes < sizeof(PacketHeader))
			break;

		PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);
		if (remainBytes < header->packetSize)
			break;

		// Packet 처리
		OnRecv(buffer);

		buffer += header->packetSize;
		processedSize += header->packetSize;
	}

	return processedSize;
}

void Session::RegisterSend()
{
	_sendEvent->Init();
	_sendEvent->SetOwner(shared_from_this());

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

			_sendEvent->PushSendBuffer(sendBuffer);
		}
	}

	DWORD numOfBytes = 0;

	if (::WSASend(_socket, wsaBufs.data(), static_cast<DWORD>(wsaBufs.size()), &numOfBytes, 0, _sendEvent, nullptr) == SOCKET_ERROR)
	{
		int32 errorCode = ::WSAGetLastError();
		if (errorCode != WSA_IO_PENDING)
		{
			_sendEvent->SetOwner(nullptr);
			_sendEvent->ClearSendBuffers();
			_bSendRegistered.store(false);

			spdlog::error("Session[{}] : Send Error[{}]", _sessionId, errorCode);
		}
	}
}

void Session::ProcessSend(uint32 numOfBytes)
{
	OnSend(numOfBytes);

	_sendEvent->SetOwner(nullptr);
	_sendEvent->ClearSendBuffers();

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
