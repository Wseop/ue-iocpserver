#pragma once

#include "IocpCore.h"

class Session;

using SessionFactory = function<shared_ptr<Session>(uint32)>;

class Service : public enable_shared_from_this<Service>
{
public:
	Service(NetAddress netAddress, SessionFactory sessionFactory);
	virtual ~Service();

	virtual bool Start() abstract;

public:
	IocpCore& GetIocpCore() { return _iocpCore; }
	NetAddress GetNetAddress() { return _netAddress; }

	// Session
	shared_ptr<Session> CreateSession();
	shared_ptr<Session> GetSession(uint32 sessionId);
	void AddSession(shared_ptr<Session> session);
	void RemoveSession(uint32 sessionId);

private:
	IocpCore _iocpCore;
	NetAddress _netAddress;

	// Session
	mutex _mutex;
	SessionFactory _sessionFactory = nullptr;
	unordered_map<uint32, shared_ptr<Session>> _sessions;
};

