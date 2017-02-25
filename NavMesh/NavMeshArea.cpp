#include "NavMeshArea.h"
#include "vector.h"

CNavMeshArea::CNavMeshArea(unsigned int id, unsigned int flags, unsigned int placeID,
	float nwExtentX, float nwExtentY, float nwExtentZ,
	float seExtentX, float seExtentY, float seExtentZ,
	float neCornerZ, float swCornerZ,
	IList<INavMeshConnection*> *connections, IList<INavMeshHidingSpot*> *hidingSpots, IList<INavMeshEncounterPath*> *encounterPaths,
	IList<INavMeshLadderConnection*> *ladderConnections, IList<INavMeshCornerLightIntensity*> *cornerLightIntensities,
	IList<INavMeshVisibleArea*> *visibleAreas, unsigned int inheritVisibilityFromAreaID,
	float earliestOccupyTimeFirstTeam, float earliestOccupyTimeSecondTeam, unsigned char unk01)
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
	this->unk01 = unk01;
	this->blocked = false;
	this->m_Parent = 0;
	this->m_NextOpen = 0;
	this->m_PrevOpen = 0;
	this->m_fCostSoFar = 0.0f;
	this->m_fTotalCost = 0.0f;
	this->m_fLengthSoFar = 0.0f;
	this->m_iTraverse = NAV_TRAVERSE_COUNT;
	this->m_iMarker = 0;
	this->m_iOpenMarker = 0;
	this->m_NearMarker = 0;
}

CNavMeshArea::~CNavMeshArea() {}

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

IList<INavMeshConnection*> *CNavMeshArea::GetConnections() { return this->connections; }

IList<INavMeshHidingSpot*> *CNavMeshArea::GetHidingSpots() { return this->hidingSpots; }

IList<INavMeshEncounterPath*> *CNavMeshArea::GetEncounterPaths() { return this->encounterPaths; }

IList<INavMeshLadderConnection*> *CNavMeshArea::GetLadderConnections() { return this->ladderConnections; }

IList<INavMeshCornerLightIntensity*> *CNavMeshArea::GetCornerLightIntensities() { return this->cornerLightIntensities; }

IList<INavMeshVisibleArea*> *CNavMeshArea::GetVisibleAreas() { return this->visibleAreas; }

unsigned int CNavMeshArea::GetInheritVisibilityFromAreaID() { return this->inheritVisibilityFromAreaID; }

unsigned char CNavMeshArea::GetUnk01() { return this->unk01; }

bool CNavMeshArea::IsBlocked() { return this->blocked; }

void CNavMeshArea::SetBlocked(bool blocked) { this->blocked = blocked; }

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

float CNavMeshArea::GetZ(float fX, float fY)
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

bool CNavMeshArea::IsMarked() const { return (this->m_iMarker == m_iMasterMarker) ? true : false; }

bool CNavMeshArea::IsOpen() const { return (this->m_iOpenMarker == m_iMasterMarker) ? true : false; }

/**
* Add to open list in decreasing value order
*/
void CNavMeshArea::AddToOpenList()
{
	this->m_iOpenMarker = m_iMasterMarker;

	if (!m_OpenList)
	{
		m_OpenList = this;
		this->SetPrevOpen(0);
		this->SetNextOpen(0);
		return;
	}

	INavMeshArea *area, *last = 0;
	for (area = m_OpenList; area; area = area->GetNextOpen())
	{
		if (this->GetTotalCost() < area->GetTotalCost())
			break;

		last = area;
	}

	if (area)
	{
		// insert before this area
		this->SetPrevOpen(area->GetPrevOpen());
		if (this->GetPrevOpen())
			this->GetPrevOpen()->SetNextOpen(this);
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
		this->SetNextOpen(0);
	}
}

/**
* A smaller value has been found, update this area on the open list
*/
void CNavMeshArea::UpdateOnOpenList()
{
	while (m_PrevOpen && this->GetTotalCost() < m_PrevOpen->GetTotalCost())
	{
		// swap position with predecessor
		INavMeshArea *other = m_PrevOpen;
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
	if (m_PrevOpen)
		m_PrevOpen->SetNextOpen(m_NextOpen);
	else
		m_OpenList = m_NextOpen;

	if (m_NextOpen)
		m_NextOpen->SetPrevOpen(m_PrevOpen);

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

	return NULL;
}

bool CNavMeshArea::IsClosed() const
{
	if (this->IsMarked() && !this->IsOpen())
		return true;

	return false;
}

void CNavMeshArea::AddToClosedList() { this->Mark(); }

void CNavMeshArea::RemoveFromClosedList() {}

void CNavMeshArea::ClearSearchList()
{
	CNavMeshArea::MakeNewMarker();

	m_OpenList = 0;
}

INavMeshArea *CNavMeshArea::GetNextOpen() { return this->m_NextOpen; }

void CNavMeshArea::SetNextOpen(INavMeshArea *open) { this->m_NextOpen = open; }

INavMeshArea *CNavMeshArea::GetPrevOpen() { return this->m_PrevOpen; }

void CNavMeshArea::SetPrevOpen(INavMeshArea *open) { this->m_PrevOpen = open; }

int CNavMeshArea::GetNearMarker() { return this->m_NearMarker; }

void CNavMeshArea::SetNearMarker(int marker) { this->m_NearMarker = marker; }