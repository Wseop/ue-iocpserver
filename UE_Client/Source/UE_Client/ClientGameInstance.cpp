// Fill out your copyright notice in the Description page of Project Settings.


#include "ClientGameInstance.h"
#include "Sockets.h"
#include "SocketSubsystem.h"
#include "Common/TcpSocketBuilder.h"

void UClientGameInstance::FinishDestroy()
{
	DisconnectFromServer();

	Super::FinishDestroy();
}

void UClientGameInstance::ConnectToServer()
{
	_socket = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateSocket(TEXT("Stream"), TEXT("ClientSocket"));

	FIPv4Address ip;
	FIPv4Address::Parse(TEXT("127.0.0.1"), ip);

	TSharedRef<FInternetAddr> internetAddr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
	internetAddr->SetIp(ip.Value);
	internetAddr->SetPort(7777);

	bool bConnected = _socket->Connect(*internetAddr);

	if (bConnected)
	{
		// 서버 연결 성공, 세션 시작
		UE_LOG(LogTemp, Log, TEXT("서버 연결 성공"));
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("서버 연결 실패"));
	}
}

void UClientGameInstance::DisconnectFromServer()
{
	if (_socket)
	{
		ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(_socket);
		_socket = nullptr;
	}
}
