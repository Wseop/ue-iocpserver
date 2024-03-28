#include "pch.h"
#include "Object.h"

Object::Object(uint32 objectId, Protocol::ObjectType objectType) :
	_objectInfo(new Protocol::ObjectInfo()),
	_posInfo(new Protocol::PosInfo())
{
	_objectInfo->set_object_id(objectId);
	_objectInfo->set_object_type(objectType);
	_posInfo = _objectInfo->mutable_pos_info();
	_posInfo->set_move_state(Protocol::MOVE_STATE_IDLE);
}

Object::~Object()
{
	delete _objectInfo;
}

void Object::SetPosInfo(const Protocol::PosInfo& posInfo)
{
	_posInfo->CopyFrom(posInfo);
}