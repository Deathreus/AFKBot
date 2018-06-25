#include "List.h"
#include "NavMeshGrid.h"


CNavMeshGrid::CNavMeshGrid(Vector2D low, Vector2D high, int x, int y, GridList_t list)
{
	this->extentLo = low;
	this->extentHi = high;
	this->gridSizeX = x;
	this->gridSizeY = y;
	this->grid = list;
}

void CNavMeshGrid::Destroy()
{
	ForEachItem(this->grid, it)
	{
		CList<INavMeshArea*> list = grid.Element(it);
		list.Clear(true);
	}

	this->grid.Clear(false);

	delete this;
}

Vector2D CNavMeshGrid::GetExtentLow() { return this->extentLo; }

Vector2D CNavMeshGrid::GetExtentHigh() { return this->extentHi; }

int CNavMeshGrid::GetGridSizeX() { return this->gridSizeX; }

int CNavMeshGrid::GetGridSizeY() { return this->gridSizeY; }

CList<INavMeshArea*> CNavMeshGrid::GetGridAreas(int index) { return this->grid[index]; }

CList<CList<INavMeshArea*>> CNavMeshGrid::GetGridLists() { return this->grid; }