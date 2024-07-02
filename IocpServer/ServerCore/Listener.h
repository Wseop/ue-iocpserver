#pragma once

#include "IocpObject.h"

class JobQueue;

class Listener : public IocpObject
{
public:
	Listener();
	virtual ~Listener();

public:
	virtual void processEvent(IocpEvent* iocpEvent, uint32 numOfBytes) override;

public:
	bool start(uint32 acceptCount);

private:
	void registerAccept(IocpEvent* acceptEvent);
	void processAccept(IocpEvent* acceptEvent);

private:
	vector<IocpEvent*> _acceptEvents;
	shared_ptr<JobQueue> _jobQueue = nullptr;
};
