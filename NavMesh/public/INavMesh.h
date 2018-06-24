#ifndef __war3source_inavmash_h__
#define __war3source_inavmash_h__

#include "List.h"
#include "INavMeshPlace.h"
#include "INavMeshArea.h"
#include "INavMeshLadder.h"
#include "INavMeshHint.h"
#include "INavMeshGrid.h"


class Vector;

class INavMesh
{
public:
	virtual unsigned int GetMagicNumber() = 0;
	virtual unsigned int GetVersion() = 0;
	virtual unsigned int GetSubVersion() = 0;
	virtual unsigned int GetSaveBSPSize() = 0;
	virtual bool IsMeshAnalyzed() = 0;
	virtual bool HasUnnamedAreas() = 0;
	virtual CList<INavMeshPlace*> &GetPlaces() = 0;
	virtual CList<INavMeshArea*> &GetAreas() = 0;
	virtual CList<INavMeshLadder*> &GetLadders() = 0;
	virtual void AddHint(INavMeshHint *hint) = 0;
	virtual void AddHint(const Vector pos, const float yaw, const unsigned char flags) = 0;
	virtual bool RemoveHint(const Vector &vPos) = 0;
	virtual CList<INavMeshHint*> &GetHints() = 0;
	virtual INavMeshGrid *GetGrid() = 0;
	virtual int WorldToGridX(float fWX) = 0;
	virtual int WorldToGridY(float FWY) = 0;
	virtual Vector GridToWorld(int gridX, int gridY) = 0;
	virtual CList<INavMeshArea*> GetAreasOnGrid(int x, int y) = 0;
	virtual INavMeshArea *GetArea(const Vector &vPos, float fBeneathLimit = 120.0f) = 0;
	virtual INavMeshArea *GetAreaByID(const unsigned int iAreaIndex) = 0;
};

#endif
