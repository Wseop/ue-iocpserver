#pragma once

enum class EventType : uint8
{
	Connect,
	Disconnect,
	Accept,
	Recv,
	Send
};

class IocpObject;
class Session;
class SendBuffer;

class IocpEvent : public OVERLAPPED
{
public:
	IocpEvent(EventType eventType);
	~IocpEvent();

	void Init();

public:
	EventType GetEventType() const { return _eventType; }

	shared_ptr<IocpObject> GetOwner() const { return _owner; }
	void SetOwner(shared_ptr<IocpObject> owner) { _owner = owner; }

	shared_ptr<Session> GetSession() const { return _session; }
	void SetSession(shared_ptr<Session> session) { _session = session; }

	void PushSendBuffer(shared_ptr<SendBuffer> sendBuffer);
	void ClearSendBuffers();

private:
	EventType _eventType;
	shared_ptr<IocpObject> _owner = nullptr;

	// Accept
	shared_ptr<Session> _session = nullptr;

	// Send
	vector<shared_ptr<SendBuffer>> _sendBuffers;
};

