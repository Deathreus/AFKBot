#pragma once

#include "public/INavMeshGrid.h"
#include "public/INavMeshArea.h"
#include "List.h"

#include "../extension.h"


// TODO: Figure out if this ever changes per game
const float GridCellSize = 300.0f; /*< defines the extent for a single grid block, used for spatial partitioning*/

typedef CList<CList<INavMeshArea*>> GridList_t;

class CNavMeshGrid : public INavMeshGrid
{
public:
	CNavMeshGrid(Vector2D low, Vector2D high, int x, int y, GridList_t list);
	~CNavMeshGrid() {}

	void Destroy();

	Vector2D GetExtentLow();
	Vector2D GetExtentHigh();

	int GetGridSizeX();
	int GetGridSizeY();

	CList<CList<INavMeshArea*>> GetGridLists();
	CList<INavMeshArea*> GetGridAreas(int index);

private:
	Vector2D extentLo;
	Vector2D extentHi;

	int gridSizeX;
	int gridSizeY;

	GridList_t grid;
};