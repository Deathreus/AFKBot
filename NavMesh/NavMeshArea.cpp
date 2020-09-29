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
	const CList<CList<INavMeshConnection*>> &connections, const CList<INavMeshHidingSpot*> &hidingSpots,
	const CList<INavMeshEncounterPath*> &encounterPaths, const CList<CList<INavMeshLadderConnection*>> &ladderConnections,
	const CList<INavMeshCornerLightIntensity*> &cornerLightIntensities, const CList<INavMeshVisibleArea*> &visibleAreas,
	unsigned int inheritVisibilityFromAreaID, float earliestOccupyTimeFirstTeam, float earliestOccupyTimeSecondTeam)
{
	this->id = id;
	this->flags = flags;
	this->placeID = placeID;
	this->nwExtent = Vector(nwExtentX, nwExtentY, nwExtentZ);
	this->seExtent = Vector(seExtentX, seExtentY, seExtentZ);
	this->center = (this->nwExtent + this->seExtent) / 2.0f;
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
	for (auto path : this->encounterPaths)
	{
		if (path) path->Destroy();
	}

	for(short int dir = 0; dir < NAV_DIR_COUNT; dir++)
	{
		this->connections[dir].Clear();
	}

	for(short int dir = 0; dir < NAV_LADDER_DIR_COUNT; dir++)
	{
		this->ladderConnections[dir].Clear();
	}

	this->hidingSpots.Clear();
	this->encounterPaths.Clear();
	this->cornerLightIntensities.Clear();
	this->visibleAreas.Clear();

	delete this;
}

unsigned int CNavMeshArea::GetID() { return this->id; }

unsigned int CNavMeshArea::GetAttributes() { return this->flags; }

unsigned int CNavMeshArea::GetPlaceID() { return this->placeID; }

float CNavMeshArea::GetNWExtentX() { return this->nwExtent.x; }

float CNavMeshArea::GetNWExtentY() { return this->nwExtent.y; }

float CNavMeshArea::GetNWExtentZ() { return this->nwExtent.z; }

float CNavMeshArea::GetSEExtentX() { return this->seExtent.x; }

float CNavMeshArea::GetSEExtentY() { return this->seExtent.y; }

float CNavMeshArea::GetSEExtentZ() { return this->seExtent.z; }

float CNavMeshArea::GetEarliestOccupyTimeFirstTeam() { return this->earliestOccupyTimeFirstTeam; }

float CNavMeshArea::GetEarliestOccupyTimeSecondTeam() { return this->earliestOccupyTimeSecondTeam; }

float CNavMeshArea::GetNECornerZ() { return this->neCornerZ; }

float CNavMeshArea::GetSWCornerZ() { return this->swCornerZ; }

const CList<INavMeshConnection*> *CNavMeshArea::GetConnections(eNavDir dir) { return &this->connections[dir]; }

const CList<INavMeshHidingSpot*> *CNavMeshArea::GetHidingSpots() { return &this->hidingSpots; }

const CList<INavMeshEncounterPath*> *CNavMeshArea::GetEncounterPaths() { return &this->encounterPaths; }

const CList<INavMeshLadderConnection*> *CNavMeshArea::GetLadderConnections(eNavLadderDir dir) { return &this->ladderConnections[dir]; }

const CList<INavMeshCornerLightIntensity*> *CNavMeshArea::GetCornerLightIntensities() { return &this->cornerLightIntensities; }

const CList<INavMeshVisibleArea*> *CNavMeshArea::GetVisibleAreas() { return &this->visibleAreas; }

unsigned int CNavMeshArea::GetInheritVisibilityFromAreaID() { return this->inheritVisibilityFromAreaID; }

bool CNavMeshArea::IsBlocked(void) const { return this->blocked; }

void CNavMeshArea::SetBlocked(const bool blocked) { this->blocked = blocked; }

void CNavMeshArea::AddFlags(const unsigned int flags) { this->flags |= flags; }

void CNavMeshArea::RemoveFlags(const unsigned int flags) { this->flags &= ~flags; }

const Vector &CNavMeshArea::GetExtentLow()
{
	return this->nwExtent;
}

const Vector &CNavMeshArea::GetExtentHigh()
{
	return this->seExtent;
}

const Vector &CNavMeshArea::GetCenter()
{
	return this->center;
}

float CNavMeshArea::GetZ(const Vector &vPos)
{
	if (!vPos.IsValid())
		return 0.0f;

	const Vector vExtLo = this->nwExtent;
	const Vector vExtHi = this->seExtent;

	const float dx = vExtHi.x - vExtLo.x;
	const float dy = vExtHi.y - vExtLo.y;

	// Catch divide by zero
	if (dx == 0.0f || dy == 0.0f)
		return this->neCornerZ;

	const float u = clamp((vPos.x - vExtLo.x) / dx, 0.0f, 1.0f);
	const float v = clamp((vPos.y - vExtLo.y) / dy, 0.0f, 1.0f);

	const float fNorthZ = vExtLo.z + u * (this->neCornerZ - vExtLo.z);
	const float fSouthZ = this->swCornerZ + u * (vExtHi.z - this->swCornerZ);

	return fNorthZ + v * (fSouthZ - fNorthZ);
}

float CNavMeshArea::GetZ(const float fX, const float fY)
{
	const Vector vPos(fX, fY, 0.0f);
	return this->GetZ(vPos);
}

bool CNavMeshArea::IsOverlapping(const Vector &vPos, const float flTolerance)
{
	const Vector vExtLo = this->GetExtentLow();
	const Vector vExtHi = this->GetExtentHigh();

	if(vPos.x + flTolerance >= vExtLo.x && vPos.x - flTolerance <= vExtHi.x 
	&& vPos.y + flTolerance >= vExtLo.y && vPos.y - flTolerance <= vExtHi.y)
	{
		return true;
	}

	return false;
}

bool CNavMeshArea::IsOverlapping(INavMeshArea *toArea)
{
	const Vector vFromExtLo = this->GetExtentLow();
	const Vector vFromExtHi = this->GetExtentHigh();

	const Vector vToExtLo = toArea->GetExtentLow();
	const Vector vToExtHi = toArea->GetExtentHigh();

	if(vToExtLo.x < vFromExtHi.x && vToExtHi.x > vFromExtLo.x 
	&& vToExtLo.y < vFromExtHi.y && vToExtHi.y > vFromExtLo.y)
	{
		return true;
	}

	return false;
}

INavMeshEncounterPath *CNavMeshArea::GetSpotEncounter(const int iFromID, const int iToID)
{
	for(INavMeshEncounterPath *e : this->encounterPaths)
	{
		if(e->GetFromAreaID() == iFromID && e->GetToAreaID() == iToID)
			return e;
	}

	return nullptr;
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
		INavMeshArea *other = this->m_PrevOpen;
		INavMeshArea *before = other->GetPrevOpen();
		INavMeshArea *after = this->m_NextOpen;

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

#if(SOURCE_ENGINE == SE_TF2)
void CNavMeshArea::SetTFAttribs(unsigned int iFlags) { this->TFFlags = iFlags; }

void CNavMeshArea::AddTFAttribs(unsigned int iFlags) { this->TFFlags |= iFlags; }

void CNavMeshArea::RemoveTFAttribs(unsigned int iFlags) { this->TFFlags &= ~iFlags; }

unsigned int CNavMeshArea::GetTFAttribs() { return this->TFFlags; }
#endif