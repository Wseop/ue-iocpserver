#include "pch.h"
#include "PacketHandler.h"

PacketHandlerFunc gPacketHandler[UINT16_MAX];

void PacketHandler::Init()
{
	for (uint16 i = 0; i < UINT16_MAX; i++)
		gPacketHandler[i] = HandleInvalid;
}

void PacketHandler::HandlePacket(shared_ptr<Session> session, BYTE* packet)
{
	// 헤더 parsing
	PacketType packetType = PacketType::None;
	uint32 payloadSize = 0;
	BYTE* payload = HandleHeader(packet, OUT packetType, OUT payloadSize);

	// PacketType에 맞는 Handler 호출
	gPacketHandler[static_cast<uint16>(packetType)](session, payload, payloadSize);
}

BYTE* PacketHandler::HandleHeader(BYTE* packet, OUT PacketType& packetType, OUT uint32& payloadSize)
{
	PacketHeader* header = reinterpret_cast<PacketHeader*>(packet);

	// PacketType, Payload 크기 추출
	packetType = header->packetType;
	payloadSize = header->packetSize - sizeof(PacketHeader);

	// Payload 반환
	return reinterpret_cast<BYTE*>(header + 1);
}

void PacketHandler::HandleInvalid(shared_ptr<Session> session, BYTE* payload, uint32 payloadSize)
{
	cout << "Invalid Packet!" << endl;
}