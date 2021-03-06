#ifndef __war3source_navmeshencounterpath_h__
#define __war3source_navmeshencounterpath_h__

#include "public/INavMeshEncounterPath.h"
#include "public/INavMeshEncounterSpot.h"
#include "NavDirType.h"


class CNavMeshEncounterPath : public INavMeshEncounterPath 
{
public:
	CNavMeshEncounterPath(unsigned int fromAreaID, eNavDir fromDirection, unsigned int toAreaID, eNavDir toDirection, const CList<INavMeshEncounterSpot*> &encounterSpots);

	void Destroy();

	unsigned int GetFromAreaID();
	eNavDir GetFromDirection();
	unsigned int GetToAreaID();
	eNavDir GetToDirection();
	const CList<INavMeshEncounterSpot*> *GetEncounterSpots();

protected:
	~CNavMeshEncounterPath() { }

private:
	unsigned int fromAreaID;
	eNavDir fromDirection;
	unsigned int toAreaID;
	eNavDir toDirection;
	CList<INavMeshEncounterSpot*> encounterSpots;
};

#endif