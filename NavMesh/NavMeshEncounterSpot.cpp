#include "NavMeshEncounterSpot.h"


CNavMeshEncounterSpot::CNavMeshEncounterSpot(unsigned int orderID, float parametricDistance)
{
	this->orderID = orderID;
	this->parametricDistance = parametricDistance;
}

unsigned int CNavMeshEncounterSpot::GetOrderID() { return this->orderID; }

float CNavMeshEncounterSpot::GetParametricDistance() { return this->parametricDistance; }