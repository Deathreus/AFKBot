#ifndef __war3source_inavmeshencounterpath_h__
#define __war3source_inavmeshencounterpath_h__

#include "..\NavDirType.h"
#include "List.h"
#include "INavMeshEncounterSpot.h"

class INavMeshEncounterPath 
{
public:
	virtual void Destroy() = 0;

	virtual unsigned int GetFromAreaID() = 0;
	virtual eNavDir GetFromDirection() = 0;
	virtual unsigned int GetToAreaID() = 0;
	virtual eNavDir GetToDirection() = 0;

	virtual CList<INavMeshEncounterSpot*> &GetEncounterSpots() = 0;
};

#endif