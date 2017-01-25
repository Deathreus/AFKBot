#include "NavMeshVisibleArea.h"


CNavMeshVisibleArea::CNavMeshVisibleArea(unsigned int visibleAreaID, unsigned char attributes)
{
	this->visibleAreaID = visibleAreaID;
	this->attributes = attributes;
}

CNavMeshVisibleArea::~CNavMeshVisibleArea() {}

unsigned int CNavMeshVisibleArea::GetVisibleAreaID() { return this->visibleAreaID; }

unsigned char CNavMeshVisibleArea::GetAttributes() { return this->attributes; }
