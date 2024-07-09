#include "pch.h"
#include "ObjectManager.h"
#include "Player.h"
#include "Session.h"
#include "Utils.h"

atomic<uint32> ObjectManager::_sObjectId = 0;

shared_ptr<Player> ObjectManager::CreatePlayer(shared_ptr<PacketSession> session)
{
    const uint32 objectId = _sObjectId.fetch_add(1);
    return make_shared<Player>(objectId, session);
}
