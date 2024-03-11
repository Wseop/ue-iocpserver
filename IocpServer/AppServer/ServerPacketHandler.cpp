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
    bool bResult = cEnter.key() == "12345" ? true : false;
    shared_ptr<PacketSession> packetSession = dynamic_pointer_cast<PacketSession>(session);

    if (packetSession)
    {
        // Player 입장
        if (bResult)
        {
            GRoom->EnterPlayer(ObjectManager::CreatePlayer(packetSession));
        }
        else
        {
            packetSession->PushSendJob(
                make_shared<Job>([packetSession, bResult]() { packetSession->Send(MakeS_Enter(bResult, -1)); }),
                false
            );
        }
    }
}

shared_ptr<SendBuffer> ServerPacketHandler::MakePing()
{
    Protocol::Ping ping;
    ping.set_msg("Pong!");

    return MakeSendBuffer(PacketType::Ping, &ping);
}

shared_ptr<SendBuffer> ServerPacketHandler::MakeS_Enter(bool bResult, uint64 playerId)
{
    Protocol::S_Enter sEnter;

    if (bResult)
        sEnter.set_result(1);
    else
        sEnter.set_result(-1);

    sEnter.set_player_id(playerId);

    return MakeSendBuffer(PacketType::S_Enter, &sEnter);
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
