#include "List.h"
#include "NavMesh.h"
#include "NavMeshArea.h"
#include "NavMeshHint.h"
#include "NavMeshGrid.h"

#include "../extension.h"


int CNavMesh::m_iHintCount = 0;

CNavMesh::CNavMesh(unsigned int magicNumber, unsigned int version, unsigned int subVersion, unsigned int saveBSPSize, bool isMeshAnalyzed, bool hasUnnamedAreas,
	const CList<INavMeshPlace*> places, const CList<INavMeshArea*> areas, const CList<INavMeshHint*> hints, const CList<INavMeshLadder*> ladders, INavMeshGrid *grid)
{
	this->magicNumber = magicNumber;
	this->version = version;
	this->subVersion = subVersion;
	this->saveBSPSize = saveBSPSize;
	this->isMeshAnalyzed = isMeshAnalyzed;
	this->hasUnnamedAreas = hasUnnamedAreas;
	this->places = places;
	this->areas = areas;
	this->hints = hints;
	this->ladders = ladders;
	this->grid = grid;
}

CNavMesh::~CNavMesh()
{
	ForEachItem(this->areas, it)
	{
		INavMeshArea *area = this->areas.Element(it);
		if (area) area->Destroy();
	}

	this->places.PurgeAndDeleteElements();
	this->areas.PurgeAndDeleteElements();
	this->hints.PurgeAndDeleteElements();
	this->ladders.PurgeAndDeleteElements();

	this->grid->Destroy();
	delete this->grid;

	CNavMesh::m_iHintCount = 0;

	CNavMeshArea::m_iMasterMarker = 0;
	CNavMeshArea::m_OpenList = nullptr;
}

unsigned int CNavMesh::GetMagicNumber() { return this->magicNumber; }

unsigned int CNavMesh::GetVersion() { return this->version; }

unsigned int CNavMesh::GetSubVersion() { return this->subVersion; }

unsigned int CNavMesh::GetSaveBSPSize() { return this->saveBSPSize; }

bool CNavMesh::IsMeshAnalyzed() { return this->isMeshAnalyzed; }

bool CNavMesh::HasUnnamedAreas() { return this->hasUnnamedAreas; }

CList<INavMeshPlace*> CNavMesh::GetPlaces() { return this->places; }

CList<INavMeshArea*> CNavMesh::GetAreas() { return this->areas; }

CList<INavMeshLadder*> CNavMesh::GetLadders() { return this->ladders; }

void CNavMesh::AddHint(INavMeshHint *hint)
{
	m_iHintCount = hint->GetID();
	this->hints.AddToTail(hint);
}

void CNavMesh::AddHint(const Vector pos, const float yaw, const unsigned char flags)
{
	float clampedYaw = clamp(yaw, -180.0f, 180.0f);
	this->hints.AddToTail(new CNavMeshHint(++m_iHintCount, pos.x, pos.y, pos.z, clampedYaw, flags));
}

bool CNavMesh::RemoveHint(const Vector &vPos)
{
	size_t size = this->hints.Size();
	if (!this->hints || size <= 0)
		return false;

	for (unsigned int i = 0; i < size; i++)
	{
		INavMeshHint *hint = this->hints.Element(i);
		if (hint)
		{
			Vector vTest(hint->GetX(), hint->GetY(), hint->GetZ());
			if ((vPos - vTest).IsLengthLessThan(6.0f))
			{
				return this->hints.FindAndRemove(hint);
			}
		}
	}

	return false;
}

CList<INavMeshHint*> CNavMesh::GetHints() { return this->hints; }

INavMeshGrid *CNavMesh::GetGrid() { return this->grid; }

int CNavMesh::WorldToGridX(float fWX)
{
	return clamp((int)((fWX - this->grid->GetExtentLow()[0]) / GridCellSize), 0, this->grid->GetGridSizeX() - 1);
}

int CNavMesh::WorldToGridY(float fWY)
{
	return clamp((int)((fWY - this->grid->GetExtentLow()[1]) / GridCellSize), 0, this->grid->GetGridSizeY() - 1);
}

Vector CNavMesh::GridToWorld(int gridX, int gridY)
{
	gridX = clamp(gridX, 0, this->grid->GetGridSizeX() - 1);
	gridY = clamp(gridY, 0, this->grid->GetGridSizeY() - 1);

	float posX = this->grid->GetExtentLow()[0] + gridX * GridCellSize;
	float posY = this->grid->GetExtentLow()[1] + gridY * GridCellSize;

	return Vector(posX, posY, 0.0f);
}

CList<INavMeshArea*> CNavMesh::GetAreasOnGrid(int x, int y)
{
	if (!this->grid)
	{
		smutils->LogError(myself, "Could not get areas because the grid doesn't exist!");
		return CList<INavMeshArea*>(0);
	}

	int index = x + y * this->grid->GetGridSizeX();
	CList<INavMeshArea*> areas = this->grid->GetGridAreas(index);
	if (!areas || areas.Size() <= 0)
		return CList<INavMeshArea*>(0);

	return areas;
}

INavMeshArea *CNavMesh::GetArea(const Vector &vPos, float fBeneathLimit)
{
	if (!this->grid)
	{
		smutils->LogError(myself, "Can't retrieve area because the grid doesn't exist!");
		return nullptr;
	}

	int x = this->WorldToGridX(vPos.x);
	int y = this->WorldToGridY(vPos.y);

	CList<INavMeshArea*> &areas = this->GetAreasOnGrid(x, y);

	INavMeshArea *useArea = nullptr;
	float fUseZ = -99999999.9f;
	Vector vTestPos = vPos + Vector(0.0f, 0.0f, 5.0f);

	while (!areas.Empty())
	{
		INavMeshArea *area = areas.Pop();

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

	return useArea;
}

INavMeshArea *CNavMesh::GetAreaByID(const unsigned int iAreaIndex)
{
	size_t size = this->areas.Size();
	if (!this->areas || size <= 0)
	{
		smutils->LogError(myself, "Can't retrieve area because there are none!");
		return nullptr;
	}

	for (unsigned int i = 0; i < size; i++)
	{
		INavMeshArea *area = this->areas.Element(i);
		if (area)
		{
			if (area->GetID() == iAreaIndex)
				return area;
		}
	}

	return nullptr;
}