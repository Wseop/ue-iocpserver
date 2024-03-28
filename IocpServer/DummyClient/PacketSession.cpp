#include "pch.h"
#include "PacketSession.h"
#include "ClientPacketHandler.h"

PacketSession::PacketSession()
{}

PacketSession::~PacketSession()
{}

void PacketSession::OnConnect()
{
	spdlog::info("Connected to server : SessionId : {}", GetSessionId());
}

void PacketSession::OnDisconnect()
{
	
}

void PacketSession::OnRecv(BYTE* packet)
{
	ClientPacketHandler::HandlePacket(dynamic_pointer_cast<Session>(shared_from_this()), packet);
}

void PacketSession::OnSend(uint32 numOfBytes)
{
	
}