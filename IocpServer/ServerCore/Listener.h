#pragma once

#include "IocpObject.h"

class JobQueue;

class Listener : public IocpObject
{
public:
	Listener();
	virtual ~Listener();

public:
	virtual void Dispatch(IocpEvent* iocpEvent, uint32 numOfBytes) override;

public:
	bool Start(uint32 acceptCount);

private:
	void RegisterAccept(IocpEvent* acceptEvent);
	void ProcessAccept(IocpEvent* acceptEvent);

private:
	vector<IocpEvent*> _acceptEvents;
	shared_ptr<JobQueue> _jobQueue = nullptr;
};
