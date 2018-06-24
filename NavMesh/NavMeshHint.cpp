#include "NavMeshHint.h"

CNavMeshHint::CNavMeshHint(unsigned int id, float x, float y, float z, float yaw, unsigned char flags)
{
	this->id = id;
	this->x = x;
	this->y = y;
	this->z = z;
	this->yaw = yaw;
	this->flags = flags;
}

unsigned int CNavMeshHint::GetID() { return this->id; }

float CNavMeshHint::GetX() { return this->x; }

float CNavMeshHint::GetY() { return this->y; }

float CNavMeshHint::GetZ() { return this->z; }

float CNavMeshHint::GetYaw() { return this->yaw; }

unsigned char CNavMeshHint::GetFlags() { return this->flags; }
