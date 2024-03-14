#include "pch.h"
#include "ObjectManager.h"
#include "Player.h"
#include "PacketSession.h"
#include "Utils.h"

shared_ptr<Player> ObjectManager::CreatePlayer(weak_ptr<Session> session)
{
    static atomic<uint32> sPlayerId = 1;

    if (session.lock() == nullptr)
        return nullptr;

    return make_shared<Player>(sPlayerId.fetch_add(1), session);
}
