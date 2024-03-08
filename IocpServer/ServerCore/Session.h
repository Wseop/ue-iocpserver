#pragma once

#include "IocpObject.h"
#include "IocpEvent.h"
#include "RecvBuffer.h"

class Session : public IocpObject
{
	const uint32 BUFFER_SIZE = 0x1000;

public:
	Session();
	virtual ~Session();

	virtual void Dispatch(IocpEvent* iocpEvent, uint32 numOfBytes) override;

protected:
	virtual void OnConnect() abstract;
	virtual void OnDisconnect() abstract;
	virtual void OnRecv(BYTE* packet) abstract;
	virtual void OnSend(uint32 numOfBytes) abstract;

public:
	bool IsConnected() { return _bConnected; }

	NetAddress GetNetAddress() { return _netAddress; }
	void SetNetAddress(NetAddress netAddress) { _netAddress = netAddress; }

	BYTE* Buffer() { return _recvBuffer.WritePos(); }
	uint32 BufferSize() { return _recvBuffer.FreeSize(); }

public:
	bool Connect();
	bool Disconnect();
	void Send(shared_ptr<SendBuffer> sendBuffer);

	bool OnAccept(NetAddress netAddress);

private:
	// Connect
	bool RegisterConnect();
	void ProcessConnect();

	// Disconnect
	bool RegisterDisconnect();
	void ProcessDisconnect();

	// Recv
	void RegisterRecv();
	void ProcessRecv(uint32 numOfBytes);
	uint32 ProcessPacket(uint32 numOfBytes);

	// Send
	void RegisterSend(vector<shared_ptr<SendBuffer>>& sendBuffers);
	void ProcessSend(uint32 numOfBytes);

private:
	atomic<bool> _bConnected = false;
	NetAddress _netAddress;
	RecvBuffer _recvBuffer;

	// IocpEvent
	IocpEvent _connectEvent{ EventType::Connect };
	IocpEvent _disconnectEvent{ EventType::Disconnect };
	IocpEvent _recvEvent{ EventType::Recv };
	IocpEvent _sendEvent{ EventType::Send };

	// Send
	atomic<bool> _bSendRegistered = false;
	LockQueue<shared_ptr<SendBuffer>> _sendQueue;
};

