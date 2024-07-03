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

void Session::processEvent(IocpEvent* iocpEvent, uint32 numOfBytes)
{
	switch (iocpEvent->getEventType())
	{
	case EventType::Connect:
		processConnect();
		break;
	case EventType::Disconnect:
		processDisconnect();
		break;
	case EventType::Recv:
		processRecv(numOfBytes);
		break;
	case EventType::Send:
		processSend(numOfBytes);
		break;
	default:
		spdlog::error("Session[{}] : Invalid EventType[{}]", _sessionId, static_cast<uint8>(iocpEvent->getEventType()));
		break;
	}
}

BYTE* Session::buffer()
{
	return _recvBuffer->buffer();
}

bool Session::connect()
{
	if (_bConnected.exchange(true) == true)
		return false;

	if (registerConnect() == false)
	{
		_bConnected.store(false);
		return false;
	}

	return true;
}

void Session::disconnect()
{
	if (_bConnected.exchange(false) == false)
		return;

	registerDisconnect();
}

void Session::send(shared_ptr<SendBuffer> sendBuffer)
{
	if (isConnected() == false)
		return;

	if (sendBuffer == nullptr)
		return;

	{
		lock_guard<mutex> lock(_mutex);

		_sendQueue.push(sendBuffer);
	}

	if (_bSendRegistered.exchange(true) == false)
		registerSend();
}

bool Session::processAccept(const NetAddress& netAddress)
{
	if (gIocpCore->registerObject(shared_from_this()) == false)
		return false;

	_bConnected.store(true);
	setNetAddress(netAddress);
	getService()->AddSession(dynamic_pointer_cast<Session>(shared_from_this()));
	registerRecv();

	return true;
}

bool Session::registerConnect()
{
	if (gIocpCore->registerObject(shared_from_this()) == false ||
		SocketUtils::setReuseAddress(_socket, true) == false ||
		SocketUtils::bindAnyAddress(_socket, 0) == false)
	{
		spdlog::error("Session[{}] : Connect initialize Fail", _sessionId);
		return false;
	}

	_connectEvent = new IocpEvent(EventType::Connect);
	_connectEvent->setOwner(shared_from_this());

	setNetAddress(getService()->GetNetAddress());
	SOCKADDR_IN sockAddr = _netAddress.getSockAddr();
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

void Session::processConnect()
{
	delete _connectEvent;

	getService()->AddSession(dynamic_pointer_cast<Session>(shared_from_this()));
	registerRecv();
	onConnect();

	spdlog::info("Session[{}] : Connect Success", _sessionId);
}

void Session::registerDisconnect()
{
	_disconnectEvent = new IocpEvent(EventType::Disconnect);
	_disconnectEvent->setOwner(shared_from_this());

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

void Session::processDisconnect()
{
	delete _disconnectEvent;

	getService()->RemoveSession(dynamic_pointer_cast<Session>(shared_from_this()));
	onDisconnect();

	spdlog::info("Session[{}] : Disconnect Success", _sessionId);
}

void Session::registerRecv()
{
	_recvEvent->init();
	_recvEvent->setOwner(shared_from_this());

	WSABUF wsaBuf;
	wsaBuf.buf = reinterpret_cast<char*>(_recvBuffer->buffer());
	wsaBuf.len = _recvBuffer->bufferSize();

	DWORD numOfBytes = 0;
	DWORD flags = 0;

	if (::WSARecv(_socket, &wsaBuf, 1, &numOfBytes, &flags, _recvEvent, nullptr) == SOCKET_ERROR)
	{
		int32 errorCode = ::WSAGetLastError();
		if (errorCode != WSA_IO_PENDING)
		{
			_recvEvent->setOwner(nullptr);

			spdlog::error("Session[{}] : Recv Error[{}]", _sessionId, errorCode);
		}
	}
}

void Session::processRecv(uint32 numOfBytes)
{
	_recvEvent->setOwner(nullptr);

	if (numOfBytes == 0 || _recvBuffer->onWrite(numOfBytes) == false)
	{
		disconnect();
		return;
	}

	uint32 processedSize = processRecvBuffer(numOfBytes);
	uint32 dataSize = _recvBuffer->dataSize();
	if (processedSize > dataSize || _recvBuffer->onRead(numOfBytes) == false)
	{
		disconnect();
		return;
	}

	_recvBuffer->cleanCursor();
	registerRecv();
}

uint32 Session::processRecvBuffer(uint32 numOfBytes)
{
	BYTE* buffer = _recvBuffer->data();
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
		onRecv(buffer);

		buffer += header->packetSize;
		processedSize += header->packetSize;
	}

	return processedSize;
}

void Session::registerSend()
{
	_sendEvent->init();
	_sendEvent->setOwner(shared_from_this());

	vector<WSABUF> wsaBufs;
	{
		lock_guard<mutex> lock(_mutex);

		while (_sendQueue.empty() == false)
		{
			shared_ptr<SendBuffer> sendBuffer = _sendQueue.front();
			_sendQueue.pop();

			WSABUF wsaBuf;
			wsaBuf.buf = reinterpret_cast<char*>(sendBuffer->buffer());
			wsaBuf.len = sendBuffer->bufferSize();
			wsaBufs.push_back(wsaBuf);

			_sendEvent->addSendBuffer(sendBuffer);
		}
	}

	DWORD numOfBytes = 0;

	if (::WSASend(_socket, wsaBufs.data(), static_cast<DWORD>(wsaBufs.size()), &numOfBytes, 0, _sendEvent, nullptr) == SOCKET_ERROR)
	{
		int32 errorCode = ::WSAGetLastError();
		if (errorCode != WSA_IO_PENDING)
		{
			_sendEvent->setOwner(nullptr);
			_sendEvent->clearSendBuffers();
			_bSendRegistered.store(false);

			spdlog::error("Session[{}] : Send Error[{}]", _sessionId, errorCode);
		}
	}
}

void Session::processSend(uint32 numOfBytes)
{
	onSend(numOfBytes);

	_sendEvent->setOwner(nullptr);
	_sendEvent->clearSendBuffers();

	bool bSendRegister = false;
	{
		lock_guard<mutex> lock(_mutex);

		if (_sendQueue.empty())
			_bSendRegistered.store(false);
		else
			bSendRegister = true;
	}
	if (bSendRegister)
		registerSend();
}
