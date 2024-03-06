#pragma once

#include "Service.h"

class Listener;

class ServerService : public Service
{
public:
	ServerService(NetAddress netAddress, SessionFactory sessionFactory, uint32 acceptCount);
	virtual ~ServerService();

	virtual bool Start() override;

private:
	uint32 _acceptCount = 0;
	shared_ptr<Listener> _listener = nullptr;
};

