#include "NavMeshEncounterPath.h"
#include "List.h"


CNavMeshEncounterPath::CNavMeshEncounterPath(unsigned int fromAreaID, eNavDir fromDirection,
	unsigned int toAreaID, eNavDir toDirection, const CList<INavMeshEncounterSpot*> encounterSpots)
{
	this->fromAreaID = fromAreaID;
	this->fromDirection = fromDirection;
	this->toAreaID = toAreaID;
	this->toDirection = toDirection;
	this->encounterSpots = encounterSpots;
}

void CNavMeshEncounterPath::Destroy()
{
	this->encounterSpots.Clear();

	delete this;
}

unsigned int CNavMeshEncounterPath::GetFromAreaID() { return this->fromAreaID; }

eNavDir CNavMeshEncounterPath::GetFromDirection() { return this->fromDirection; }

unsigned int CNavMeshEncounterPath::GetToAreaID() { return this->toAreaID; }

eNavDir CNavMeshEncounterPath::GetToDirection() { return this->toDirection; }

CList<INavMeshEncounterSpot*> *CNavMeshEncounterPath::GetEncounterSpots() { return &this->encounterSpots; }