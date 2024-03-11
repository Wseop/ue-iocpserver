// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SendBuffer.h"

enum class PacketType : uint16
{
	None,
	Ping,
	C_Enter,
	S_Enter,
	S_SpawnPlayer,
};

struct PacketHeader
{
	PacketType packetType = PacketType::None;
	uint32 packetSize = 0;
};

class PacketSession;

using PacketHandlerFunc = TFunction<void(TSharedPtr<PacketSession>, BYTE*, uint32)>;
extern PacketHandlerFunc GPacketHandler[UINT16_MAX];

/**
 * 
 */
class UE_CLIENT_API ClientPacketHandler
{
public:
	static void Init();
	static void HandlePacket(TSharedPtr<PacketSession> packetSession, BYTE* packet);

private:
	// Header parsing - PacketType, Payload, Payload�� ũ�⸦ ��ȯ
	static BYTE* HandleHeader(BYTE* packet, OUT PacketType& packetType, OUT uint32& payloadSize);

	template<typename T>
	static inline TSharedPtr<SendBuffer> MakeSendBuffer(PacketType packetType, T* payload)
	{
		// Packet ũ�� ���
		uint32 payloadSize = static_cast<uint32>(payload->ByteSizeLong());
		uint32 packetSize = sizeof(PacketHeader) + payloadSize;

		// SendBuffer ����
		TSharedPtr<SendBuffer> sendBuffer = MakeShared<SendBuffer>(packetSize);

		// SendBuffer�� Header �߰�
		PacketHeader* header = reinterpret_cast<PacketHeader*>(sendBuffer->Buffer());
		header->packetType = packetType;
		header->packetSize = packetSize;

		// SendBuffer�� Payload �߰� (����� �̾)
		check(payload->SerializeToArray(header + 1, payloadSize));

		return sendBuffer;
	}

	// Packet Handlers
private:
	static void HandleInvalid(TSharedPtr<PacketSession> packetSession, BYTE* payload, uint32 payloadSize);
	static void HandlePing(TSharedPtr<PacketSession> packetSession, BYTE* payload, uint32 payloadSize);
	static void HandleS_Enter(TSharedPtr<PacketSession> packetSession, BYTE* payload, uint32 payloadSize);
	static void HandleS_SpawnPlayer(TSharedPtr<PacketSession> packetSession, BYTE* payload, uint32 payloadSize);

	// Packet Makers
public:
	static TSharedPtr<SendBuffer> MakePing();
	static TSharedPtr<SendBuffer> MakeC_Enter();
};
