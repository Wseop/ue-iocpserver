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

    // 클라에서 Ping 메세지가 정상적으로 왔다면 응답
    if (message.msg() == "Ping!")
        session->Send(MakePing());
}

void ServerPacketHandler::HandleC_Enter(shared_ptr<Session> session, BYTE* payload, uint32 payloadSize)
{
    Protocol::C_Enter message;
    message.ParseFromArray(payload, payloadSize);

    int32 result = -1;
    uint64 newPlayerId = 0;

    // key가 일치하면 입장
    if (message.key() == "12345")
    {
        // 캐릭터 생성
        shared_ptr<Player> newPlayer = ObjectManager::CreatePlayer(dynamic_pointer_cast<PacketSession>(session));

        if (newPlayer && gRoom->EnterPlayer(newPlayer))
        {
            result = 1;
            newPlayerId = newPlayer->GetPlayerId();
        }
    }

    // Enter 결과 전송
    session->Send(MakeS_Enter(result, newPlayerId));
}

void ServerPacketHandler::HandleC_Exit(shared_ptr<Session> session, BYTE* payload, uint32 payloadSize)
{
    Protocol::C_Exit message;
    message.ParseFromArray(payload, payloadSize);

    int32 result = -1;

    if (gRoom->ExitPlayer(message.player_id()))
        result = 1;

    session->Send(MakeS_Exit(result));
}

shared_ptr<SendBuffer> ServerPacketHandler::MakePing()
{
    Protocol::Ping payload;
    payload.set_msg("Pong!");

    return MakeSendBuffer(PacketType::Ping, &payload);
}

shared_ptr<SendBuffer> ServerPacketHandler::MakeS_Enter(int32 result, uint64 playerId)
{
    Protocol::S_Enter payload;
    payload.set_result(result);
    payload.set_player_id(playerId);

    return MakeSendBuffer(PacketType::S_Enter, &payload);
}

shared_ptr<SendBuffer> ServerPacketHandler::MakeS_Exit(int32 result)
{
    Protocol::S_Exit payload;
    payload.set_result(result);

    return MakeSendBuffer(PacketType::S_Exit, &payload);
}

shared_ptr<SendBuffer> ServerPacketHandler::MakeS_SpawnPlayer(vector<shared_ptr<Player>> players)
{
    Protocol::S_SpawnPlayer payload;

    for (shared_ptr<Player>& player : players)
    {
        Protocol::PlayerInfo* playerInfo = payload.add_player_infos();
        playerInfo->set_player_id(player->GetPlayerId());
        playerInfo->set_x(player->GetX());
        playerInfo->set_y(player->GetY());
        playerInfo->set_z(player->GetZ());
        playerInfo->set_yaw(player->GetYaw());
    }

    return MakeSendBuffer(PacketType::S_SpawnPlayer, &payload);
}

shared_ptr<SendBuffer> ServerPacketHandler::MakeS_DespawnPlayer(vector<uint64> playerIds)
{
    Protocol::S_DespawnPlayer payload;

    for (uint64 playerId : playerIds)
    {
        payload.add_player_ids(playerId);
    }

    return MakeSendBuffer(PacketType::S_DespawnPlayer, &payload);
}
