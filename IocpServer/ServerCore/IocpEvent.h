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

	void init();

public:
	EventType getEventType() const { return _eventType; }

	shared_ptr<IocpObject> getOwner() const { return _owner; }
	void setOwner(shared_ptr<IocpObject> owner) { _owner = owner; }

	void addSendBuffer(shared_ptr<SendBuffer> sendBuffer);
	void clearSendBuffers();

	shared_ptr<Session> getSession() const { return _session; }
	void setSession(shared_ptr<Session> session) { _session = session; }

private:
	EventType _eventType;
	shared_ptr<IocpObject> _owner = nullptr;		// reference count
	vector<shared_ptr<SendBuffer>> _sendBuffers;	// send, reference count
	shared_ptr<Session> _session = nullptr;			// accept, reference count
};

