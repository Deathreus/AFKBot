/*
 *    This file is part of RCBot.
 *
 *    RCBot by Paul Murphy adapted from Botman's HPB Bot 2 template.
 *
 *    RCBot is free software; you can redistribute it and/or modify it
 *    under the terms of the GNU General Public License as published by the
 *    Free Software Foundation; either version 2 of the License, or (at
 *    your option) any later version.
 *
 *    RCBot is distributed in the hope that it will be useful, but
 *    WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with RCBot; if not, write to the Free Software Foundation,
 *    Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *    In addition, as a special exception, the author gives permission to
 *    link the code of this program with the Half-Life Game engine ("HL
 *    engine") and Modified Game Libraries ("MODs") developed by Valve,
 *    L.L.C ("Valve").  You must obey the GNU General Public License in all
 *    respects for all of the code used other than the HL engine and MODs
 *    from Valve.  If you modify this file, you may extend this exception
 *    to your version of the file, but you are not obligated to do so.  If
 *    you do not wish to do so, delete this exception statement from your
 *    version.
 *
 */
#ifndef __RCBOT_NAVIGATOR_H__
#define __RCBOT_NAVIGATOR_H__

#include <vector>
#include <queue>
using namespace std;

#include "bot_base.h"
#include "bot_waypoint.h"
#include "..\NavMesh\NavMesh.h"

#include "bot_belief.h"
#include "bot_genclass.h"

class CNavMesh;
class CWaypointVisibilityTable;
class INavMeshArea;

#define MAX_BELIEF 200.0f

class INavigatorNode
{
public:
	inline Vector GetOrigin() { return m_vOrigin; }
protected:
	Vector m_vOrigin;
};

class IBotNavigator
{
public:
	virtual void Init() = 0;

	// returns true when working out route finishes, not if successful
	virtual bool WorkRoute(Vector vFrom, Vector vTo, bool *bFail, bool bRestart = true, bool bNoInterruptions = false, int iGoalId = -1, int iConditions = 0, int iDangerId = -1) = 0;

	virtual void RollBackPosition() = 0;

	virtual void FailMove() = 0;

	virtual float GetNextYaw() = 0;

	virtual bool GetNextRoutePoint(Vector *vPoint) = 0;

	inline Vector GetPreviousPoint() { return m_vPreviousPoint; }

	virtual bool HasNextPoint() = 0;

	virtual int GetCurrentWaypointID() = 0;

	virtual int GetCurrentGoalID() = 0;

	virtual Vector GetNextPoint() = 0;

	virtual void UpdatePosition() = 0;

	virtual bool CanGetTo(Vector vOrigin) = 0;

	virtual int GetCurrentFlags() { return 0; }
	virtual int GetPathFlags(int iPath) { return 0; }

	virtual float DistanceTo(Vector vOrigin) = 0;

	virtual float DistanceTo(CWaypoint *pWaypoint) = 0;

	//virtual void GoBack () = 0;

	virtual void FreeMapMemory() = 0;

	virtual void FreeAllMemory() = 0;

	virtual bool RouteFound() = 0;

	virtual void Clear() = 0;

	virtual void GetFailedGoals(dataUnconstArray <int> **goals) = 0;

	inline Vector GetGoalOrigin() { return m_vGoal; }

	virtual bool NextPointIsOnLadder() { return false; }

	virtual bool BeliefLoad() { return false; };

	virtual bool BeliefSave(bool bOverride = false) { return false; };

	virtual void Belief(Vector origin, Vector vOther, float fBelief, float fStrength, BotBelief iType) = 0;

	// nearest cover position to vOrigin only
	virtual bool GetCoverPosition(Vector vCoverOrigin, Vector *vCover) = 0;
	// nearest cover postion to both vectors
	virtual bool GetHideSpotPosition(Vector vCoverOrigin, Vector *vCover) = 0;

	virtual float GetCurrentBelief() { return 0; }

	virtual float GetBelief(int index) { return 0; }

	virtual void BeliefOne(int iWptIndex, BotBelief iBeliefType, float fDist) { return; }

	virtual int NumPaths() { return 0; }

	virtual Vector GetPath(int pathid) { return Vector(0, 0, 0); }

	virtual bool RandomDangerPath(Vector *vec) { return false; }

	bool GetDangerPoint(Vector *vec) { *vec = m_bDangerPoint ? m_vDangerPoint : Vector(0, 0, 0); return m_bDangerPoint; }

	bool WantToLoadBelief() { return m_bLoadBelief; }
	virtual bool WantToSaveBelief() { return false; }
	float GetGoalDistance() { return m_fGoalDistance; }

	static const int MAX_PATH_TICKS = 200;

protected:
	Vector m_vGoal;
	float m_fGoalDistance;
	Vector m_vPreviousPoint;
	Vector m_vDangerPoint;
	bool m_bDangerPoint;
	short int m_iBeliefTeam;
	bool m_bBeliefChanged;
	bool m_bLoadBelief;
};

// Tries to mimic the old waypoint navigator as closely as possible
class INavMeshNavigator
{
public:
	virtual void FreeMapMemory() = 0;

	virtual bool WorkRoute(Vector vFrom, Vector vTo, bool *bFail, bool bRestart = true, bool bNoInterruptions = false, INavMeshArea *goal = NULL, int iConditions = 0, int iDangerId = -1) = 0;

	virtual INavMeshArea *ChooseBestFromBelief(IList<INavMeshArea*> *goals, bool bHighDanger = false, int iSearchFlags = 0, int iTeam = 0) = 0;
	virtual INavMeshArea *ChooseBestFromBeliefBetweenAreas(IList<INavMeshArea*> *goals, bool bHighDanger = false, bool bIgnoreBelief = false) = 0;

	virtual void RollBackPosition() = 0;

	virtual void FailMove() = 0;

	virtual Vector GetNextPoint() = 0;
	inline Vector GetPreviousPoint() { return m_vPreviousPoint; }

	virtual bool HasNextPoint() = 0;

	virtual void UpdatePosition() = 0;

	virtual INavMeshArea *GetCurrentWaypoint() = 0;
	virtual INavMeshArea *GetCurrentGoal() = 0;

	virtual bool GetNextRoutePoint(Vector *vPoint) = 0;

	virtual bool NextPointIsOnLadder() = 0;

	virtual bool CanGetTo(Vector vOrigin) = 0;

	virtual int GetCurrentFlags() { return 0; }

	virtual bool GetCoverPosition(Vector vCoverOrigin, Vector *vCover) = 0;
	virtual bool GetHideSpotPosition(Vector vCoverOrigin, Vector *vCover) = 0;

	virtual float DistanceTo(Vector vOrigin) = 0;
	virtual float DistanceTo(INavMeshArea *area) = 0;

	virtual bool BeliefLoad() { return false; };
	virtual bool BeliefSave(bool bOverride = false) { return false; };
	virtual void Belief(Vector vOrigin, Vector vOther, float fBelief, float fStrength, BotBelief iType) = 0;
	virtual void BeliefOne(int iAreaID, BotBelief iBeliefType, float fDist) { return; }
	virtual float GetBelief(int index) { return 0; }

	inline Vector GetGoalOrigin() { return m_vGoal; }

	float GetGoalDistance() { return m_fGoalDistance; }

	bool GetDangerPoint(Vector *vec) { *vec = m_bDangerPoint ? m_vDangerPoint : Vector(0, 0, 0); return m_bDangerPoint; }

	virtual bool CalculateAimVector(QAngle *qAim) = 0;

	virtual INavMeshArea *GetArea(const Vector &vPos, float fBeneathLimit = 120.0f) = 0;
	virtual INavMeshArea *GetNearestArea(const Vector &vPos, bool bAnyZ = false, float fMaxDist = 10000.0f, bool bCheckLOS = false, bool bCheckGround = true, int iTeam = -2) = 0;
	virtual INavMeshArea *GetAreaByID(const unsigned int iAreaIndex) = 0;

	virtual int WorldToGridX(float flWX) = 0;
	virtual int WorldToGridY(float flWY) = 0;

	virtual IList<INavMeshArea*> *CollectSurroundingAreas(INavMeshArea *fromArea, float fTravelDistanceLimit = 1500.0f, float fMaxStepUpLimit = 18.0f, float fMaxDropDownLimit = 100.0f) = 0;
	virtual IList<INavMeshArea*> *GetAdjacentList(INavMeshArea *fromArea, eNavDir iDirection) = 0;
	virtual IList<INavMeshLadder*> *GetLadderList(INavMeshArea *fromArea, eNavLadderDir iLadderDirection) = 0;
	virtual IList<INavMeshArea*> *GetAreasOnGrid(int x, int y) = 0;

	virtual bool IsWalkableTraceLineClear(Vector vFrom, Vector vTo, unsigned int iFlags) = 0;

	bool WantToLoadBelief() { return m_bLoadBelief; }
	virtual bool WantToSaveBelief() { return false; }

	static const int MAX_PATH_TICKS = 200;

protected:
	Vector m_vGoal;
	float m_fGoalDistance;
	Vector m_vPreviousPoint;
	Vector m_vDangerPoint;
	bool m_bDangerPoint;
	short int m_iBeliefTeam;
	bool m_bBeliefChanged;
	bool m_bLoadBelief;
};

#define FL_ASTAR_CLOSED		1
#define FL_ASTAR_PARENT		2
#define FL_ASTAR_OPEN		4
#define FL_HEURISTIC_SET	8

class AStarNode
{
public:
	AStarNode() { memset(this, 0, sizeof(AStarNode)); }
	///////////////////////////////////////////////////////
	inline void Close() { SetFlag(FL_ASTAR_CLOSED); }
	inline void UnClose() { RemoveFlag(FL_ASTAR_CLOSED); }
	inline bool IsOpen() { return HasFlag(FL_ASTAR_OPEN); }
	inline void UnOpen() { RemoveFlag(FL_ASTAR_OPEN); }
	inline bool IsClosed() { return HasFlag(FL_ASTAR_CLOSED); }
	inline void Open() { SetFlag(FL_ASTAR_OPEN); }
	//////////////////////////////////////////////////////	
	inline void SetHeuristic(float fHeuristic) { m_fHeuristic = fHeuristic; SetFlag(FL_HEURISTIC_SET); }
	inline bool HeuristicSet() { return HasFlag(FL_HEURISTIC_SET); }
	inline const float GetHeuristic() { return m_fHeuristic; } const

	////////////////////////////////////////////////////////
	inline void SetFlag(int iFlag) { m_iFlags |= iFlag; }
	inline bool HasFlag(int iFlag) { return ((m_iFlags & iFlag) == iFlag); }
	inline void RemoveFlag(int iFlag) { m_iFlags &= ~iFlag; }
	/////////////////////////////////////////////////////////
	inline int GetParent() { if (HasFlag(FL_ASTAR_PARENT)) return m_iParent; else return -1; }
	inline void SetParent(short int iParent)
	{
		m_iParent = iParent;

		if (m_iParent == -1)
			RemoveFlag(FL_ASTAR_PARENT); // no parent
		else
			SetFlag(FL_ASTAR_PARENT);
	}
	////////////////////////////////////////////////////////
	inline const float GetCost() { return m_fCost; } const
	inline void SetCost(float fCost) { m_fCost = fCost; }
	////////////////////////////////////////////////////////
	// for comparison
	bool Precedes(AStarNode *other) const
	{
		return (m_fCost + m_fHeuristic) < (other->GetCost() + other->GetHeuristic());
	}
	void SetWaypoint(int iWpt) { m_iWaypoint = iWpt; }
	inline int GetWaypoint() { return m_iWaypoint; }
private:
	float m_fCost;
	float m_fHeuristic;
	unsigned char m_iFlags;
	short int m_iParent;
	int m_iWaypoint;
};
// Insertion sorted list
class AStarListNode
{
public:
	AStarListNode(AStarNode *data)
	{
		m_Data = data;
		m_Next = NULL;
	}
	AStarNode *m_Data;
	AStarListNode *m_Next;
};

class AStarOpenList
{
public:
	AStarOpenList()
	{
		m_Head = NULL;
	}

	bool Empty()
	{
		return (m_Head == NULL);
	}

	AStarNode *Top()
	{
		if (m_Head == NULL)
			return NULL;

		return m_Head->m_Data;
	}

	void Pop()
	{
		if (m_Head != NULL)
		{
			AStarListNode *t = m_Head;

			m_Head = m_Head->m_Next;

			delete t;
		}
	}


	void Add(AStarNode *data)
	{
		AStarListNode *newNode = new AStarListNode(data);
		AStarListNode *t;
		AStarListNode *p;

		if (m_Head == NULL)
			m_Head = newNode;
		else
		{
			if (data->Precedes(m_Head->m_Data))
			{
				newNode->m_Next = m_Head;
				m_Head = newNode;
			}
			else
			{
				p = m_Head;
				t = m_Head->m_Next;

				while (t != NULL)
				{
					if (data->Precedes(t->m_Data))
					{
						p->m_Next = newNode;
						newNode->m_Next = t;
						break;
					}

					p = t;
					t = t->m_Next;
				}

				if (t == NULL)
					p->m_Next = newNode;

			}
		}
	}

	void Destroy()
	{
		AStarListNode *t;

		while (m_Head != NULL)
		{
			t = m_Head;
			m_Head = m_Head->m_Next;
			delete t;
			t = NULL;
		}

		m_Head = NULL;
	}

private:
	AStarListNode *m_Head;
};

/*struct AstarNodeCompare : binary_function<AStarNode*, AStarNode*, bool>
{
	// Other stuff...
	bool operator()(AStarNode* x, AStarNode* y) const
	{
		return y->betterCost(x);
	}
};*/

/*class AStarOpenList : public vector<AStarNode*>
{
	AstarNodeCompare comp;
public:
	AStarOpenList(AstarNodeCompare cmp = AstarNodeCompare()) : comp(cmp)
	{
		make_heap(begin(), end(), comp);
	}
	AStarNode* top() { return front(); }
	void push(AStarNode* x)
	{
		push_back(x);
		push_heap(begin(), end(), comp);
	}
	void pop()
	{
		pop_heap(begin(), end(), comp);
		pop_back();
	}

	bool operator<( const AStarNode & A, const AStarNode & B )
	{
		return A.betterCost(&B);
	}

	bool operator<( const AStarNode * A, const AStarNode * B )
	{
		return A->betterCost(B);
	}
};*/

#define WPT_SEARCH_AVOID_SENTRIES 1
#define WPT_SEARCH_AVOID_SNIPERS 2
#define WPT_SEARCH_AVOID_TEAMMATE 4

typedef struct
{
	short int iFrom;
	short int iTo;
	bool bValid;
	bool bSkipped;
}failedpath_t;

class CWaypointNavigator : public IBotNavigator
{
public:
	CWaypointNavigator(CBot *pBot)
	{
		Init();
		m_pBot = pBot;
		m_fNextClearFailedGoals = 0;
		m_bDangerPoint = false;
		m_iBeliefTeam = -1;
		m_bLoadBelief = true;
		m_bBeliefChanged = false;
		memset(&m_lastFailedPath, 0, sizeof(failedpath_t));
	}

	void Init();

	CWaypoint *ChooseBestFromBelief(dataUnconstArray<CWaypoint*> *goals, bool bHighDanger = false, int iSearchFlags = 0, int iTeam = 0);
	CWaypoint *ChooseBestFromBeliefBetweenAreas(dataUnconstArray<AStarNode*> *goals, bool bHighDanger = false, bool bIgnoreBelief = false);

	float GetNextYaw();

	bool WorkRoute(Vector vFrom, Vector vTo, bool *bFail, bool bRestart = true, bool bNoInterruptions = false, int iGoalId = -1, int iConditions = 0, int iDangerId = -1);

	bool GetNextRoutePoint(Vector *vPoint);

	void Clear();

	Vector GetNextPoint();

	void UpdatePosition();

	float GetBelief(int index) { if (index >= 0) return m_fBelief[index]; return 0; }

	void FailMove();

	bool HasNextPoint();

	void FreeMapMemory();

	void FreeAllMemory();

	bool CanGetTo(Vector vOrigin);

	bool RouteFound();

	void RollBackPosition();

	bool NextPointIsOnLadder();

	void Open(AStarNode *pNode);

	AStarNode *NextNode();

	float DistanceTo(Vector vOrigin);

	float DistanceTo(CWaypoint *pWaypoint);

	Vector GetCoverOrigin(Vector vCover);

	void ClearOpenList();

	float GetCurrentBelief();

	//virtual void GoBack();

	void Belief(Vector origin, Vector vOther, float fBelief, float fStrength, BotBelief iType);

	void BeliefOne(int iWptIndex, BotBelief iBeliefType, float fDist);

	// nearest cover position to vOrigin only
	bool GetCoverPosition(Vector vCoverOrigin, Vector *vCover);
	// nearest cover postion to both vectors
	bool GetHideSpotPosition(Vector vCoverOrigin, Vector *vCover);

	void GetFailedGoals(dataUnconstArray <int> **goals) { *goals = &m_iFailedGoals; }

	int numPaths();

	Vector GetPath(int pathid);

	bool RandomDangerPath(Vector *vec);

	bool BeliefLoad();

	bool BeliefSave(bool bOverride = false);

	bool WantToSaveBelief();

	inline int GetCurrentWaypointID()
	{
		return m_iCurrentWaypoint;
	}

	inline int GetCurrentGoalID()
	{
		return m_iGoalWaypoint;
	}

	int GetCurrentFlags();
	int GetPathFlags(int iPath);

private:
	CBot *m_pBot;

	//CWaypointVisibilityTable *m_pDangerNodes;

	//int m_iPrevWaypoint;
	int m_iCurrentWaypoint;
	int m_iPrevWaypoint;
	int m_iNextWaypoint;
	int m_iGoalWaypoint;
	bool m_bWorkingRoute;

	failedpath_t m_lastFailedPath;

	dataStack<int> m_currentRoute;
	queue<int> m_oldRoute;

	int m_iLastFailedWpt;

	AStarNode paths[CWaypoints::MAX_WAYPOINTS];
	AStarNode *curr;
	AStarNode *succ;

	dataUnconstArray<int> m_iFailedGoals;
	float m_fNextClearFailedGoals;

	float m_fBelief[CWaypoints::MAX_WAYPOINTS];

	AStarOpenList m_theOpenList;

	Vector m_vOffset;
	bool m_bOffsetApplied;
};

#endif