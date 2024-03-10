#include "pch.h"
#include "ServerPacketHandler.h"
#include "PacketSession.h"
#include "Protocol.pb.h"
#include "Job.h"

void ServerPacketHandler::Init()
{
    PacketHandler::Init();

    GPacketHandler[static_cast<uint16>(PacketType::Ping)] = HandlePing;
    GPacketHandler[static_cast<uint16>(PacketType::C_Enter)] = HandleC_Enter;
}

void ServerPacketHandler::HandlePing(shared_ptr<Session> session, BYTE* payload, uint32 payloadSize)
{
    Protocol::Ping ping;
    ping.ParseFromArray(payload, payloadSize);

    // Ŭ�󿡼� Ping �޼����� ���������� �Դٸ� ����
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

void ServerPacketHandler::HandleC_Enter(shared_ptr<Session> session, BYTE* payload, uint32 payloadSize)
{
    Protocol::C_Enter cEnter;
    cEnter.ParseFromArray(payload, payloadSize);

    // key check
    bool bResult = cEnter.key() == "12345" ? true : false;

    // ���� �������� ����
    if (shared_ptr<PacketSession> packetSession = dynamic_pointer_cast<PacketSession>(session))
    {
        packetSession->PushSendJob(
            make_shared<Job>([packetSession, bResult]() { packetSession->Send(MakeS_Enter(bResult)); }),
            false
        );
    }

    // TODO. key�� ��ġ�ϸ� ĳ���� Spawn
}

shared_ptr<SendBuffer> ServerPacketHandler::MakePing()
{
    Protocol::Ping ping;
    ping.set_msg("Pong!");

    return MakeSendBuffer(PacketType::Ping, &ping);
}

shared_ptr<SendBuffer> ServerPacketHandler::MakeS_Enter(bool bResult)
{
    Protocol::S_Enter sEnter;

    if (bResult)
        sEnter.set_result(1);
    else
        sEnter.set_result(-1);

    return MakeSendBuffer(PacketType::S_Enter, &sEnter);
}
