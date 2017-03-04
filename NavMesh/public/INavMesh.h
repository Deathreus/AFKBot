#ifndef __war3source_inavmash_h__
#define __war3source_inavmash_h__

#include "IList.h"
#include "INavMeshPlace.h"
#include "INavMeshArea.h"
#include "INavMeshLadder.h"
#include "INavMeshGrid.h"


class INavMesh
{
public:
	virtual unsigned int GetMagicNumber() = 0;
	virtual unsigned int GetVersion() = 0;
	virtual unsigned int GetSubVersion() = 0;
	virtual unsigned int GetSaveBSPSize() = 0;
	virtual bool IsMeshAnalyzed() = 0;
	virtual IList<INavMeshPlace*> *GetPlaces() = 0;
	virtual IList<INavMeshArea*> *GetAreas() = 0;
	virtual IList<INavMeshLadder*> *GetLadders() = 0;
	virtual INavMeshGrid *GetGrid() = 0;
	virtual int WorldToGridX(float fWX) = 0;
	virtual int WorldToGridY(float FWY) = 0;
	virtual IList<INavMeshArea*> *GetAreasOnGrid(int x, int y) = 0;
	virtual INavMeshArea *GetArea(const Vector &vPos, float fBeneathLimit = 120.0f) = 0;
	virtual INavMeshArea *GetAreaByID(const unsigned int iAreaIndex) = 0;
};

#endif