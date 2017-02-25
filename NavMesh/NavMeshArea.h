#ifndef __war3source_navmesharea_h__
#define __war3source_navmesharea_h__

#include "public\INavMeshArea.h"
#include "public\IList.h"
#include "public\INavMeshConnection.h"
#include "public\INavMeshHidingSpot.h"
#include "public\INavMeshEncounterPath.h"
#include "public\INavMeshLadderConnection.h"
#include "public\INavMeshCornerLightIntensity.h"
#include "public\INavMeshVisibleArea.h"


static int m_iMasterMarker;
static INavMeshArea *m_OpenList;

class CNavMeshArea : public INavMeshArea
{
public:
	CNavMeshArea(unsigned int id, unsigned int flags, unsigned int placeID,
		float nwExtentX, float nwExtentY, float nwExtentZ,
		float seExtentX, float seExtentY, float seExtentZ,
		float neCornerZ, float swCornerZ,
		IList<INavMeshConnection*> *connections, IList<INavMeshHidingSpot*> *hidingSpots, IList<INavMeshEncounterPath*> *encounterPaths,
		IList<INavMeshLadderConnection*> *ladderConnections, IList<INavMeshCornerLightIntensity*> *cornerLightIntensities,
		IList<INavMeshVisibleArea*> *visibleAreas, unsigned int inheritVisibilityFromAreaID,
		float earliestOccupyTimeFirstTeam, float earliestOccupyTimeSecondTeam, unsigned char unk01);
	~CNavMeshArea();

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

	IList<INavMeshConnection*> *GetConnections();
	IList<INavMeshHidingSpot*> *GetHidingSpots();
	IList<INavMeshEncounterPath*> *GetEncounterPaths();
	IList<INavMeshLadderConnection*> *GetLadderConnections();
	IList<INavMeshCornerLightIntensity*> *GetCornerLightIntensities();
	IList<INavMeshVisibleArea*> *GetVisibleAreas();

	unsigned int GetInheritVisibilityFromAreaID();

	unsigned char GetUnk01();

	// The following functions come from Kit o' Rifty and the SDK

	bool IsBlocked();
	void SetBlocked(bool blocked);

	Vector GetExtentLow();
	Vector GetExtentHigh();
	Vector GetCenter();

	float GetZ(const Vector &vPos);
	float GetZ(float fX, float fY);

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

	static void MakeNewMarker();
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

	static void ClearSearchList();

	INavMeshArea *GetNextOpen();
	void SetNextOpen(INavMeshArea *open);
	INavMeshArea *GetPrevOpen();
	void SetPrevOpen(INavMeshArea *open);

	int GetNearMarker();
	void SetNearMarker(int marker);

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

	IList<INavMeshConnection*> *connections;
	IList<INavMeshHidingSpot*> *hidingSpots;
	IList<INavMeshEncounterPath*> *encounterPaths;
	IList<INavMeshLadderConnection*> *ladderConnections;
	IList<INavMeshCornerLightIntensity*> *cornerLightIntensities;
	IList<INavMeshVisibleArea*> *visibleAreas;

	float earliestOccupyTimeFirstTeam;
	float earliestOccupyTimeSecondTeam;

	unsigned int inheritVisibilityFromAreaID;

	unsigned char unk01;

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
	int m_NearMarker;
};

#endif