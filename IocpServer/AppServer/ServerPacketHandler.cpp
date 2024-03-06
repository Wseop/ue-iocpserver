#include "pch.h"
#include "ServerPacketHandler.h"
#include "Protocol.pb.h"

void ServerPacketHandler::Init()
{
    PacketHandler::Init();

    GPacketHandler[static_cast<uint16>(PacketType::Test)] = HandleTest;
}

void ServerPacketHandler::HandleTest(shared_ptr<Session> session, BYTE* payload, uint32 payloadSize)
{
    Protocol::Test test;
    test.ParseFromArray(payload, payloadSize);

    cout << "From Client : " << test.msg() << endl;
}

shared_ptr<SendBuffer> ServerPacketHandler::MakeTest(string msg)
{
    Protocol::Test test;
    test.set_msg(msg);

    return MakeSendBuffer(PacketType::Test, &test);
}
