#include "pch.h"
#include "PacketSession.h"
#include "ServerPacketHandler.h"
#include "JobQueue.h"

PacketSession::PacketSession() :
	_sendJobQueue(make_shared<JobQueue>())
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
	ServerPacketHandler::HandlePacket(dynamic_pointer_cast<Session>(shared_from_this()), packet);
}

void PacketSession::OnSend(uint32 numOfBytes)
{
}

void PacketSession::PushSendJob(shared_ptr<Job> sendJob, bool pushOnly)
{
	_sendJobQueue->Push(sendJob, pushOnly);
}
