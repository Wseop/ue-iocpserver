#include "pch.h"
#include "PacketSession.h"
#include "ServerPacketHandler.h"

PacketSession::PacketSession()
{
	
}

PacketSession::~PacketSession()
{
}

void PacketSession::OnConnect()
{
}

void PacketSession::OnDisconnect()
{
	cout << "Client Disconnected" << endl;
}

void PacketSession::OnRecv(BYTE* packet)
{
}

void PacketSession::OnSend(uint32 numOfBytes)
{
}
