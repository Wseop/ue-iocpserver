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
}

void ServerPacketHandler::HandlePing(shared_ptr<Session> session, BYTE* payload, uint32 payloadSize)
{
    Protocol::Ping message;
    message.ParseFromArray(payload, payloadSize);

    // Ŭ�󿡼� Ping �޼����� ���������� �Դٸ� ����
    if (message.msg() == "Ping!")
        session->Send(MakePing());
}

void ServerPacketHandler::HandleC_Enter(shared_ptr<Session> session, BYTE* payload, uint32 payloadSize)
{
    Protocol::C_Enter message;
    message.ParseFromArray(payload, payloadSize);

    // Ű�� ��ġ�ϸ� �濡 ����
    if (message.enter_key() == "123")
        gRoom->Enter(session);
    // Ű�� ��ġ���� ������ ���� ���� �޼��� ����
    else
        session->Send(MakeS_Enter(false, session->GetSessionId(), nullptr));
}

void ServerPacketHandler::HandleC_Exit(shared_ptr<Session> session, BYTE* payload, uint32 payloadSize)
{
    Protocol::C_Exit message;
    message.ParseFromArray(payload, payloadSize);

    const uint32 enterId = message.enter_id();

    bool result = true;

    // EnterId ���� Packet�� ������ ������ Id�� ��ġ�ؾ� ��
    if (session->GetSessionId() != enterId)
        result = false;

    // Exit ó��
    if (result)
        result = gRoom->Exit(enterId);

    session->Send(MakeS_Exit(result, enterId));
}

shared_ptr<SendBuffer> ServerPacketHandler::MakePing()
{
    Protocol::Ping payload;
    payload.set_msg("Pong!");

    return MakeSendBuffer(PacketType::Ping, &payload);
}

shared_ptr<SendBuffer> ServerPacketHandler::MakeS_Enter(bool result, uint32 enterId, Protocol::PlayerInfo* playerInfo)
{
    Protocol::S_Enter payload;
    
    payload.set_result(result);
    payload.set_enter_id(enterId);

    if (playerInfo != nullptr)
        payload.mutable_player_info()->CopyFrom(*playerInfo);

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
    Protocol::S_Spawn payload;
    
    for (shared_ptr<Player>& player : players)
    {
        Protocol::PlayerInfo* playerInfo = payload.add_player_infos();
        playerInfo->CopyFrom(player->GetPlayerInfo());
    }

    return MakeSendBuffer(PacketType::S_Spawn, &payload);
}

shared_ptr<SendBuffer> ServerPacketHandler::MakeS_Despawn(vector<uint32> playerIds)
{
    Protocol::S_Despawn payload;

    for (uint32 id : playerIds)
        payload.add_player_ids(id);

    return MakeSendBuffer(PacketType::S_Despawn, &payload);
}