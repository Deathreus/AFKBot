#ifndef __war3source_inavmeshencounterpath_h__
#define __war3source_inavmeshencounterpath_h__

#include "..\NavDirType.h"
#include "IList.h"
#include "INavMeshEncounterSpot.h"

class INavMeshEncounterPath 
{
public:
	virtual unsigned int GetFromAreaID() = 0;
	virtual eNavDir GetFromDirection() = 0;
	virtual unsigned int GetToAreaID() = 0;
	virtual eNavDir GetToDirection() = 0;
	virtual IList<INavMeshEncounterSpot*> *GetEncounterSpots() = 0;
};

#endif