#include "pch.h"
#include "ObjectManager.h"
#include "Player.h"
#include "PacketSession.h"
#include "Utils.h"

shared_ptr<Player> ObjectManager::CreatePlayer(weak_ptr<PacketSession> packetSession)
{
    static atomic<uint64> s_playerId = 1;
    return make_shared<Player>(s_playerId.fetch_add(1), packetSession);
}
