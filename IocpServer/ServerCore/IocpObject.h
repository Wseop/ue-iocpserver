#pragma once

class IocpEvent;
class Service;

class IocpObject : public enable_shared_from_this<IocpObject>
{
public:
	IocpObject();
	virtual ~IocpObject();

public:
	virtual void Dispatch(IocpEvent* iocpEvent, uint32 numOfBytes) abstract;

public:
	SOCKET GetSocket() const { return _socket; }
	HANDLE GetHandle() const { return reinterpret_cast<HANDLE>(_socket); }

	shared_ptr<Service> GetService() const { return _service.lock(); }
	void SetService(weak_ptr<Service> service) { _service = service; }

protected:
	SOCKET _socket = INVALID_SOCKET;

private:
	weak_ptr<Service> _service;
};

