#include "List.h"
#include "NavMesh.h"
#include "NavMeshArea.h"
#include "NavMeshHint.h"
#include "NavMeshGrid.h"

#include "../extension.h"


constexpr float StepHeight = 18.0f;
constexpr float HalfHumanHeight = 36.0f;


int CNavMesh::m_iHintCount = 0;

CNavMesh::CNavMesh(unsigned int magicNumber, unsigned int version, unsigned int subVersion, unsigned int saveBSPSize, bool isMeshAnalyzed, bool hasUnnamedAreas,
	const CList<INavMeshPlace*> &places, const CList<INavMeshArea*> &areas, const CList<INavMeshHint*> &hints, const CList<INavMeshLadder*> &ladders, INavMeshGrid *grid)
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
	for (auto area : this->areas)
	{
		if (area) area->Destroy();
	}

	this->places.Clear();
	this->areas.Clear();
	this->hints.Clear();
	this->ladders.Clear();

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

const CList<INavMeshPlace*> *CNavMesh::GetPlaces() { return &this->places; }

const CList<INavMeshArea*> *CNavMesh::GetAreas() { return &this->areas; }

const CList<INavMeshLadder*> *CNavMesh::GetLadders() { return &this->ladders; }

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
	if (size <= 0)
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

const CList<INavMeshHint*> *CNavMesh::GetHints() { return &this->hints; }

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

const CList<INavMeshArea*> *CNavMesh::GetAreasOnGrid(int x, int y)
{
	if (!this->grid)
	{
		smutils->LogError(myself, "Could not get areas because the grid doesn't exist!");
		return nullptr;
	}

	int index = x + y * this->grid->GetGridSizeX();
	CList<INavMeshArea*> const *areas = this->grid->GetGridAreas(index);
	if (!areas || areas->Count() <= 0)
		return nullptr;

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

	const CList<INavMeshArea *> *areas = this->GetAreasOnGrid(x, y);

	INavMeshArea *useArea = nullptr;
	float fUseZ = -99999999.9f;
	Vector vTestPos = vPos + Vector(0.0f, 0.0f, 5.0f);

	for( INavMeshArea *area : *areas )
	{
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

INavMeshArea *CNavMesh::GetNearestArea(const Vector &vPos, bool bAnyZ, float fMaxDist, bool bCheckLOS, bool bCheckGround, int iTeam)
{
	if(!this->grid)
	{
		smutils->LogError(myself, "Can't retrieve area because the grid doesn't exist!");
		return nullptr;
	}

	INavMeshArea *closest = NULL;
	if(!bCheckLOS && !bCheckGround)
	{
		closest = this->GetArea(vPos);
		if(closest) return closest;
	}

	extern IEngineTrace *engtrace;
	static CTraceFilterWorldOnly filter;

	float fClosest = fMaxDist * fMaxDist;

	Vector vDest;
	vDest.Init(vPos.x, vPos.y, vPos.z - 1e4f);

	Ray_t ray; trace_t result;
	ray.Init(vPos, vDest);
	engtrace->TraceRay(ray, MASK_PLAYERSOLID, &filter, &result);

	if(result.startsolid && bCheckGround)
		return nullptr;

	vDest.z = result.endpos.z + HalfHumanHeight;

	CList<INavMeshArea*> collected;

	const int iGridX = this->WorldToGridX(vPos.x);
	const int iGridY = this->WorldToGridY(vPos.y);

	int iShiftLimit = Ceil2Int(fMaxDist / GridCellSize);
	for(int iShift = 0; iShift <= iShiftLimit; iShift++)
	{
		for(int x = (iGridX - iShift); x <= (iGridX + iShift); x++)
		{
			if(x < 0 || x > this->grid->GetGridSizeX())
				continue;

			for(int y = (iGridY - iShift); y <= (iGridY + iShift); y++)
			{
				if(y < 0 || y > this->grid->GetGridSizeY())
					continue;

				if(x > (iGridX - iShift) 
				&& x < (iGridX + iShift) 
				&& y >(iGridY - iShift) 
				&& y < (iGridY + iShift))
				{
					continue;
				}

				const CList<INavMeshArea *> *areas = this->GetAreasOnGrid(x, y);

				for( INavMeshArea *area : *areas )
				{
					if(collected.Find(area) != collected.InvalidIndex())
						continue;	// we've already visited this area

					collected.AddToTail(area);

					const Vector vAreaPos = GetClosestPointOnArea(area, vDest);
					const float fDistSqr = (vAreaPos - vDest).LengthSqr();
					if(fDistSqr < fClosest)
					{
						if(bCheckLOS)
						{
							ray.Init(vDest, vAreaPos + Vector(0.0f, 0.0f, HalfHumanHeight));
							engtrace->TraceRay(ray, MASK_PLAYERSOLID_BRUSHONLY, &filter, &result);

							Vector vSafePos;
							if(result.fraction <= 0.0f)
							{
								vSafePos = result.endpos + Vector(0.0f, 0.0f, 1.0f);
							}
							else
							{
								vSafePos = vPos;
							}

							const float fHeightDelta = fabs(vAreaPos.z - vSafePos.z);
							if(fHeightDelta > StepHeight)
							{
								ray.Init(Vector(vAreaPos.x, vAreaPos.y, vAreaPos.z + StepHeight), Vector(vAreaPos.x, vAreaPos.y, vSafePos.z));
								engtrace->TraceRay(ray, MASK_PLAYERSOLID_BRUSHONLY, &filter, &result);

								if(result.fraction < 1.0f)
								{
									continue;
								}
							}
						}

						fClosest = fDistSqr;
						closest = area;
						iShiftLimit = iShift + 1;
					}
				}
			}
		}
	}

	collected.Clear();

	return closest;
}

INavMeshArea *CNavMesh::GetAreaByID(const unsigned int iAreaIndex)
{
	if (this->areas.Count() <= 0)
	{
		smutils->LogError(myself, "Can't retrieve area because there are none!");
		return nullptr;
	}

	for (INavMeshArea *area : this->areas)
	{
		if (area->GetID() == iAreaIndex)
			return area;
	}

	return nullptr;
}

Vector CNavMesh::GetClosestPointOnArea(INavMeshArea *area, const Vector &vPos)
{
	if(area == nullptr)
		return vPos;

	const Vector vExtLo = area->GetExtentLow();
	const Vector vExtHi = area->GetExtentHigh();
	Vector vClosest(0.0f);

	if(vPos.x < vExtLo.x)
	{
		if(vPos.y < vExtLo.y)
		{
			// position is north-west of area
			return vExtLo;
		}
		else if(vPos.y > vExtHi.y)
		{
			// position is south-west of area
			vClosest.x = vExtLo.x;
			vClosest.y = vExtHi.y;
		}
		else
		{
			// position is west of area
			vClosest.x = vExtLo.x;
			vClosest.y = vPos.y;
		}
	}
	else if(vPos.x > vExtHi.x)
	{
		if(vPos.y < vExtLo.y)
		{
			// position is north-east of area
			vClosest.x = vExtHi.x;
			vClosest.y = vExtLo.y;
		}
		else if(vPos.y > vExtHi.y)
		{
			// position is south-east of area
			vClosest = vExtHi;
		}
		else
		{
			// position is east of area
			vClosest.x = vExtHi.x;
			vClosest.y = vPos.y;
		}
	}
	else if(vPos.y < vExtLo.y)
	{
		// position is north of area
		vClosest.x = vPos.x;
		vClosest.y = vExtLo.y;
	}
	else if(vPos.y > vExtHi.y)
	{
		// position is south of area
		vClosest.x = vPos.x;
		vClosest.y = vExtHi.y;
	}
	else
	{
		// position is inside of area - it is the 'closest point' to itself
		vClosest = vPos;
	}

	vClosest.z = area->GetZ(vClosest);
	return vClosest;
}