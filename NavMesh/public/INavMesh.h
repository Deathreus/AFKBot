#ifndef __war3source_inavmesh_h__
#define __war3source_inavmesh_h__

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
	virtual const CList<INavMeshPlace*> *GetPlaces() = 0;
	virtual const CList<INavMeshArea*> *GetAreas() = 0;
	virtual const CList<INavMeshLadder*> *GetLadders() = 0;
	virtual void AddHint(INavMeshHint *) = 0;
	virtual void AddHint(const Vector pos, const float yaw, const unsigned char flags) = 0;
	virtual bool RemoveHint(const Vector &) = 0;
	virtual const CList<INavMeshHint*> *GetHints() = 0;
	virtual INavMeshGrid *GetGrid() = 0;
	virtual int WorldToGridX(float) = 0;
	virtual int WorldToGridY(float) = 0;
	virtual Vector GridToWorld(int, int) = 0;
	virtual const CList<INavMeshArea*> *GetAreasOnGrid(int, int) = 0;
	virtual INavMeshArea *GetArea(const Vector &vPos, float fBeneathLimit = 120.0f) = 0;
	virtual INavMeshArea *GetNearestArea(const Vector &vPos, bool bAnyZ = false, float fMaxDist = 10000.0f, bool bCheckLOS = false, bool bCheckGround = true, int iTeam = -2) = 0;
	virtual INavMeshArea *GetAreaByID(const unsigned int iAreaIndex) = 0;
	virtual Vector GetClosestPointOnArea(INavMeshArea *, const Vector &) = 0;
};

#endif
