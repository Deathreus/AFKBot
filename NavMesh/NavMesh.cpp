#include "NavMesh.h"


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
	INavMeshGrid *grid = GetGrid();
	int x = (int)((fWX - grid->GetExtentLowX()) / 300.0f);

	if (x < 0)
		x = 0;
	else if (x >= grid->GetGridSizeX())
		x = grid->GetGridSizeX() - 1;

	return x;
}

int CNavMesh::WorldToGridY(float fWY)
{
	INavMeshGrid *grid = GetGrid();
	int y = (int)((fWY - grid->GetExtentLowY()) / 300.0f);

	if (y < 0)
		y = 0;
	else if (y >= grid->GetGridSizeY())
		y = grid->GetGridSizeY() - 1;

	return y;
}