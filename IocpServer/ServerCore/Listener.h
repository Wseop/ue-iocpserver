#pragma once

#include "IocpObject.h"

class Listener : public IocpObject
{
public:
	Listener(uint32 acceptCount);
	virtual ~Listener();

	virtual void Dispatch(IocpEvent* iocpEvent, uint32 numOfBytes) override;

public:
	bool StartAccept();

private:
	void RegisterAccept(IocpEvent* acceptEvent);
	void ProcessAccept(IocpEvent* acceptEvent);

private:
	uint32 _acceptCount = 0;
	vector<IocpEvent*> _acceptEvents;
};

