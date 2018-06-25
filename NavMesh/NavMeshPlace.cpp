#include "NavMeshPlace.h"
#include <amtl/am-string.h>


CNavMeshPlace::CNavMeshPlace(unsigned int id, const char *name)
{
	this->id = id;
	ke::SafeStrcpy(this->name, sizeof(name), name);
}

const char *CNavMeshPlace::GetName() { return this->name; }

unsigned int CNavMeshPlace::GetID() { return this->id; }