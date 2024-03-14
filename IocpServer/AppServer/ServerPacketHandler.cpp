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
    gPacketHandler[static_cast<uint16>(PacketType::C_Spawn)] = HandleC_Spawn;
}

void ServerPacketHandler::HandlePing(shared_ptr<Session> session, BYTE* payload, uint32 payloadSize)
{
    Protocol::Ping message;
    message.ParseFromArray(payload, payloadSize);

    // 클라에서 Ping 메세지가 정상적으로 왔다면 응답
    if (message.msg() == "Ping!")
        session->Send(MakePing());
}

void ServerPacketHandler::HandleC_Enter(shared_ptr<Session> session, BYTE* payload, uint32 payloadSize)
{
    Protocol::C_Enter message;
    message.ParseFromArray(payload, payloadSize);

    bool result = false;

    // 키가 일치하면 방에 입장
    if (message.enter_key() == "123")
    {
        result = gRoom->Enter(session);
    }

    // 입장 처리 결과 전송
    session->Send(MakeS_Enter(result, session->GetSessionId()));
}

void ServerPacketHandler::HandleC_Exit(shared_ptr<Session> session, BYTE* payload, uint32 payloadSize)
{
    Protocol::C_Exit message;
    message.ParseFromArray(payload, payloadSize);

    const uint32 enterId = message.enter_id();

    bool result = gRoom->Exit(enterId);

    session->Send(MakeS_Exit(result, enterId));
}

void ServerPacketHandler::HandleC_Spawn(shared_ptr<Session> session, BYTE* payload, uint32 payloadSize)
{
}

shared_ptr<SendBuffer> ServerPacketHandler::MakePing()
{
    Protocol::Ping payload;
    payload.set_msg("Pong!");

    return MakeSendBuffer(PacketType::Ping, &payload);
}

shared_ptr<SendBuffer> ServerPacketHandler::MakeS_Enter(bool result, uint32 enterId)
{
    Protocol::S_Enter payload;
    
    payload.set_result(result);
    payload.set_enter_id(enterId);

    return MakeSendBuffer(PacketType::S_Enter, &payload);
}

shared_ptr<SendBuffer> ServerPacketHandler::MakeS_Exit(bool result, uint32 enterId)
{
    Protocol::S_Exit payload;
    
    payload.set_result(result);
    payload.set_enter_id(enterId);

    return MakeSendBuffer(PacketType::S_Exit, &payload);
}

shared_ptr<SendBuffer> ServerPacketHandler::MakeS_Spawn(vector<shared_ptr<Player>> players)
{
    return shared_ptr<SendBuffer>();
}

shared_ptr<SendBuffer> ServerPacketHandler::MakeS_Despawn(vector<uint64> playerIds)
{
    return shared_ptr<SendBuffer>();
}