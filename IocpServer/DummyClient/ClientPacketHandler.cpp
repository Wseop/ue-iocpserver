#include "pch.h"
#include "ClientPacketHandler.h"
#include "PacketSession.h"
#include "GameInstance.h"
#include "Job.h"

void ClientPacketHandler::Init()
{
	PacketHandler::Init();

	gPacketHandler[static_cast<uint16>(PacketType::Ping)] = HandlePing;
	gPacketHandler[static_cast<uint16>(PacketType::S_Enter)] = HandleS_Enter;
	gPacketHandler[static_cast<uint16>(PacketType::S_Exit)] = HandleS_Exit;
	gPacketHandler[static_cast<uint16>(PacketType::S_Spawn)] = HandleS_Spawn;
	gPacketHandler[static_cast<uint16>(PacketType::S_Despawn)] = HandleS_Despawn;
	gPacketHandler[static_cast<uint16>(PacketType::S_Move)] = HandleS_Move;
}

void ClientPacketHandler::HandlePing(shared_ptr<Session> session, BYTE* payload, uint32 payloadSize)
{
	Protocol::Ping pingPacket;
	pingPacket.ParseFromArray(payload, payloadSize);

	spdlog::info(pingPacket.msg());
}

void ClientPacketHandler::HandleS_Enter(shared_ptr<Session> session, BYTE* payload, uint32 payloadSize)
{
	Protocol::S_Enter enterPacket;
	enterPacket.ParseFromArray(payload, payloadSize);
	gGameInstance->Push(make_shared<Job>(gGameInstance, &GameInstance::HandleEnterGameRoom, enterPacket));
}

void ClientPacketHandler::HandleS_Exit(shared_ptr<Session> session, BYTE* payload, uint32 payloadSize)
{
	Protocol::S_Exit exitPacket;
	exitPacket.ParseFromArray(payload, payloadSize);
	gGameInstance->Push(make_shared<Job>(gGameInstance, &GameInstance::HandleExitGameRoom, exitPacket));
}

void ClientPacketHandler::HandleS_Spawn(shared_ptr<Session> session, BYTE* payload, uint32 payloadSize)
{
	Protocol::S_Spawn spawnPacket;
	spawnPacket.ParseFromArray(payload, payloadSize);
	gGameInstance->Push(make_shared<Job>(gGameInstance, &GameInstance::HandleSpawnPlayer, spawnPacket));
}

void ClientPacketHandler::HandleS_Despawn(shared_ptr<Session> session, BYTE* payload, uint32 payloadSize)
{
	Protocol::S_Despawn despawnPacket;
	despawnPacket.ParseFromArray(payload, payloadSize);
	gGameInstance->Push(make_shared<Job>(gGameInstance, &GameInstance::HandleDespawnPlayer, despawnPacket));
}

void ClientPacketHandler::HandleS_Move(shared_ptr<Session> session, BYTE* payload, uint32 payloadSize)
{
	Protocol::S_Move movePacket;
	movePacket.ParseFromArray(payload, payloadSize);
	gGameInstance->Push(make_shared<Job>(gGameInstance, &GameInstance::HandleMovePlayer, movePacket));
}