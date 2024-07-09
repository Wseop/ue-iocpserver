#include "pch.h"
#include "ServerPacketHandler.h"
#include "PacketManager.h"
#include "SendBuffer.h"
#include "Session.h"
#include "Protocol.pb.h"
#include "Job.h"
#include "Room.h"
#include "ObjectManager.h"
#include "Player.h"

void ServerPacketHandler::init()
{
    PacketManager* packetManager = PacketManager::instance();

    packetManager->addPacketHandler(PacketType::Ping, handlePing);
    packetManager->addPacketHandler(PacketType::C_Enter, handleC_Enter);
    packetManager->addPacketHandler(PacketType::C_Exit, handleC_Exit);
    packetManager->addPacketHandler(PacketType::C_Spawn, handleC_Spawn);
    packetManager->addPacketHandler(PacketType::C_Move, handleC_Move);
}

void ServerPacketHandler::handlePing(shared_ptr<PacketSession> session, BYTE* payload, uint32 payloadSize)
{
    Protocol::Ping message;
    message.ParseFromArray(payload, payloadSize);

    // 클라에서 Ping 메세지가 정상적으로 왔다면 Pong으로 응답
    if (message.msg() == "Ping!")
    {
        Protocol::Ping reply;
        reply.set_msg("Pong!");
        session->send(makePing(&reply));
    }
}

void ServerPacketHandler::handleC_Enter(shared_ptr<PacketSession> session, BYTE* payload, uint32 payloadSize)
{
    Protocol::C_Enter message;
    message.ParseFromArray(payload, payloadSize);
    gRoom->push(make_shared<Job>(gRoom, &Room::enter, session, message));
}

void ServerPacketHandler::handleC_Exit(shared_ptr<PacketSession> session, BYTE* payload, uint32 payloadSize)
{
    Protocol::C_Exit message; 
    message.ParseFromArray(payload, payloadSize);
    gRoom->push(make_shared<Job>(gRoom, &Room::exit, session, message));
}

void ServerPacketHandler::handleC_Spawn(shared_ptr<PacketSession> session, BYTE* payload, uint32 payloadSize)
{
    Protocol::C_Spawn message;
    message.ParseFromArray(payload, payloadSize);
    gRoom->push(make_shared<Job>(gRoom, &Room::spawnPlayer, session));
}

void ServerPacketHandler::handleC_Move(shared_ptr<PacketSession> session, BYTE* payload, uint32 payloadSize)
{
    Protocol::C_Move message;
    message.ParseFromArray(payload, payloadSize);
    gRoom->push(make_shared<Job>(gRoom, &Room::movePlayer, session, message));
}

shared_ptr<SendBuffer> ServerPacketHandler::makePing(Protocol::Ping* payload)
{
    return PacketManager::instance()->makePacket(PacketType::Ping, payload);
}

shared_ptr<SendBuffer> ServerPacketHandler::makeS_Enter(Protocol::S_Enter* payload)
{
    return PacketManager::instance()->makePacket(PacketType::S_Enter, payload);
}

shared_ptr<SendBuffer> ServerPacketHandler::makeS_Exit(Protocol::S_Exit* payload)
{
    return PacketManager::instance()->makePacket(PacketType::S_Exit, payload);
}

shared_ptr<SendBuffer> ServerPacketHandler::makeS_Spawn(Protocol::S_Spawn* payload)
{
    return PacketManager::instance()->makePacket(PacketType::S_Spawn, payload);
}

shared_ptr<SendBuffer> ServerPacketHandler::makeS_Despawn(Protocol::S_Despawn* payload)
{
    return PacketManager::instance()->makePacket(PacketType::S_Despawn, payload);
}

shared_ptr<SendBuffer> ServerPacketHandler::makeS_Move(Protocol::S_Move* payload)
{
    return PacketManager::instance()->makePacket(PacketType::S_Move, payload);
}
