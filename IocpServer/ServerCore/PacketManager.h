#pragma once

#include "SendBuffer.h"

enum class PacketType : uint16
{
	Ping,
	C_Enter,
	S_Enter,
	C_Exit,
	S_Exit,
	C_Spawn,
	S_Spawn,
	S_Despawn,
	C_Move,
	S_Move,
};

struct PacketHeader
{
	PacketType packetType;
	uint32 packetSize;
};

using PacketHandlerFunc = function<void(shared_ptr<class PacketSession>, BYTE*, uint32)>;

class PacketManager
{
private:
	static PacketManager* _instance;

	PacketManager();
	~PacketManager();

public:
	static PacketManager* instance();

	inline uint32 getHeaderSize() const { return _headerSize; }

	template<typename T>
	shared_ptr<SendBuffer> makePacket(PacketType packetType, T* payload)
	{
		// Packet 크기 계산 (PacketSize = HeaderSize + PayloadSize)
		uint32 payloadSize = static_cast<uint32>(payload->ByteSizeLong());
		uint32 packetSize = _headerSize + payloadSize;

		// SendBuffer 할당
		shared_ptr<SendBuffer> sendBuffer = make_shared<SendBuffer>();

		// Header 추가
		PacketHeader* header = reinterpret_cast<PacketHeader*>(sendBuffer->buffer());
		header->packetType = packetType;
		header->packetSize = packetSize;

		// Payload 추가
		assert(payload->SerializeToArray(header + 1, payloadSize));

		return sendBuffer;
	}

	void addPacketHandler(PacketType packetType, PacketHandlerFunc func);
	uint32 handlePacket(shared_ptr<class PacketSession> session, BYTE* data, uint32 dataSize);

private:
	const uint32 _headerSize;
	PacketHandlerFunc _packetHandlers[UINT16_MAX];
};

