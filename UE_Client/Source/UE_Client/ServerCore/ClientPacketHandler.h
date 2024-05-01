// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SendBuffer.h"
#include "Protocol.pb.h"

enum class EPacketType : uint16
{
	None,
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

struct FPacketHeader
{
	EPacketType PacketType = EPacketType::None;
	uint32 PacketSize = 0;
};

class FPacketSession;

using PacketHandlerFunc = TFunction<void(TSharedPtr<FPacketSession>, BYTE*, uint32)>;
extern PacketHandlerFunc GPacketHandler[UINT16_MAX];

/**
 * 
 */
class UE_CLIENT_API FClientPacketHandler
{
public:
	static void Init();
	static void HandlePacket(TSharedPtr<FPacketSession> PacketSession, BYTE* Packet);

private:
	// Header parsing - PacketType, Payload, Payload의 크기를 반환
	static BYTE* HandleHeader(BYTE* Packet, OUT EPacketType& PacketType, OUT uint32& PayloadSize);

	template<typename T>
	static inline TSharedPtr<FSendBuffer> MakeSendBuffer(EPacketType PacketType, T* Payload)
	{
		// Packet 크기 계산
		uint32 PayloadSize = StaticCast<uint32>(Payload->ByteSizeLong());
		uint32 PacketSize = sizeof(FPacketHeader) + PayloadSize;

		// SendBuffer 생성
		TSharedPtr<FSendBuffer> SendBuffer = MakeShared<FSendBuffer>(PacketSize);

		// SendBuffer에 Header 추가
		FPacketHeader* Header = reinterpret_cast<FPacketHeader*>(SendBuffer->GetBuffer());
		Header->PacketType = PacketType;
		Header->PacketSize = PacketSize;

		// SendBuffer에 Payload 추가 (헤더에 이어서)
		check(Payload->SerializeToArray(Header + 1, PayloadSize));

		return SendBuffer;
	}

	// Packet Handlers
private:
	static void HandleInvalid(TSharedPtr<FPacketSession> PacketSession, BYTE* Payload, uint32 PayloadSize);
	static void HandlePing(TSharedPtr<FPacketSession> PacketSession, BYTE* Payload, uint32 PayloadSize);
	static void HandleS_Enter(TSharedPtr<FPacketSession> PacketSession, BYTE* Payload, uint32 PayloadSize);
	static void HandleS_Exit(TSharedPtr<FPacketSession> PacketSession, BYTE* Payload, uint32 PayloadSize);
	static void HandleS_Spawn(TSharedPtr<FPacketSession> PacketSession, BYTE* Payload, uint32 PayloadSize);
	static void HandleS_Despawn(TSharedPtr<FPacketSession> PacketSession, BYTE* Payload, uint32 PayloadSize);
	static void HandleS_Move(TSharedPtr<FPacketSession> PacketSession, BYTE* Payload, uint32 PayloadSize);

	// Packet Makers
public:
	static inline TSharedPtr<FSendBuffer> MakePing(Protocol::Ping* Payload) { return MakeSendBuffer(EPacketType::Ping, Payload); }
	static inline TSharedPtr<FSendBuffer> MakeC_Enter(Protocol::C_Enter* Payload) { return MakeSendBuffer(EPacketType::C_Enter, Payload); }
	static inline TSharedPtr<FSendBuffer> MakeC_Exit(Protocol::C_Exit* Payload) { return MakeSendBuffer(EPacketType::C_Exit, Payload); }
	static inline TSharedPtr<FSendBuffer> MakeC_Spawn(Protocol::C_Spawn* Payload) { return MakeSendBuffer(EPacketType::C_Spawn, Payload); }
	static inline TSharedPtr<FSendBuffer> MakeC_Move(Protocol::C_Move* Payload) { return MakeSendBuffer(EPacketType::C_Move, Payload); }
};