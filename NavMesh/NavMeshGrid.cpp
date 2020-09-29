#include "List.h"
#include "NavMeshGrid.h"


CNavMeshGrid::CNavMeshGrid(Vector2D const &low, Vector2D const &high, int x, int y, const CList<CList<INavMeshArea*>> &list)
{
	this->extentLo = low;
	this->extentHi = high;
	this->gridSizeX = x;
	this->gridSizeY = y;
	this->grid = list;
}

void CNavMeshGrid::Destroy()
{
	for (auto list : this->grid)
	{
		list.Clear();
	}

	delete this;
}

const Vector2D &CNavMeshGrid::GetExtentLow() { return this->extentLo; }

const Vector2D &CNavMeshGrid::GetExtentHigh() { return this->extentHi; }

int CNavMeshGrid::GetGridSizeX() { return this->gridSizeX; }

int CNavMeshGrid::GetGridSizeY() { return this->gridSizeY; }

const CList<INavMeshArea*> *CNavMeshGrid::GetGridAreas(int index) { return &this->grid[index]; }

const CList<CList<INavMeshArea*>> *CNavMeshGrid::GetGridLists() { return &this->grid; }