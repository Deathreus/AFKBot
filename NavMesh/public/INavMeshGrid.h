#ifndef __war3source_inavmeshgrid_h__
#define __war3source_inavmeshgrid_h__

#include "../extension.h"
#include "../core/logic/CellArray.h"


enum eGridAreas
{
	NavMeshGrid_ListStartIndex,
	NavMeshGrid_ListEndIndex,
	NavMeshGrid_MaxStats
};

enum eGridList
{
	NavMeshGridList_AreaIndex,
	NavMeshGridList_Owner,
	NavMeshGridList_MaxStats
};

class INavMeshGrid
{
public:
	virtual float GetExtentLowX() = 0;
	virtual float GetExtentLowY() = 0;
	virtual float GetExtentHighX() = 0;
	virtual float GetExtentHighY() = 0;
	virtual int GetGridSizeX() = 0;
	virtual int GetGridSizeY() = 0;
	virtual ICellArray *GetGridList() = 0;
	virtual ICellArray *GetGridAreas() = 0;
	virtual void GetGridListBounds(int iGridIndex, int *iStartIndex, int *iEndIndex) = 0;
};

#endif