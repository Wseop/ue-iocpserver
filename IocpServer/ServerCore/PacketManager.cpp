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
	// Header Parsing�� �Ұ����� ũ���̸� 0 ��ȯ
	if (dataSize < _headerSize)
		return 0;

	// Header Parsing
	PacketHeader* header = reinterpret_cast<PacketHeader*>(data);
	PacketType packetType = header->packetType;
	uint32 packetSize = header->packetSize;

	// ������ Packet�� �ƴϸ� 0 ��ȯ
	if (dataSize < packetSize)
		return 0;

	BYTE* payload = reinterpret_cast<BYTE*>(header + 1);
	uint32 payloadSize = packetSize - _headerSize;

	// Handler ȣ��
	_packetHandlers[static_cast<uint16>(packetType)](session, payload, payloadSize);

	return packetSize;
}
