#pragma once

class IocpCore;
class Session;

using SessionFactory = function<shared_ptr<Session>(void)>;

class Service : public enable_shared_from_this<Service>
{
public:
	Service(NetAddress netAddress, SessionFactory sessionFactory);
	virtual ~Service();

public:
	shared_ptr<IocpCore> GetIocpCore() const { return _iocpCore; }

	NetAddress GetNetAddress() const { return _netAddress; }

	shared_ptr<Session> CreateSession();
	void AddSession(shared_ptr<Session> session);
	void RemoveSession(shared_ptr<Session> session);
	shared_ptr<Session> GetSession(uint32 sessionId);

private:
	shared_ptr<IocpCore> _iocpCore = nullptr;
	NetAddress _netAddress;
	SessionFactory _sessionFactory = nullptr;

	mutex _mutex;
	map<uint32, shared_ptr<Session>> _sessions;
};

