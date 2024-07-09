#pragma once

class Session;

using SessionFactory = function<shared_ptr<Session>(void)>;

class NetworkService : public enable_shared_from_this<NetworkService>
{
public:
	NetworkService(NetAddress netAddress, SessionFactory sessionFactory);
	virtual ~NetworkService();

public:
	inline NetAddress getNetAddress() const { return _netAddress; }

	bool listen(uint32 acceptCount);

	shared_ptr<Session> createSession();
	void addSession(shared_ptr<Session> session);
	void removeSession(shared_ptr<Session> session);
	shared_ptr<Session> getSession(uint32 sessionId);

private:
	NetAddress _netAddress;
	SessionFactory _sessionFactory = nullptr;

	mutex _mutex;
	shared_ptr<class Listener> _listener;
	map<uint32, shared_ptr<Session>> _sessions;
};
