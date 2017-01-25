#ifndef __war3source_inavmesharea_h__
#define __war3source_inavmesharea_h__

#include "IList.h"
#include "INavMeshConnection.h"
#include "INavMeshHidingSpot.h"
#include "INavMeshEncounterPath.h"
#include "INavMeshLadderConnection.h"
#include "INavMeshCornerLightIntensity.h"
#include "INavMeshVisibleArea.h"


class INavMeshArea 
{
public:
	virtual unsigned int GetID() = 0;
	virtual unsigned int GetFlags() = 0;
	virtual unsigned int GetPlaceID() = 0;

	virtual float GetNWExtentX() = 0;
	virtual float GetNWExtentY() = 0;
	virtual float GetNWExtentZ() = 0;

	virtual float GetSEExtentX() = 0;
	virtual float GetSEExtentY() = 0;
	virtual float GetSEExtentZ() = 0;

	virtual float GetEarliestOccupyTimeFirstTeam() = 0;
	virtual float GetEarliestOccupyTimeSecondTeam() = 0;

	virtual float GetNECornerZ() = 0;
	virtual float GetSWCornerZ() = 0;

	virtual IList<INavMeshConnection*> *GetConnections() = 0;
	virtual IList<INavMeshHidingSpot*> *GetHidingSpots() = 0;
	virtual IList<INavMeshEncounterPath*> *GetEncounterPaths() = 0;
	virtual IList<INavMeshLadderConnection*> *GetLadderConnections() = 0;
	virtual IList<INavMeshCornerLightIntensity*> *GetCornerLightIntensities() = 0;
	virtual IList<INavMeshVisibleArea*> *GetVisibleAreas() = 0;

	virtual unsigned int GetInheritVisibilityFromAreaID() = 0;

	virtual unsigned char GetUnk01() = 0;

	virtual bool IsBlocked() = 0;
	virtual void SetBlocked(bool blocked) = 0;

	virtual void SetTotalCost(float total) = 0;
	virtual void SetCostSoFar(float cost) = 0;
	virtual float GetTotalCost() = 0;
	virtual float GetCostSoFar() = 0;

	virtual void SetLengthSoFar(float length) = 0;
	virtual float GetLengthSoFar() = 0;

	virtual INavMeshArea *GetParent() = 0;
	virtual eNavTraverse GetParentHow() = 0;
	virtual void SetParent(INavMeshArea *parent) = 0;
	virtual void SetParentHow(eNavTraverse traverse) = 0;

	virtual void Mark() = 0;
	virtual bool IsMarked() const = 0;

	virtual bool IsOpen() const = 0;
	virtual void AddToOpenList() = 0;
	virtual void UpdateOnOpenList() = 0;
	virtual void RemoveFromOpenList() = 0;

	virtual bool IsClosed() const = 0;
	virtual void AddToClosedList() = 0;
	virtual void RemoveFromClosedList() = 0;

	virtual INavMeshArea *GetNextOpen() = 0;
	virtual void SetNextOpen(INavMeshArea *open) = 0;
	virtual INavMeshArea *GetPrevOpen() = 0;
	virtual void SetPrevOpen(INavMeshArea *prev) = 0;
};

#endif