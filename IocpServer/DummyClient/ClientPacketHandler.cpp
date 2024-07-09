#include "pch.h"
#include "ClientPacketHandler.h"
#include "PacketManager.h"
#include "Session.h"
#include "GameInstance.h"
#include "Job.h"

void ClientPacketHandler::init()
{
	PacketManager* packetManager = PacketManager::instance();

	packetManager->addPacketHandler(PacketType::Ping, handlePing);
	packetManager->addPacketHandler(PacketType::S_Enter, handleS_Enter);
	packetManager->addPacketHandler(PacketType::S_Exit, handleS_Exit);
	packetManager->addPacketHandler(PacketType::S_Spawn, handleS_Spawn);
	packetManager->addPacketHandler(PacketType::S_Despawn, handleS_Despawn);
	packetManager->addPacketHandler(PacketType::S_Move, handleS_Move);
}

void ClientPacketHandler::handlePing(shared_ptr<PacketSession> session, BYTE* payload, uint32 payloadSize)
{
	Protocol::Ping pingPacket;
	pingPacket.ParseFromArray(payload, payloadSize);

	spdlog::info(pingPacket.msg());
}

void ClientPacketHandler::handleS_Enter(shared_ptr<PacketSession> session, BYTE* payload, uint32 payloadSize)
{
	Protocol::S_Enter enterPacket;
	enterPacket.ParseFromArray(payload, payloadSize);
	gGameInstance->push(make_shared<Job>(gGameInstance, &GameInstance::handleEnterGameRoom, enterPacket));
}

void ClientPacketHandler::handleS_Exit(shared_ptr<PacketSession> session, BYTE* payload, uint32 payloadSize)
{
	Protocol::S_Exit exitPacket;
	exitPacket.ParseFromArray(payload, payloadSize);
	gGameInstance->push(make_shared<Job>(gGameInstance, &GameInstance::handleExitGameRoom, exitPacket));
}

void ClientPacketHandler::handleS_Spawn(shared_ptr<PacketSession> session, BYTE* payload, uint32 payloadSize)
{
	Protocol::S_Spawn spawnPacket;
	spawnPacket.ParseFromArray(payload, payloadSize);
	gGameInstance->push(make_shared<Job>(gGameInstance, &GameInstance::handleSpawnPlayer, spawnPacket));
}

void ClientPacketHandler::handleS_Despawn(shared_ptr<PacketSession> session, BYTE* payload, uint32 payloadSize)
{
	Protocol::S_Despawn despawnPacket;
	despawnPacket.ParseFromArray(payload, payloadSize);
	gGameInstance->push(make_shared<Job>(gGameInstance, &GameInstance::handleDespawnPlayer, despawnPacket));
}

void ClientPacketHandler::handleS_Move(shared_ptr<PacketSession> session, BYTE* payload, uint32 payloadSize)
{
	Protocol::S_Move movePacket;
	movePacket.ParseFromArray(payload, payloadSize);
	gGameInstance->push(make_shared<Job>(gGameInstance, &GameInstance::handleMovePlayer, movePacket));
}

shared_ptr<SendBuffer> ClientPacketHandler::makePing(Protocol::Ping* payload)
{
	return PacketManager::instance()->makePacket(PacketType::Ping, payload);
}

shared_ptr<SendBuffer> ClientPacketHandler::makeC_Enter(Protocol::C_Enter* payload)
{
	return PacketManager::instance()->makePacket(PacketType::C_Enter, payload);
}

shared_ptr<SendBuffer> ClientPacketHandler::makeC_Exit(Protocol::C_Exit* payload)
{
	return PacketManager::instance()->makePacket(PacketType::C_Exit, payload);
}

shared_ptr<SendBuffer> ClientPacketHandler::makeC_Spawn(Protocol::C_Spawn* payload)
{
	return PacketManager::instance()->makePacket(PacketType::C_Spawn, payload);
}

shared_ptr<SendBuffer> ClientPacketHandler::makeC_Move(Protocol::C_Move* payload)
{
	return PacketManager::instance()->makePacket(PacketType::C_Move, payload);
}
