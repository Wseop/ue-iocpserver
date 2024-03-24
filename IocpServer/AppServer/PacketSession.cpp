#include "pch.h"
#include "PacketSession.h"
#include "ServerPacketHandler.h"
#include "Room.h"
#include "Job.h"
#include "Protocol.pb.h"

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
	gRoom->Push(make_shared<Job>(gRoom, &Room::Exit, dynamic_pointer_cast<Session>(shared_from_this()), Protocol::C_Exit()));
}

void PacketSession::OnRecv(BYTE* packet)
{
	ServerPacketHandler::HandlePacket(dynamic_pointer_cast<Session>(shared_from_this()), packet);
}

void PacketSession::OnSend(uint32 numOfBytes)
{
}
