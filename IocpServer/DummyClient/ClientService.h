#pragma once

#include "Service.h"

class ClientService : public Service
{
public:
	ClientService(NetAddress netAddress, SessionFactory sessionFactory);
	virtual ~ClientService();
};

