#include "pch.h"
#include "ObjectManager.h"
#include "Player.h"
#include "PacketSession.h"
#include "Utils.h"

shared_ptr<Player> ObjectManager::CreatePlayer(weak_ptr<PacketSession> packetSession)
{
    static atomic<uint64> sPlayerId = 1;

    if (packetSession.lock() == nullptr)
        return nullptr;

    return make_shared<Player>(sPlayerId.fetch_add(1), packetSession);
}
