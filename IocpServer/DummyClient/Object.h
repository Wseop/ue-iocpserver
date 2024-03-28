#pragma once

#include "Struct.pb.h"

class Object
{
public:
	Object(uint32 objectId, Protocol::ObjectType objectType);
	virtual ~Object();

public:
	Protocol::ObjectInfo* GetObjectInfo() const { return _objectInfo; }
	uint32 GetObjectId() const { return _objectInfo->object_id(); }

	Protocol::PosInfo* GetPosInfo() const { return _posInfo; }
	void SetPosInfo(const Protocol::PosInfo& posInfo);
	double GetX() const { return _posInfo->x(); }
	void SetX(const double x) { _posInfo->set_x(x); }
	double GetY() const { return _posInfo->y(); }
	void SetY(const double y) { _posInfo->set_y(y); }
	double GetZ() const { return _posInfo->z(); }
	void SetZ(const double z) { _posInfo->set_z(z); }
	double GetYaw() const { return _posInfo->yaw(); }
	void SetYaw(const double yaw) { _posInfo->set_yaw(yaw); }
	Protocol::MoveState GetMoveState() const { return _posInfo->move_state(); }
	void SetMoveState(const Protocol::MoveState moveState) { _posInfo->set_move_state(moveState); }

private:
	Protocol::ObjectInfo* _objectInfo = nullptr;
	Protocol::PosInfo* _posInfo = nullptr;
};

