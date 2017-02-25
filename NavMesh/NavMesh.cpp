#include "List.h"
#include "NavMesh.h"
#include "NavMeshGrid.h"
#include "../extension.h"


CNavMesh::CNavMesh(unsigned int magicNumber, unsigned int version, unsigned int subVersion, unsigned int saveBSPSize, bool isMeshAnalyzed,
	IList<INavMeshPlace*> *places, IList<INavMeshArea*> *areas, IList<INavMeshLadder*> *ladders, INavMeshGrid *grid)
{
	this->magicNumber = magicNumber;
	this->version = version;
	this->subVersion = subVersion;
	this->saveBSPSize = saveBSPSize;
	this->isMeshAnalyzed = isMeshAnalyzed;
	this->places = places;
	this->areas = areas;
	this->ladders = ladders;
	this->grid = grid;
}

CNavMesh::~CNavMesh() {}

unsigned int CNavMesh::GetMagicNumber() { return this->magicNumber; }

unsigned int CNavMesh::GetVersion() { return this->version; }

unsigned int CNavMesh::GetSubVersion() { return this->subVersion; }

unsigned int CNavMesh::GetSaveBSPSize() { return this->saveBSPSize; }

bool CNavMesh::IsMeshAnalyzed() { return this->isMeshAnalyzed; }

IList<INavMeshPlace*> *CNavMesh::GetPlaces() { return this->places; }

IList<INavMeshArea*> *CNavMesh::GetAreas() { return this->areas; }

IList<INavMeshLadder*> *CNavMesh::GetLadders() { return this->ladders; }

INavMeshGrid *CNavMesh::GetGrid() { return this->grid; }

int CNavMesh::WorldToGridX(float fWX)
{
	INavMeshGrid *grid = this->GetGrid();
	int x = (int)((fWX - grid->GetExtentLowX()) / 300.0f);

	if (x < 0)
		x = 0;
	else if (x >= grid->GetGridSizeX())
		x = grid->GetGridSizeX() - 1;

	return x;
}

int CNavMesh::WorldToGridY(float fWY)
{
	INavMeshGrid *grid = this->GetGrid();
	int y = (int)((fWY - grid->GetExtentLowY()) / 300.0f);

	if (y < 0)
		y = 0;
	else if (y >= grid->GetGridSizeY())
		y = grid->GetGridSizeY() - 1;

	return y;
}

IList<INavMeshArea*> *CNavMesh::GetAreasOnGrid(int x, int y)
{
	if (!this->isMeshAnalyzed)
	{
		smutils->LogError(myself, "Could not get grid areas because the navmesh doesn't exist!");
		return NULL;
	}

	INavMeshGrid *grid = this->GetGrid();
	if (!grid)
	{
		smutils->LogError(myself, "Could not get grid areas because the grid doesn't exist!");
		return NULL;
	}

	int iGridIndex = x + y * grid->GetGridSizeX();
	int iListStartIndex = GetArrayCell(grid->GetGridAreas(), iGridIndex, NavMeshGrid_ListStartIndex);
	int iListEndIndex = GetArrayCell(grid->GetGridAreas(), iGridIndex, NavMeshGrid_ListEndIndex);

	if (iListStartIndex == -1)
		return NULL;

	IList<INavMeshArea*> *ret = new CList<INavMeshArea*>();

	IList<INavMeshArea*> *areas = this->GetAreas();
	for (int i = iListStartIndex; i <= iListEndIndex; i++)
	{
		INavMeshArea *area = areas->At(GetArrayCell(grid->GetGridList(), i, NavMeshGridList_AreaIndex));
		if (area)
			ret->Push(area);
	}

	return ret;
}

INavMeshArea *CNavMesh::GetArea(const Vector &vPos, float fBeneathLimit)
{
	if (!this->isMeshAnalyzed)
	{
		smutils->LogError(myself, "Can't retrieve area because the navmesh doesn't exist!");
		return NULL;
	}

	INavMeshGrid *grid = this->GetGrid();
	if (!grid)
	{
		smutils->LogError(myself, "Can't retrieve area because the grid doesn't exist!");
		return NULL;
	}

	int x = this->WorldToGridX(vPos.x);
	int y = this->WorldToGridY(vPos.y);

	IList<INavMeshArea*> *areas = this->GetAreasOnGrid(x, y);

	INavMeshArea *useArea = 0;
	float fUseZ = -99999999.9f;
	Vector vTestPos = vPos + Vector(0.0f, 0.0f, 5.0f);

	if (areas)
	{
		while (areas->Size() > 0)
		{
			INavMeshArea *area = areas->Tail();
			areas->PopList();

			if (area->IsOverlapping(vPos))
			{
				float z = area->GetZ(vTestPos);

				if (z > vTestPos.z)
					continue;

				if (z < vPos.z - fBeneathLimit)
					continue;

				if (z > fUseZ)
				{
					useArea = area;
					fUseZ = z;
				}
			}
		}
	}

	return useArea;
}

INavMeshArea *CNavMesh::GetAreaByID(const unsigned int iAreaIndex)
{
	if (!this->isMeshAnalyzed)
	{
		smutils->LogError(myself, "Can't retrieve area because the navmesh doesn't exist!");
		return NULL;
	}

	IList<INavMeshArea*> *areas = this->GetAreas();
	for (unsigned int i = 0; i < areas->Size(); i++)
	{
		INavMeshArea *area = areas->At(i);
		if (area->GetID() == iAreaIndex)
			return area;
	}

	return NULL;
}