#pragma once

#include "IocpObject.h"

class IocpEvent;
class RecvBuffer;
class SendBuffer;

class Session : public IocpObject
{
public:
	Session();
	virtual ~Session();

public:
	virtual void Dispatch(IocpEvent* iocpEvent, uint32 numOfBytes) override;

protected:
	virtual void OnConnect() abstract;
	virtual void OnDisconnect() abstract;
	virtual void OnRecv(BYTE* buffer) abstract;
	virtual void OnSend(uint32 numOfBytes) abstract;

public:
	uint32 GetSessionId() const { return _sessionId; }
	void SetSessionId(uint32 sessionId) { _sessionId = sessionId; }

	bool IsConnected() const { return _bConnected; }

	NetAddress GetNetAddress() const { return _netAddress; }
	void SetNetAddress(const NetAddress& netAddress) { _netAddress = netAddress; }

	BYTE* Buffer();

public:
	bool Connect();
	void Disconnect();
	void Send(shared_ptr<SendBuffer> sendBuffer);

	bool ProcessAccept(const NetAddress& netAddress);

private:
	bool RegisterConnect();
	void ProcessConnect();

	void RegisterDisconnect();
	void ProcessDisconnect();

	void RegisterRecv();
	void ProcessRecv(uint32 numOfBytes);
	uint32 ProcessRecvBuffer(uint32 numOfBytes);

	void RegisterSend();
	void ProcessSend(uint32 numOfBytes);

private:
	uint32 _sessionId = 0;
	atomic<bool> _bConnected = false;
	NetAddress _netAddress = {};

	IocpEvent* _connectEvent = nullptr;
	IocpEvent* _disconnectEvent = nullptr;
	IocpEvent* _recvEvent = nullptr;
	IocpEvent* _sendEvent = nullptr;

	// Recv
	const uint32 BUFFER_SIZE = 0x1000;
	shared_ptr<RecvBuffer> _recvBuffer = nullptr;

	// Send
	mutex _mutex;
	atomic<bool> _bSendRegistered = false;
	queue<shared_ptr<SendBuffer>> _sendQueue;
};

