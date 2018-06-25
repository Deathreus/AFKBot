#include "NavMeshHidingSpot.h"


CNavMeshHidingSpot::CNavMeshHidingSpot(unsigned int id, float x, float y, float z, unsigned char flags)
{
	this->id = id;
	this->x = x;
	this->y = y;
	this->z = z;
	this->flags = flags;
}

unsigned int CNavMeshHidingSpot::GetID() { return this->id; }

float CNavMeshHidingSpot::GetX() { return this->x; }

float CNavMeshHidingSpot::GetY() { return this->y; }

float CNavMeshHidingSpot::GetZ() { return this->z; }

unsigned char CNavMeshHidingSpot::GetFlags() { return this->flags; }