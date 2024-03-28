#pragma once

#include "Service.h"

class ClientService : public Service
{
public:
	ClientService(NetAddress netAddress, SessionFactory sessionFactory, uint32 sessionCount);
	virtual ~ClientService();

	virtual bool Start() override;

private:
	uint32 _sessionCount = 0;
};

