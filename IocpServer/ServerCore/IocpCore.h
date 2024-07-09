#pragma once

class IocpObject;

class IocpCore
{
public:
	IocpCore();
	~IocpCore();

public:
	bool registerObject(shared_ptr<IocpObject> iocpObject);
	void dispatchEvent(uint32 timeoutMs = INFINITE);

private:
	HANDLE _iocpHandle = INVALID_HANDLE_VALUE;
};

