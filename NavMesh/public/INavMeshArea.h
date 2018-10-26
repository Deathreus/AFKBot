#ifndef __war3source_inavmesharea_h__
#define __war3source_inavmesharea_h__

#include "List.h"
#include "INavMeshConnection.h"
#include "INavMeshHidingSpot.h"
#include "INavMeshEncounterPath.h"
#include "INavMeshLadderConnection.h"
#include "INavMeshCornerLightIntensity.h"
#include "INavMeshVisibleArea.h"


enum eNavDir;
enum eNavLadderDir;
class Vector;

class INavMeshArea
{
public:
	virtual void Destroy() = 0;

	virtual unsigned int GetID() = 0;
	virtual unsigned int GetAttributes() = 0;
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

	virtual CList<INavMeshConnection*> *GetConnections(eNavDir) = 0;
	virtual CList<INavMeshHidingSpot*> *GetHidingSpots() = 0;
	virtual CList<INavMeshEncounterPath*> *GetEncounterPaths() = 0;
	virtual CList<INavMeshLadderConnection*> *GetLadderConnections(eNavLadderDir) = 0;
	virtual CList<INavMeshCornerLightIntensity*> *GetCornerLightIntensities() = 0;
	virtual CList<INavMeshVisibleArea*> *GetVisibleAreas() = 0;

	virtual unsigned int GetInheritVisibilityFromAreaID() = 0;

	virtual bool IsBlocked() const = 0;
	virtual void SetBlocked(const bool) = 0;

	virtual void AddFlags(const unsigned int) = 0;
	virtual void RemoveFlags(const unsigned int) = 0;

	virtual const Vector GetExtentLow() = 0;
	virtual const Vector GetExtentHigh() = 0;
	virtual const Vector GetCenter() = 0;

	virtual float GetZ(const Vector &) = 0;
	virtual float GetZ(const float, const float) = 0;

	virtual bool IsOverlapping(const Vector &, const float = 0.0f) = 0;
	virtual bool IsOverlapping(INavMeshArea *) = 0;

	virtual INavMeshEncounterPath *GetSpotEncounter(const int, const int) = 0;

	virtual void SetTotalCost(float total) = 0;
	virtual void SetCostSoFar(float cost) = 0;
	virtual float GetTotalCost() = 0;
	virtual float GetCostSoFar() = 0;

	virtual void SetLengthSoFar(float length) = 0;
	virtual float GetLengthSoFar() = 0;

	virtual INavMeshArea *GetParent() = 0;
	virtual eNavTraverse GetParentHow() = 0;
	virtual void SetParent(INavMeshArea *) = 0;
	virtual void SetParentHow(eNavTraverse) = 0;

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
	virtual void SetNextOpen(INavMeshArea *) = 0;
	virtual INavMeshArea *GetPrevOpen() = 0;
	virtual void SetPrevOpen(INavMeshArea *) = 0;

#if(SOURCE_ENGINE == SE_TF2)
	virtual void SetTFAttribs(unsigned int) = 0;
	virtual void AddTFAttribs(unsigned int) = 0;
	virtual void RemoveTFAttribs(unsigned int) = 0;
	virtual unsigned GetTFAttribs() = 0;
#endif
};

#endif