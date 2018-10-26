#include "NavMeshHidingSpot.h"


CNavMeshHidingSpot::CNavMeshHidingSpot(unsigned int id, float x, float y, float z, unsigned char flags)
{
	this->id = id;
	this->pos = Vector(x, y, z);
	this->flags = flags;
}

unsigned int CNavMeshHidingSpot::GetID() { return this->id; }

float CNavMeshHidingSpot::GetX() { return this->pos.x; }

float CNavMeshHidingSpot::GetY() { return this->pos.y; }

float CNavMeshHidingSpot::GetZ() { return this->pos.z; }

unsigned char CNavMeshHidingSpot::GetFlags() { return this->flags; }