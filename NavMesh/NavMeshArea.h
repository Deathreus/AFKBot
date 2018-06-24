#ifndef __war3source_navmesharea_h__
#define __war3source_navmesharea_h__

#include "public\INavMeshArea.h"
#include "public\INavMeshConnection.h"
#include "public\INavMeshHidingSpot.h"
#include "public\INavMeshEncounterPath.h"
#include "public\INavMeshLadderConnection.h"
#include "public\INavMeshCornerLightIntensity.h"
#include "public\INavMeshVisibleArea.h"


typedef CList<CList<INavMeshConnection*>> ConnectionList_t;
typedef CList<CList<INavMeshLadderConnection*>> LadderConList_t;

class CNavMeshArea : public INavMeshArea
{
public:
	CNavMeshArea(unsigned int id, unsigned int flags, unsigned int placeID,
		float nwExtentX, float nwExtentY, float nwExtentZ,
		float seExtentX, float seExtentY, float seExtentZ,
		float neCornerZ, float swCornerZ,
		const CList<CList<INavMeshConnection*>> connections, const CList<INavMeshHidingSpot*> hidingSpots,
		const CList<INavMeshEncounterPath*> encounterPaths, const CList<CList<INavMeshLadderConnection*>> ladderConnections,
		const CList<INavMeshCornerLightIntensity*> cornerLightIntensities, const CList<INavMeshVisibleArea*> visibleAreas,
		unsigned int inheritVisibilityFromAreaID,
		float earliestOccupyTimeFirstTeam, float earliestOccupyTimeSecondTeam);
	~CNavMeshArea() {}

	void Destroy();

	unsigned int GetID();
	unsigned int GetFlags();
	unsigned int GetPlaceID();

	float GetNWExtentX();
	float GetNWExtentY();
	float GetNWExtentZ();

	float GetSEExtentX();
	float GetSEExtentY();
	float GetSEExtentZ();

	float GetEarliestOccupyTimeFirstTeam();
	float GetEarliestOccupyTimeSecondTeam();

	float GetNECornerZ();
	float GetSWCornerZ();

	CList<INavMeshConnection*> GetConnections(eNavDir dir);
	CList<INavMeshHidingSpot*> GetHidingSpots();
	CList<INavMeshEncounterPath*> GetEncounterPaths();
	CList<INavMeshLadderConnection*> GetLadderConnections(eNavLadderDir dir);
	CList<INavMeshCornerLightIntensity*> GetCornerLightIntensities();
	CList<INavMeshVisibleArea*> GetVisibleAreas();

	unsigned int GetInheritVisibilityFromAreaID();
	
	void AddFlags(const unsigned int flags);
	void RemoveFlags(const unsigned int flags);

	// The following functions come from Kit o' Rifty and the SDK

	bool IsBlocked(void) const;
	void SetBlocked(const bool blocked);

	Vector GetExtentLow();
	Vector GetExtentHigh();
	Vector GetCenter();

	float GetZ(const Vector &vPos);
	float GetZ(const float fX, const float fY);

	bool IsOverlapping(const Vector &vPos, float fTolerance = 0.0f);
	bool IsOverlapping(INavMeshArea *toArea);

	void SetTotalCost(float total);
	void SetCostSoFar(float cost);
	float GetTotalCost();
	float GetCostSoFar();

	void SetLengthSoFar(float length);
	float GetLengthSoFar();

	INavMeshArea *GetParent();
	eNavTraverse GetParentHow();
	void SetParent(INavMeshArea *area);
	void SetParentHow(eNavTraverse traverse);

	void Mark();
	bool IsMarked() const;

	bool IsOpen() const;
	void AddToOpenList();
	void UpdateOnOpenList();
	void RemoveFromOpenList();
	static bool IsOpenListEmpty();
	static INavMeshArea *PopOpenList();

	bool IsClosed() const;
	void AddToClosedList();
	void RemoveFromClosedList();

	static void MakeNewMarker();
	static void ClearSearchList();

	INavMeshArea *GetNextOpen();
	void SetNextOpen(INavMeshArea *open);
	INavMeshArea *GetPrevOpen();
	void SetPrevOpen(INavMeshArea *open);

	static int m_iMasterMarker;
	static INavMeshArea *m_OpenList;

private:
	unsigned int id;
	unsigned int flags;
	unsigned int placeID;

	float nwExtentX;
	float nwExtentY;
	float nwExtentZ;

	float seExtentX;
	float seExtentY;
	float seExtentZ;

	float neCornerZ;
	float swCornerZ;

	CList<CList<INavMeshConnection*>> connections;
	CList<INavMeshHidingSpot*> hidingSpots;
	CList<INavMeshEncounterPath*> encounterPaths;
	CList<CList<INavMeshLadderConnection*>> ladderConnections;
	CList<INavMeshCornerLightIntensity*> cornerLightIntensities;
	CList<INavMeshVisibleArea*> visibleAreas;

	float earliestOccupyTimeFirstTeam;
	float earliestOccupyTimeSecondTeam;

	unsigned int inheritVisibilityFromAreaID;

	bool blocked;

	// A* pathfinding
	INavMeshArea *m_Parent;
	eNavTraverse m_iTraverse;
	float m_fCostSoFar;
	float m_fTotalCost;
	int m_iMarker;
	int m_iOpenMarker;
	INavMeshArea *m_PrevOpen;
	INavMeshArea *m_NextOpen;
	float m_fLengthSoFar;
};

#endif
