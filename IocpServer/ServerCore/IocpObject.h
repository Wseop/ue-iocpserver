#pragma once

class IocpEvent;
class Service;

class IocpObject : public enable_shared_from_this<IocpObject>
{
public:
	IocpObject();
	virtual ~IocpObject();

public:
	virtual void processEvent(IocpEvent* iocpEvent, uint32 numOfBytes) abstract;

public:
	SOCKET getSocket() const { return _socket; }
	HANDLE getHandle() const { return reinterpret_cast<HANDLE>(_socket); }

	shared_ptr<Service> getService() const { return _service.lock(); }
	void setService(weak_ptr<Service> service) { _service = service; }

protected:
	SOCKET _socket = INVALID_SOCKET;

private:
	weak_ptr<Service> _service;
};

