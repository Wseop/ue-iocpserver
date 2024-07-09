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
	virtual void processEvent(IocpEvent* iocpEvent, uint32 numOfBytes) override;

protected:
	virtual void onConnect() abstract;
	virtual void onDisconnect() abstract;
	virtual uint32 onRecv(BYTE* data, uint32 numOfBytes) abstract;
	virtual void onSend(uint32 numOfBytes) abstract;

public:
	uint32 getSessionId() const { return _sessionId; }
	void setSessionId(uint32 sessionId) { _sessionId = sessionId; }

	bool isConnected() const { return _bConnected; }

	NetAddress getNetAddress() const { return _netAddress; }
	void setNetAddress(const NetAddress& netAddress) { _netAddress = netAddress; }

	BYTE* buffer();

public:
	bool connect();
	void disconnect();
	void send(shared_ptr<SendBuffer> sendBuffer);

	bool processAccept(const NetAddress& netAddress);

private:
	bool registerConnect();
	void processConnect();

	void registerDisconnect();
	void processDisconnect();

	void registerRecv();
	void processRecv(uint32 numOfBytes);

	void registerSend();
	void processSend(uint32 numOfBytes);

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

class PacketSession : public Session
{
public:
	PacketSession();
	virtual ~PacketSession();

protected:
	virtual void onConnect() override;
	virtual void onDisconnect() override;
	virtual uint32 onRecv(BYTE* data, uint32 numOfBytes) override final;
	virtual void onSend(uint32 numOfBytes) override;
};