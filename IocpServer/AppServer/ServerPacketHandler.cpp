#include "pch.h"
#include "ServerPacketHandler.h"
#include "PacketSession.h"
#include "Protocol.pb.h"
#include "Job.h"
#include "Room.h"
#include "ObjectManager.h"
#include "Player.h"

void ServerPacketHandler::Init()
{
    PacketHandler::Init();

    gPacketHandler[static_cast<uint16>(PacketType::Ping)] = HandlePing;
    gPacketHandler[static_cast<uint16>(PacketType::C_Enter)] = HandleC_Enter;
    gPacketHandler[static_cast<uint16>(PacketType::C_Exit)] = HandleC_Exit;
    gPacketHandler[static_cast<uint16>(PacketType::C_Move)] = HandleC_Move;
}

void ServerPacketHandler::HandlePing(shared_ptr<Session> session, BYTE* payload, uint32 payloadSize)
{
    Protocol::Ping message;
    message.ParseFromArray(payload, payloadSize);

    // 클라에서 Ping 메세지가 정상적으로 왔다면 Pong으로 응답
    if (message.msg() == "Ping!")
    {
        Protocol::Ping reply;
        reply.set_msg("Pong!");
        session->Send(MakePing(&reply));
    }
}

void ServerPacketHandler::HandleC_Enter(shared_ptr<Session> session, BYTE* payload, uint32 payloadSize)
{
    Protocol::C_Enter message;
    message.ParseFromArray(payload, payloadSize);
    gRoom->Push(make_shared<Job>(gRoom, &Room::Enter, session, message));
}

void ServerPacketHandler::HandleC_Exit(shared_ptr<Session> session, BYTE* payload, uint32 payloadSize)
{
    Protocol::C_Exit message; 
    message.ParseFromArray(payload, payloadSize);
    gRoom->Push(make_shared<Job>(gRoom, &Room::Exit, session, message));
}

void ServerPacketHandler::HandleC_Move(shared_ptr<Session> session, BYTE* payload, uint32 payloadSize)
{
    Protocol::C_Move message;
    message.ParseFromArray(payload, payloadSize);
    gRoom->Push(make_shared<Job>(gRoom, &Room::MovePlayer, session, message));
}