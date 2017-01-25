#include "NavMeshEncounterPath.h"


CNavMeshEncounterPath::CNavMeshEncounterPath(unsigned int fromAreaID, eNavDir fromDirection,
	unsigned int toAreaID, eNavDir toDirection, IList<INavMeshEncounterSpot*> *encounterSpots)
{
	this->fromAreaID = fromAreaID;
	this->fromDirection = fromDirection;
	this->toAreaID = toAreaID;
	this->toDirection = toDirection;
	this->encounterSpots = encounterSpots;
}

CNavMeshEncounterPath::~CNavMeshEncounterPath() {}

unsigned int CNavMeshEncounterPath::GetFromAreaID() { return this->fromAreaID; }

eNavDir CNavMeshEncounterPath::GetFromDirection() { return this->fromDirection; }

unsigned int CNavMeshEncounterPath::GetToAreaID() { return this->toAreaID; }

eNavDir CNavMeshEncounterPath::GetToDirection() { return this->toDirection; }

IList<INavMeshEncounterSpot*> *CNavMeshEncounterPath::GetEncounterSpots() { return this->encounterSpots; }
