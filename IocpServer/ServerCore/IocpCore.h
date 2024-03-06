#pragma once

class IocpObject;

class IocpCore
{
public:
	IocpCore();
	~IocpCore();

public:
	bool Register(shared_ptr<IocpObject> iocpObject);
	void Dispatch(uint32 timeoutMs = INFINITE);

private:
	HANDLE _iocpHandle = INVALID_HANDLE_VALUE;
};

