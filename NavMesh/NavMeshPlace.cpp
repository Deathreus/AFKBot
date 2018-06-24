#include "NavMeshPlace.h"
#include <iostream>


CNavMeshPlace::CNavMeshPlace(unsigned int id, const char *name)
{
	this->id = id;
	strcpy_s(this->name, sizeof(name), name);
}

const char *CNavMeshPlace::GetName() { return this->name; }

unsigned int CNavMeshPlace::GetID() { return this->id; }
