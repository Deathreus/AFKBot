#ifndef __war3source_inavmeshencounterspot_h__
#define __war3source_inavmeshencounterspot_h__

class INavMeshEncounterSpot
{
public:
	virtual unsigned int GetOrderID() = 0;
	virtual float GetParametricDistance() = 0;
};

#endif