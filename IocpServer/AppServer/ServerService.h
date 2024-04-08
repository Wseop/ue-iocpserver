#pragma once

#include "Service.h"

class Listener;

class ServerService : public Service
{
public:
	ServerService(NetAddress netAddress, SessionFactory sessionFactory);
	virtual ~ServerService();

public:
	bool Start(uint32 acceptCount);

private:
	shared_ptr<Listener> _listener = nullptr;
};

