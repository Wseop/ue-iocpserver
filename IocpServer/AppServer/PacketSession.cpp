#include "pch.h"
#include "PacketSession.h"
#include "ServerPacketHandler.h"

PacketSession::PacketSession(uint32 sessionId) :
	Session(sessionId)
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
}

void PacketSession::OnRecv(BYTE* packet)
{
	ServerPacketHandler::HandlePacket(dynamic_pointer_cast<Session>(shared_from_this()), packet);
}

void PacketSession::OnSend(uint32 numOfBytes)
{
}
