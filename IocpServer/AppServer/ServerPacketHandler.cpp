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

    GPacketHandler[static_cast<uint16>(PacketType::Ping)] = HandlePing;
    GPacketHandler[static_cast<uint16>(PacketType::C_Enter)] = HandleC_Enter;
    GPacketHandler[static_cast<uint16>(PacketType::C_Exit)] = HandleC_Exit;
}

void ServerPacketHandler::HandlePing(shared_ptr<Session> session, BYTE* payload, uint32 payloadSize)
{
    Protocol::Ping ping;
    ping.ParseFromArray(payload, payloadSize);

    // 클라에서 Ping 메세지가 정상적으로 왔다면 응답
    if (ping.msg() == "Ping!")
    {
        if (shared_ptr<PacketSession> packetSession = dynamic_pointer_cast<PacketSession>(session))
        {
            packetSession->PushSendJob(
                make_shared<Job>([packetSession](){ packetSession->Send(MakePing()); }), 
                false
            );
        }
    }
}

void ServerPacketHandler::HandleC_Enter(shared_ptr<Session> session, BYTE* payload, uint32 payloadSize)
{
    Protocol::C_Enter cEnter;
    cEnter.ParseFromArray(payload, payloadSize);

    // key check
    int32 result = cEnter.key() == "12345" ? 1 : -1;
    shared_ptr<PacketSession> packetSession = dynamic_pointer_cast<PacketSession>(session);

    if (packetSession)
    {
        // Player 입장
        if (result)
        {
            GRoom->EnterPlayer(ObjectManager::CreatePlayer(packetSession));
        }
        else
        {
            packetSession->PushSendJob(
                make_shared<Job>([packetSession, result]() { packetSession->Send(MakeS_Enter(result, -1)); }),
                false
            );
        }
    }
}

void ServerPacketHandler::HandleC_Exit(shared_ptr<Session> session, BYTE* payload, uint32 payloadSize)
{
    Protocol::C_Exit cExit;
    cExit.ParseFromArray(payload, payloadSize);

    GRoom->ExitPlayer(cExit.player_id());
}

shared_ptr<SendBuffer> ServerPacketHandler::MakePing()
{
    Protocol::Ping ping;
    ping.set_msg("Pong!");

    return MakeSendBuffer(PacketType::Ping, &ping);
}

shared_ptr<SendBuffer> ServerPacketHandler::MakeS_Enter(int32 result, uint64 playerId)
{
    Protocol::S_Enter sEnter;

    sEnter.set_result(result);
    sEnter.set_player_id(playerId);

    return MakeSendBuffer(PacketType::S_Enter, &sEnter);
}

shared_ptr<SendBuffer> ServerPacketHandler::MakeS_Exit(int32 result)
{
    Protocol::S_Exit sExit;

    sExit.set_result(result);

    return MakeSendBuffer(PacketType::S_Exit, &sExit);
}

shared_ptr<SendBuffer> ServerPacketHandler::MakeS_SpawnPlayer(vector<shared_ptr<Player>> players)
{
    Protocol::S_SpawnPlayer spawnPlayer;

    for (shared_ptr<Player>& player : players)
    {
        Protocol::PlayerInfo* playerInfo = spawnPlayer.add_players();
        playerInfo->set_player_id(player->GetPlayerId());
        playerInfo->set_x(player->GetX());
        playerInfo->set_y(player->GetY());
        playerInfo->set_z(player->GetZ());
        playerInfo->set_yaw(player->GetYaw());
    }

    return MakeSendBuffer(PacketType::S_SpawnPlayer, &spawnPlayer);
}

shared_ptr<SendBuffer> ServerPacketHandler::MakeS_DespawnPlayer(vector<uint64> playerIds)
{
    Protocol::S_DespawnPlayer despawnPlayer;

    for (uint64 playerId : playerIds)
    {
        despawnPlayer.add_player_ids(playerId);
    }

    return MakeSendBuffer(PacketType::S_DespawnPlayer, &despawnPlayer);
}
