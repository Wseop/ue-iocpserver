#pragma once

enum class EventType : uint8
{
	None,
	Connect,
	Disconnect,
	Accept,
	Recv,
	Send,
};

class IocpObject;
class SendBuffer;
class Session;

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

	void PushSendBuffer(shared_ptr<SendBuffer> sendBuffer);
	void ClearSendBuffers();

	shared_ptr<Session> GetSession() const { return _session; }
	void SetSession(shared_ptr<Session> session) { _session = session; }

private:
	EventType _eventType;
	shared_ptr<IocpObject> _owner = nullptr;

	// Send 처리중 SendBuffer의 소멸 방지를 위해 보관하는 용도. (use_count++)
	vector<shared_ptr<SendBuffer>> _sendBuffers;

	// Accept
	shared_ptr<Session> _session = nullptr;
};

