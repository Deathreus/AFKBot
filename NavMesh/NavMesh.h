#ifndef __war3source_navmesh_h__
#define __war3source_navmesh_h__

#include "List.h"
#include "public\INavMesh.h"
#include "public\INavMeshPlace.h"
#include "public\INavMeshLadder.h"
#include "public\INavMeshHint.h"
#include "public\INavMeshArea.h"
#include "public\INavMeshGrid.h"


class Vector;

class CNavMesh : public INavMesh
{
public:
	CNavMesh(unsigned int magicNumber, unsigned int version, unsigned int subVersion, unsigned int saveBSPSize, bool isMeshAnalyzed, bool hasUnnamedAreas,
		const CList<INavMeshPlace*> places, const CList<INavMeshArea*> areas, const CList<INavMeshHint*> hints, const CList<INavMeshLadder*> ladders, INavMeshGrid *grid);
	~CNavMesh();

	unsigned int GetMagicNumber();

	unsigned int GetVersion();
	unsigned int GetSubVersion();

	unsigned int GetSaveBSPSize();

	bool IsMeshAnalyzed();

	bool HasUnnamedAreas();

	CList<INavMeshPlace*> &GetPlaces();
	CList<INavMeshArea*> &GetAreas();
	CList<INavMeshLadder*> &GetLadders();

	void AddHint(INavMeshHint *hint);
	void AddHint(const Vector pos, const float yaw, const unsigned char flags);
	bool RemoveHint(const Vector &vPos);
	CList<INavMeshHint*> &GetHints();

	INavMeshGrid *GetGrid();
	int WorldToGridX(float fWX);
	int WorldToGridY(float fWY);
	Vector GridToWorld(int gridX, int gridY);

	CList<INavMeshArea*> GetAreasOnGrid(int x, int y);

	INavMeshArea *GetArea(const Vector &vPos, float fBeneathLimit = 120.0f);
	INavMeshArea *GetAreaByID(const unsigned int iAreaIndex);

	static int m_iHintCount;

private:
	unsigned int magicNumber;
	unsigned int version;
	unsigned int subVersion;
	unsigned int saveBSPSize;
	bool isMeshAnalyzed;
	bool hasUnnamedAreas;
	CList<INavMeshPlace*> places;
	CList<INavMeshArea*> areas;
	CList<INavMeshLadder*> ladders;
	CList<INavMeshHint*> hints;
	INavMeshGrid *grid;
};

#endif
