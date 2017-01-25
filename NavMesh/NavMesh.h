#ifndef __war3source_navmesh_h__
#define __war3source_navmesh_h__

#include "public\INavMesh.h"
#include "public\IList.h"
#include "public\INavMeshPlace.h"
#include "public\INavMeshLadder.h"
#include "public\INavMeshArea.h"
#include "public\INavMeshGrid.h"


class CNavMesh : public INavMesh
{
public:
	CNavMesh(unsigned int magicNumber, unsigned int version, unsigned int subVersion, unsigned int saveBSPSize, bool isMeshAnalyzed,
		IList<INavMeshPlace*> *places, IList<INavMeshArea*> *areas, IList<INavMeshLadder*> *ladders, INavMeshGrid *grid);
	~CNavMesh();

	unsigned int GetMagicNumber();
	unsigned int GetVersion();
	unsigned int GetSubVersion();
	unsigned int GetSaveBSPSize();
	bool IsMeshAnalyzed();
	IList<INavMeshPlace*> *GetPlaces();
	IList<INavMeshArea*> *GetAreas();
	IList<INavMeshLadder*> *GetLadders();
	INavMeshGrid *GetGrid();
	int WorldToGridX(float fWX);
	int WorldToGridY(float fWY);

private:
	unsigned int magicNumber;
	unsigned int version;
	unsigned int subVersion;
	unsigned int saveBSPSize;
	bool isMeshAnalyzed;
	IList<INavMeshPlace*> *places;
	IList<INavMeshArea*> *areas;
	IList<INavMeshLadder*> *ladders;
	INavMeshGrid *grid;
};

#endif