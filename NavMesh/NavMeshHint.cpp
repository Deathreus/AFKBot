#include "NavMeshHint.h"

CNavMeshHint::CNavMeshHint(unsigned int id, float x, float y, float z, float yaw, unsigned char flags)
{
	this->id = id;
	this->pos = Vector(x, y, z);
	this->yaw = yaw;
	this->flags = flags;
}

unsigned int CNavMeshHint::GetID() { return this->id; }

float CNavMeshHint::GetX() { return this->pos.x; }

float CNavMeshHint::GetY() { return this->pos.y; }

float CNavMeshHint::GetZ() { return this->pos.z; }

const Vector CNavMeshHint::GetPos() { return this->pos; }

float CNavMeshHint::GetYaw() { return this->yaw; }

unsigned char CNavMeshHint::GetFlags() { return this->flags; }