#include "pch.h"
#include "ObjectManager.h"
#include "Player.h"
#include "PacketSession.h"
#include "Utils.h"

atomic<uint32> ObjectManager::_sObjectId = 1;

shared_ptr<Player> ObjectManager::CreatePlayer(weak_ptr<Session> session)
{
    if (session.lock() == nullptr)
        return nullptr;

    const uint32 objectId = _sObjectId.fetch_add(1);
    return make_shared<Player>(objectId, session);
}
