#ifndef __war3source_navmeshencounterspot_h__
#define __war3source_navmeshencounterspot_h__

#include "public\INavMeshEncounterSpot.h"


class CNavMeshEncounterSpot : public INavMeshEncounterSpot
{
public:
	CNavMeshEncounterSpot(unsigned int orderID, float parametricDistance);
	~CNavMeshEncounterSpot() {}

	inline void Destroy() { delete this; }

	unsigned int GetOrderID();
	float GetParametricDistance();

private:
	unsigned int orderID;
	float parametricDistance;
};

#endif