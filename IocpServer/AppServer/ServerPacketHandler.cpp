#include "pch.h"
#include "ServerPacketHandler.h"
#include "PacketSession.h"
#include "Protocol.pb.h"
#include "Job.h"

void ServerPacketHandler::Init()
{
    PacketHandler::Init();

    GPacketHandler[static_cast<uint16>(PacketType::Ping)] = HandlePing;
}

void ServerPacketHandler::HandlePing(shared_ptr<Session> session, BYTE* payload, uint32 payloadSize)
{
    Protocol::Ping ping;
    ping.ParseFromArray(payload, payloadSize);

    // 클라에서 Ping 메세지가 정상적으로 왔다면 응답
    if (ping.msg() == "Ping!")
    {
        if (shared_ptr<PacketSession> packetSession = dynamic_pointer_cast<PacketSession>(session))
        {
            packetSession->PushSendJob(
                make_shared<Job>([packetSession](){ packetSession->Send(MakePing()); }), 
                false
            );
        }
    }
}

shared_ptr<SendBuffer> ServerPacketHandler::MakePing()
{
    Protocol::Ping ping;
    ping.set_msg("Pong!");

    return MakeSendBuffer(PacketType::Ping, &ping);
}
