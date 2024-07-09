#include "pch.h"
#include "PacketManager.h"

PacketManager* PacketManager::_instance = nullptr;

PacketManager::PacketManager() :
	_headerSize(sizeof(PacketHeader))
{
}

PacketManager::~PacketManager()
{
}

PacketManager* PacketManager::instance()
{
	if (_instance == nullptr)
	{
		_instance = new PacketManager();
	}

	return _instance;
}

void PacketManager::addPacketHandler(PacketType packetType, PacketHandlerFunc func)
{
	_packetHandlers[static_cast<uint16>(packetType)] = func;
}

uint32 PacketManager::handlePacket(shared_ptr<class PacketSession> session, BYTE* data, uint32 dataSize)
{
	// Header Parsing이 불가능한 크기이면 0 반환
	if (dataSize < _headerSize)
		return 0;

	// Header Parsing
	PacketHeader* header = reinterpret_cast<PacketHeader*>(data);
	PacketType packetType = header->packetType;
	uint32 packetSize = header->packetSize;

	// 완전한 Packet이 아니면 0 반환
	if (dataSize < packetSize)
		return 0;

	BYTE* payload = reinterpret_cast<BYTE*>(header + 1);
	uint32 payloadSize = packetSize - _headerSize;

	// Handler 호출
	_packetHandlers[static_cast<uint16>(packetType)](session, payload, payloadSize);

	return packetSize;
}
