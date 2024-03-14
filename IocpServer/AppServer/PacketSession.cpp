#include "pch.h"
#include "PacketSession.h"
#include "ServerPacketHandler.h"
#include "Room.h"

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
	// Room에서 Exit 처리
	gRoom->Exit(GetSessionId());
}

void PacketSession::OnRecv(BYTE* packet)
{
	ServerPacketHandler::HandlePacket(dynamic_pointer_cast<Session>(shared_from_this()), packet);
}

void PacketSession::OnSend(uint32 numOfBytes)
{
}
