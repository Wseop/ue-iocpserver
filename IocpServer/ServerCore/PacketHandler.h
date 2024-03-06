#pragma once

#include "SendBuffer.h"

enum class PacketType : uint16
{
	None,
	Test
};

struct PacketHeader
{
	PacketType packetType = PacketType::None;
	uint32 packetSize = 0;
};

class Session;

using PacketHandlerFunc = function<void(shared_ptr<Session>, BYTE*, uint32)>;
extern PacketHandlerFunc GPacketHandler[UINT16_MAX];

class PacketHandler
{
public:
	static void Init();
	static void HandlePacket(shared_ptr<Session> session, BYTE* packet);

private:
	// Header parsing - PacketType, Payload, Payload�� ũ�⸦ ��ȯ
	static BYTE* HandleHeader(BYTE* packet, OUT PacketType& packetType, OUT uint32& payloadSize);

protected:
	template<typename T>
	static inline shared_ptr<SendBuffer> MakeSendBuffer(PacketType packetType, T* payload)
	{
		// Packet ũ�� ���
		uint32 payloadSize = static_cast<uint32>(payload->ByteSizeLong());
		uint32 packetSize = payloadSize + sizeof(PacketHeader);

		// SendBuffer ����
		shared_ptr<SendBuffer> sendBuffer = make_shared<SendBuffer>(packetSize);

		// SendBuffer�� Header �߰�
		PacketHeader* header = reinterpret_cast<PacketHeader*>(sendBuffer->Buffer());
		header->packetType = packetType;
		header->packetSize = packetSize;

		// SendBuffer�� Payload �߰�
		assert(payload->SerializeToArray(header + 1, payloadSize));

		return sendBuffer;
	}

	// Packet Handlers
private:
	static void HandleInvalid(shared_ptr<Session> session, BYTE* payload, uint32 payloadSize);

	// Packet Makers
public:
};

