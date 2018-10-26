#ifndef __war3source_navmeshgrid_h__
#define __war3source_navmeshgrid_h__

#include "public/INavMeshGrid.h"
#include "public/INavMeshArea.h"
#include "List.h"

#include "../extension.h"


// TODO: Figure out if this ever changes per game
const float GridCellSize = 300.0f; /*< defines the extent for a single grid block, used for spatial partitioning*/

class CNavMeshGrid : public INavMeshGrid
{
public:
	CNavMeshGrid(Vector2D low, Vector2D high, int x, int y, const CList<CList<INavMeshArea*>> list);

	void Destroy();

	Vector2D GetExtentLow();
	Vector2D GetExtentHigh();

	int GetGridSizeX();
	int GetGridSizeY();

	CList<CList<INavMeshArea*>> *GetGridLists();
	CList<INavMeshArea*> *GetGridAreas(int index);

protected:
	~CNavMeshGrid() { }

private:
	Vector2D extentLo;
	Vector2D extentHi;

	int gridSizeX;
	int gridSizeY;

	CList<CList<INavMeshArea*>> grid;
};

#endif