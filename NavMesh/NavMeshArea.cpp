#include "NavMeshArea.h"
#include "NavDirType.h"
#include "List.h"
#include "vector.h"


int CNavMeshArea::m_iMasterMarker = 0;
INavMeshArea *CNavMeshArea::m_OpenList = NULL;

CNavMeshArea::CNavMeshArea(unsigned int id, unsigned int flags, unsigned int placeID,
	float nwExtentX, float nwExtentY, float nwExtentZ,
	float seExtentX, float seExtentY, float seExtentZ,
	float neCornerZ, float swCornerZ,
	const CList<CList<INavMeshConnection*>> connections, const CList<INavMeshHidingSpot*> hidingSpots,
	const CList<INavMeshEncounterPath*> encounterPaths, const CList<CList<INavMeshLadderConnection*>> ladderConnections,
	const CList<INavMeshCornerLightIntensity*> cornerLightIntensities, const CList<INavMeshVisibleArea*> visibleAreas,
	unsigned int inheritVisibilityFromAreaID, float earliestOccupyTimeFirstTeam, float earliestOccupyTimeSecondTeam)
{
	this->id = id;
	this->flags = flags;
	this->placeID = placeID;
	this->nwExtentX = nwExtentX;
	this->nwExtentY = nwExtentY;
	this->nwExtentZ = nwExtentZ;
	this->seExtentX = seExtentX;
	this->seExtentY = seExtentY;
	this->seExtentZ = seExtentZ;
	this->neCornerZ = neCornerZ;
	this->swCornerZ = swCornerZ;
	this->connections = connections;
	this->hidingSpots = hidingSpots;
	this->encounterPaths = encounterPaths;
	this->ladderConnections = ladderConnections;
	this->cornerLightIntensities = cornerLightIntensities;
	this->visibleAreas = visibleAreas;
	this->inheritVisibilityFromAreaID = inheritVisibilityFromAreaID;
	this->earliestOccupyTimeFirstTeam = earliestOccupyTimeFirstTeam;
	this->earliestOccupyTimeSecondTeam = earliestOccupyTimeSecondTeam;
	this->m_Parent = 0;
	this->m_NextOpen = 0;
	this->m_PrevOpen = 0;
	this->m_fCostSoFar = 0.0f;
	this->m_fTotalCost = 0.0f;
	this->m_fLengthSoFar = 0.0f;
	this->m_iTraverse = NAV_TRAVERSE_COUNT;
	this->m_iMarker = 0;
	this->m_iOpenMarker = 0;
}

void CNavMeshArea::Destroy()
{
	ForEachItem(this->encounterPaths, it)
	{
		INavMeshEncounterPath *path = this->encounterPaths.Element(it);
		if (path) path->Destroy();
	}

	ForEachItem(this->connections, dir)
	{
		this->connections[dir].PurgeAndDeleteElements();
	}

	ForEachItem(this->ladderConnections, dir)
	{
		this->ladderConnections[dir].PurgeAndDeleteElements();
	}

	this->hidingSpots.PurgeAndDeleteElements();
	this->encounterPaths.PurgeAndDeleteElements();
	this->cornerLightIntensities.PurgeAndDeleteElements();
	this->visibleAreas.PurgeAndDeleteElements();

	delete this;
}

unsigned int CNavMeshArea::GetID() { return this->id; }

unsigned int CNavMeshArea::GetFlags() { return this->flags; }

unsigned int CNavMeshArea::GetPlaceID() { return this->placeID; }

float CNavMeshArea::GetNWExtentX() { return this->nwExtentX; }

float CNavMeshArea::GetNWExtentY() { return this->nwExtentY; }

float CNavMeshArea::GetNWExtentZ() { return this->nwExtentZ; }

float CNavMeshArea::GetSEExtentX() { return this->seExtentX; }

float CNavMeshArea::GetSEExtentY() { return this->seExtentY; }

float CNavMeshArea::GetSEExtentZ() { return this->seExtentZ; }

float CNavMeshArea::GetEarliestOccupyTimeFirstTeam() { return this->earliestOccupyTimeFirstTeam; }

float CNavMeshArea::GetEarliestOccupyTimeSecondTeam() { return this->earliestOccupyTimeSecondTeam; }

float CNavMeshArea::GetNECornerZ() { return this->neCornerZ; }

float CNavMeshArea::GetSWCornerZ() { return this->swCornerZ; }

CList<INavMeshConnection*> CNavMeshArea::GetConnections(eNavDir dir) { return this->connections[dir]; }

CList<INavMeshHidingSpot*> CNavMeshArea::GetHidingSpots() { return this->hidingSpots; }

CList<INavMeshEncounterPath*> CNavMeshArea::GetEncounterPaths() { return this->encounterPaths; }

CList<INavMeshLadderConnection*> CNavMeshArea::GetLadderConnections(eNavLadderDir dir) { return this->ladderConnections[dir]; }

CList<INavMeshCornerLightIntensity*> CNavMeshArea::GetCornerLightIntensities() { return this->cornerLightIntensities; }

CList<INavMeshVisibleArea*> CNavMeshArea::GetVisibleAreas() { return this->visibleAreas; }

unsigned int CNavMeshArea::GetInheritVisibilityFromAreaID() { return this->inheritVisibilityFromAreaID; }

bool CNavMeshArea::IsBlocked(void) const { return this->blocked; }

void CNavMeshArea::SetBlocked(const bool blocked) { this->blocked = blocked; }

void CNavMeshArea::AddFlags(const unsigned int flags) { this->flags |= flags; }

void CNavMeshArea::RemoveFlags(const unsigned int flags) { this->flags &= ~flags; }

Vector CNavMeshArea::GetExtentLow()
{
	Vector extent;
	extent.x = this->GetNWExtentX();
	extent.y = this->GetNWExtentY();
	extent.z = this->GetNWExtentZ();

	return extent;
}

Vector CNavMeshArea::GetExtentHigh()
{
	Vector extent;
	extent.x = this->GetSEExtentX();
	extent.y = this->GetSEExtentY();
	extent.z = this->GetSEExtentZ();

	return extent;
}

Vector CNavMeshArea::GetCenter()
{
	Vector center;
	Vector top = this->GetExtentHigh();
	Vector bottom = this->GetExtentLow();

	center.x = (top.x + bottom.x) / 2.0;
	center.y = (top.y + bottom.y) / 2.0;
	center.z = (top.z + bottom.z) / 2.0;

	return center;
}

float CNavMeshArea::GetZ(const Vector &vPos)
{
	if (!vPos.IsValid())
		return 0.0f;

	Vector vExtLo = this->GetExtentLow();
	Vector vExtHi = this->GetExtentHigh();

	float dx = vExtHi.x - vExtLo.x;
	float dy = vExtHi.y - vExtLo.y;

	// Catch divide by zero
	if (dx == 0.0f || dy == 0.0f)
		return this->GetNECornerZ();

	float u = (vPos.x - vExtLo.x) / dx;
	float v = (vPos.y - vExtLo.y) / dy;

	if (u < 0.0f)
		u = 0.0f;
	else if (u > 1.0f)
		u = 1.0f;

	if (v < 0.0f)
		v = 0.0f;
	else if (v > 1.0f)
		v = 1.0f;

	float fNorthZ = vExtLo.z + u * (this->GetNECornerZ() - vExtLo.z);
	float fSouthZ = this->GetSWCornerZ() + u * (vExtHi.z - this->GetSWCornerZ());

	return fNorthZ + v * (fSouthZ - fNorthZ);
}

float CNavMeshArea::GetZ(const float fX, const float fY)
{
	Vector vPos(fX, fY, 0.0f);
	return this->GetZ(vPos);
}

bool CNavMeshArea::IsOverlapping(const Vector &vPos, float flTolerance)
{
	Vector vExtLo = this->GetExtentLow();
	Vector vExtHi = this->GetExtentHigh();

	if (vPos.x + flTolerance >= vExtLo.x && vPos.x - flTolerance <= vExtHi.x &&
		vPos.y + flTolerance >= vExtLo.y && vPos.y - flTolerance <= vExtHi.y) {
		return true;
	}

	return false;
}

bool CNavMeshArea::IsOverlapping(INavMeshArea *toArea)
{
	Vector vFromExtLo = this->GetExtentLow();
	Vector vFromExtHi = this->GetExtentHigh();

	Vector vToExtLo = toArea->GetExtentLow();
	Vector vToExtHi = toArea->GetExtentHigh();

	if (vToExtLo.x < vFromExtHi.x && vToExtHi.x > vFromExtLo.x &&
		vToExtLo.y < vFromExtHi.y && vToExtHi.y > vFromExtLo.y) {
		return true;
	}

	return false;
}

void CNavMeshArea::SetTotalCost(float total) { this->m_fTotalCost = total; }

void CNavMeshArea::SetCostSoFar(float cost) { this->m_fCostSoFar = cost; }

float CNavMeshArea::GetTotalCost() { return this->m_fTotalCost; }

float CNavMeshArea::GetCostSoFar() { return this->m_fCostSoFar; }

void CNavMeshArea::SetLengthSoFar(float length) { this->m_fLengthSoFar = length; }

float CNavMeshArea::GetLengthSoFar() { return this->m_fLengthSoFar; }

INavMeshArea *CNavMeshArea::GetParent() { return this->m_Parent; }

eNavTraverse CNavMeshArea::GetParentHow() { return this->m_iTraverse; }

void CNavMeshArea::SetParent(INavMeshArea *area) { this->m_Parent = area; }

void CNavMeshArea::SetParentHow(eNavTraverse traverse) { this->m_iTraverse = traverse; }

void CNavMeshArea::MakeNewMarker() { ++m_iMasterMarker; if (m_iMasterMarker == 0) m_iMasterMarker = 1; }

void CNavMeshArea::Mark() { this->m_iMarker = m_iMasterMarker; }

bool CNavMeshArea::IsMarked() const { return (this->m_iMarker == m_iMasterMarker); }

bool CNavMeshArea::IsOpen() const { return (this->m_iOpenMarker == m_iMasterMarker); }

/**
* Add to open list in decreasing value order
*/
void CNavMeshArea::AddToOpenList()
{
	this->m_iOpenMarker = m_iMasterMarker;

	if (!m_OpenList)
	{
		m_OpenList = this;
		this->SetPrevOpen(nullptr);
		this->SetNextOpen(nullptr);
		return;
	}

	INavMeshArea *area, *last = nullptr;
	for (area = m_OpenList; area; area = area->GetNextOpen())
	{
		if (this->m_fTotalCost < area->GetTotalCost())
			break;

		last = area;
	}

	if (area)
	{
		// insert before this area
		this->SetPrevOpen(area->GetPrevOpen());
		if (this->m_PrevOpen)
			this->m_PrevOpen->SetNextOpen(this);
		else
			m_OpenList = this;

		this->SetNextOpen(area);
		area->SetPrevOpen(this);
	}
	else
	{
		// append to end of list
		last->SetNextOpen(this);

		this->SetPrevOpen(last);
		this->SetNextOpen(nullptr);
	}
}

/**
 * A smaller value has been found, update this area on the open list
 */
void CNavMeshArea::UpdateOnOpenList()
{
	while (this->m_PrevOpen && this->m_fTotalCost < this->m_PrevOpen->GetTotalCost())
	{
		// swap position with predecessor
		INavMeshArea *other = this->GetPrevOpen();
		INavMeshArea *before = other->GetPrevOpen();
		INavMeshArea *after = this->GetNextOpen();

		this->SetNextOpen(other);
		this->SetPrevOpen(before);

		other->SetPrevOpen(this);
		other->SetNextOpen(after);

		if (before)
			before->SetNextOpen(this);
		else
			m_OpenList = this;

		if (after)
			after->SetPrevOpen(other);
	}
}

void CNavMeshArea::RemoveFromOpenList()
{
	if (this->m_PrevOpen)
		this->m_PrevOpen->SetNextOpen(this->m_NextOpen);
	else
		m_OpenList = this->m_NextOpen;

	if (this->m_NextOpen)
		this->m_NextOpen->SetPrevOpen(this->m_PrevOpen);

	// zero is an invalid marker
	this->m_iOpenMarker = 0;
}

bool CNavMeshArea::IsOpenListEmpty() { return (m_OpenList) ? false : true; }

INavMeshArea *CNavMeshArea::PopOpenList()
{
	if (m_OpenList)
	{
		INavMeshArea *area = m_OpenList;

		area->RemoveFromOpenList();

		return area;
	}

	return nullptr;
}

bool CNavMeshArea::IsClosed() const
{
	if (this->IsMarked() && !this->IsOpen())
		return true;

	return false;
}

void CNavMeshArea::AddToClosedList() { this->m_iMarker = m_iMasterMarker; }

void CNavMeshArea::RemoveFromClosedList() { this->m_iMarker = 0; }

void CNavMeshArea::ClearSearchList()
{
	CNavMeshArea::MakeNewMarker();

	m_OpenList = nullptr;
}

INavMeshArea *CNavMeshArea::GetNextOpen() { return this->m_NextOpen; }

void CNavMeshArea::SetNextOpen(INavMeshArea *open) { this->m_NextOpen = open; }

INavMeshArea *CNavMeshArea::GetPrevOpen() { return this->m_PrevOpen; }

void CNavMeshArea::SetPrevOpen(INavMeshArea *open) { this->m_PrevOpen = open; }
