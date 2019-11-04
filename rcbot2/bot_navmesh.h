#ifndef __RCBOT_NAVMESH_H__
#define __RCBOT_NAVMESH_H__

#include "bot_navigator.h"
#include "bot_getprop.h"
#include "bot_profile.h"
#include "../NavMesh/List.h"
#include "../NavMesh/NavMesh.h"
#include "../NavMesh/NavMeshArea.h"
#include "../NavMesh/NavHintType.h"
#include "../NavMesh/NavMeshGrid.h"
#include "../NavMesh/NavMeshLoader.h"

constexpr float StepHeight = 18.0f;					/*< if delta Z is greater than this, we have to jump to get up*/
constexpr float JumpHeight = 41.8f;					/*< if delta Z is less than this, we can jump up on it*/
constexpr float JumpCrouchHeight = 58.0f;			/*< if delta Z is less than or equal to this, we can jump-crouch up on it*/

constexpr float DeathDrop = 200.0f;					/*< distance at which we will die if we fall - should be about 600, and pay attention to fall damage during pathfind*/

constexpr float HalfHumanWidth = 16.0f;
constexpr float HalfHumanHeight = 36.0f;
constexpr float HumanHeight = 72.0f;
constexpr float HumanCrouchHeight = 56.0f;

enum eNavAttrib : uint
{
	NAV_MESH_INVALID		= 0,
	NAV_MESH_CROUCH			= 0x00000001,			/*< must crouch to use this node/area*/
	NAV_MESH_JUMP			= 0x00000002,			/*< must jump to traverse this area (only used during generation)*/
	NAV_MESH_PRECISE		= 0x00000004,			/*< do not adjust for obstacles, just move along area*/
	NAV_MESH_NO_JUMP		= 0x00000008,			/*< inhibit discontinuity jumping*/
	NAV_MESH_STOP			= 0x00000010,			/*< must stop when entering this area*/
	NAV_MESH_RUN			= 0x00000020,			/*< must run to traverse this area*/
	NAV_MESH_WALK			= 0x00000040,			/*< must walk to traverse this area*/
	NAV_MESH_AVOID			= 0x00000080,			/*< avoid this area unless alternatives are too dangerous*/
	NAV_MESH_TRANSIENT		= 0x00000100,			/*< area may become blocked, and should be periodically checked*/
	NAV_MESH_DONT_HIDE		= 0x00000200,			/*< area should not be considered for hiding spot generation*/
	NAV_MESH_STAND			= 0x00000400,			/*< bots hiding in this area should stand*/
	NAV_MESH_NO_HOSTAGES	= 0x00000800,			/*< hostages shouldn't use this area*/
	NAV_MESH_STAIRS			= 0x00001000,			/*< this area represents stairs, do not attempt to climb or jump them - just walk up*/
	NAV_MESH_NO_MERGE		= 0x00002000,			/*< don't merge this area with adjacent areas*/
	NAV_MESH_OBSTACLE_TOP	= 0x00004000,			/*< this nav area is the climb point on the tip of an obstacle*/
	NAV_MESH_CLIFF			= 0x00008000,			/*< this nav area is adjacent to a drop of at least CliffHeight*/

	NAV_MESH_FIRST_CUSTOM	= 0x00010000,			/*< apps may define custom app-specific bits starting with this value*/

	NAV_MESH_SPAWN_ROOM		= 0x00020000,
	NAV_MESH_CONTROL_POINT	= 0x00040000,
	NAV_MESH_BOMB_DROP		= 0x00080000,

	NAV_MESH_LAST_CUSTOM	= 0x04000000,			/*< apps must not define custom app-specific bits higher than with this value*/

	NAV_MESH_FUNC_COST		= 0x20000000,			/*< area has designer specified cost controlled by func_nav_cost entities*/
	NAV_MESH_HAS_ELEVATOR	= 0x40000000,			/*< area is in an elevator's path*/
	NAV_MESH_NAV_BLOCKER	= 0x80000000			/*< area is blocked by nav blocker*/
};

#if SOURCE_ENGINE == SE_TF2
enum eTFNavAttrib : uint
{
	BLOCKED                     = (1 << 0),

	RED_SPAWN_ROOM              = (1 << 1),
	BLUE_SPAWN_ROOM             = (1 << 2),
	SPAWN_ROOM_EXIT             = (1 << 3),

	AMMO                        = (1 << 4),
	HEALTH                      = (1 << 5),

	CONTROL_POINT               = (1 << 6),

	BLUE_SENTRY                 = (1 << 7),
	RED_SENTRY                  = (1 << 8),

	/* bit  9: unused */
	/* bit 10: unused */

	BLUE_SETUP_GATE             = (1 << 11),
	RED_SETUP_GATE              = (1 << 12),

	BLOCKED_AFTER_POINT_CAPTURE = (1 << 13),
	BLOCKED_UNTIL_POINT_CAPTURE = (1 << 14),

	BLUE_ONE_WAY_DOOR           = (1 << 15),
	RED_ONE_WAY_DOOR            = (1 << 16),

	WITH_SECOND_POINT           = (1 << 17),
	WITH_THIRD_POINT            = (1 << 18),
	WITH_FOURTH_POINT           = (1 << 19),
	WITH_FIFTH_POINT            = (1 << 20),

	SNIPER_SPOT                 = (1 << 21),
	SENTRY_SPOT                 = (1 << 22),

	/* bit 23: unused */
	/* bit 24: unused */

	NO_SPAWNING                 = (1 << 25),
	RESCUE_CLOSET               = (1 << 26),
	BOMB_DROP                   = (1 << 27),
	DOOR_NEVER_BLOCKS           = (1 << 28),
	DOOR_ALWAYS_BLOCKS          = (1 << 29),
	UNBLOCKABLE                 = (1 << 30),

	/* bit 31: unused */
};
#endif

enum eHidingAttrib
{
	IN_COVER			= 0x01,						/*< in a corner with good hard cover nearby*/
	GOOD_SNIPER_SPOT	= 0x02,						/*< had at least one decent sniping corridor*/
	IDEAL_SNIPER_SPOT	= 0x04,						/*< can see either very far, or a large area, or both*/
	EXPOSED				= 0x08						/*< spot in the open, usually on a ledge or cliff*/
};

enum eRouteType
{
	FASTEST_ROUTE,
	SAFEST_ROUTE,
};

abstract_class IPathCost
{
public:
	virtual float operator() (INavMeshArea *, INavMeshArea *, INavMeshLadder *) = 0;
};

extern INavMesh *g_pNavMesh;

// Most of this converted from work by Kit o' Rifty and https://github.com/VSES/SourceEngine2007
class CNavMeshNavigator : public IBotNavigator
{
public:
	CNavMeshNavigator(CBot *pBot) : m_pBot(pBot)
	{
		m_bLoadBelief = true;
		m_bBeliefChanged = false;
	}

	void Init();
	void FreeAllMemory();
	void FreeMapMemory();

	bool WorkRoute(Vector vFrom, Vector vTo, bool *bFail, bool bRestart = true, bool bNoInterruptions = false, int iGoalID = -1, int iConditions = 0, int iDangerId = -1);
	bool WorkRoute(Vector vFrom, Vector vTo, bool *bFail, bool bRestart = true, bool bNoInterruptions = false, INavMeshArea *goal = NULL, int iConditions = 0, int iDangerId = -1);

	bool ComputePathPositions();

	INavMeshArea *ChooseBestFromBelief(CList<INavMeshArea*> *goals, bool bHighDanger = false, int iSearchFlags = 0, int iTeam = 0);
	INavMeshArea *ChooseBestFromBeliefBetweenAreas(CList<INavMeshArea*> *goals, bool bHighDanger = false, bool bIgnoreBelief = false);

	bool GetNextRoutePoint(Vector *vPoint);

	void Clear();

	Vector GetNextPoint();

	float GetNextYaw();

	void UpdatePosition();

	void FailMove() { }

	bool HasNextPoint();

	bool NextPointIsOnLadder();

	bool CanGetTo(Vector vOrigin);

	bool RouteFound();

	void RollBackPosition();

	float DistanceTo(Vector vOrigin);
	float DistanceTo(INavMeshArea *area);

	void GetFailedGoals(dataUnconstArray <int> **goals) { }

	// nearest cover position to vOrigin only
	bool GetCoverPosition(Vector vCoverOrigin, Vector *vCover);
	// nearest cover postion to both vectors
	bool GetHideSpotPosition(Vector vCoverOrigin, Vector *vCover);

	inline INavMeshArea *GetCurrentWaypoint() { return m_CurrWaypoint; }
	inline INavMeshArea *GetCurrentGoal() { return m_GoalWaypoint; }

	inline int GetCurrentWaypointID() { return m_CurrWaypoint->GetID(); }
	inline int GetCurrentGoalID() { return m_GoalWaypoint->GetID(); }

	int GetCurrentFlags();

	void SetPathIndex(const int iIndex);
	bool FindClosestPointOnPath(const Vector &vOurPos, int startIndex, int endIndex, Vector *vClose);
	int FindOurPositionOnPath(Vector *vClose, bool bLocal);
	int FindPathPoint(float fAheadRange, Vector *vPoint, int *prevIndex);

	const bool IsNearJump() const;

	bool BeliefLoad();
	bool BeliefSave(bool bOverride = false);
	bool WantToSaveBelief();
	void Belief(Vector origin, Vector vOther, float fBelief, float fStrength, BotBelief iType);
	void BeliefOne(int iAreaID, BotBelief iBeliefType, float fDist);
	float GetCurrentBelief();
	float GetBelief(int iAreaID);

	bool ComputeAimVector(QAngle *angAim);

#if (SOURCE_ENGINE == SE_TF2)
	INavMeshArea *RandomGoalNearestArea(Vector &origin, bool bHighDanger = false, bool bIgnoreBelief = false, unsigned int iFlags = 0, int iTeam = -2, unsigned int tfFlags = 0);
	INavMeshArea *RandomGoalBetweenAreas(Vector &org1, Vector &org2, bool bHighDanger = false, bool bIgnoreBelief = false, unsigned int iFlags = 0, int iTeam = -2, unsigned int tfFlags = 0);
#else
	INavMeshArea *RandomGoalNearestArea(Vector &origin, bool bHighDanger = false, bool bIgnoreBelief = false, unsigned int iFlags = 0, int iTeam = -2);
	INavMeshArea *RandomGoalBetweenAreas(Vector &org1, Vector &org2, bool bHighDanger = false, bool bIgnoreBelief = false, unsigned int iFlags = 0, int iTeam = -2);
#endif

	bool BuildPath(INavMeshArea *startArea, INavMeshArea *goalArea, const Vector *vGoalPos, IPathCost &func, INavMeshArea **closestArea = NULL, bool bIgnoreBlockedNav = false, float fMaxPathLength = 0.0f, float fMaxAng = 0.0f);

	INavMeshArea *GetArea(const Vector &vPos, float fBeneathLimit = 120.0f);
	INavMeshArea *GetNearestArea(const Vector &vPos, bool bAnyZ = false, float fMaxDist = 10000.0f, bool bCheckLOS = false, bool bCheckGround = true, int iTeam = -2);
	INavMeshArea *GetAreaByID(const unsigned int iAreaIndex);
	
	Vector GetClosestPointOnArea(INavMeshArea *area, const Vector &vPos);
	bool GetGroundHeight(const Vector vPos, float *fHeight, Vector *vNormal = 0);
	// lightweight version of GetGroundHeight
	bool GetSimpleGroundHeight(const Vector vPos, float *fHeight, Vector *vNormal = 0);

	bool IsConnected(INavMeshArea *fromArea, INavMeshArea *toArea);

	bool IsUnderwater(INavMeshArea *area);
	
	CList<INavMeshArea*> CollectSurroundingAreas(INavMeshArea *fromArea, float fTravelDistanceLimit = 1500.0f, float fMaxStepUpLimit = StepHeight, float fMaxDropDownLimit = DeathDrop);
	CList<INavMeshArea*> GetAdjacentList(INavMeshArea *fromArea, eNavDir iDirection);
	CList<INavMeshLadder*> GetLadderList(INavMeshArea *fromArea, eNavLadderDir iLadderDirection);
	
	bool IsAreaOverlapping(INavMeshArea *area, const Vector vPos, float fTolerance = 0.0f);
	bool IsAreaOverlapping(INavMeshArea *fromArea, INavMeshArea *toArea);
	bool AreaContains(INavMeshArea *area, const Vector vPos);
	bool AreaIsEdge(INavMeshArea *area, eNavDir iDirection);

	float GetZ(INavMeshArea *area, const Vector &vPos);
	float GetZ(INavMeshArea *area, float fX, float fY);

	bool IsAreaVisibleToEnemy(INavMeshArea *area, int iTeam);

	bool ComputePortal(INavMeshArea *fromArea, INavMeshArea *toArea, eNavDir iDirection, Vector *vCenter, float *fHalfWidth);
	bool ComputeClosestPointInPortal(INavMeshArea *fromArea, INavMeshArea *toArea, eNavDir iNavDirection, const Vector &vFromPos, Vector *vClosestPos);
	bool ComputeNormal(INavMeshArea *area, Vector *vNormal, bool bAlternate);
	bool ComputeDirection(INavMeshArea *area, const Vector &vPos, eNavDir *iDirection);

	float GetLightIntensity(INavMeshArea *area, const Vector &vPos);

	float ComputeAdjacentAreaHeightChange(INavMeshArea *fromArea, INavMeshArea *toArea);
	eNavDir OppositeDirection(eNavDir iDirection);
	float DirectionToAngle(eNavDir iDirection);
	eNavDir AngleToDirection(float fAngle);
	void AddDirectionVector(Vector *v, eNavDir iDirection, float fAmount);
	void DirectionToVector(eNavDir iDirection, Vector *v);

	Vector GetLadderNormal(INavMeshLadder *ladder);

	bool IsWalkableTraceLineClear(Vector vFrom, Vector vTo, unsigned int iFlags);

	void AddAreaToOpenList(INavMeshArea *area, INavMeshArea *parent, const Vector &vStart, float fMaxRange = -1.0f);

	float TravelDistance(const Vector &vStart, const Vector &vGoal, IPathCost &func);
	float TravelDistance(INavMeshArea *fromArea, INavMeshArea *toArea, IPathCost &func);

	void DrawPath();

private:
	CBot *m_pBot;

	INavMeshArea *m_CurrWaypoint; // this is actually our next goal
	INavMeshArea *m_PrevWaypoint; // this is actually where we currently are
	INavMeshArea *m_GoalWaypoint;

	float *m_fBelief;
	float *m_fDanger;

	bool m_bWorkingRoute;

	Vector m_vOffset;
	bool m_bOffsetApplied;

	enum { MAX_SEGMENT_COUNT = 256 };
	struct Segment
	{
		INavMeshArea *area;
		INavMeshLadder *ladder;
		Vector pos;
	}
	m_Path[MAX_SEGMENT_COUNT];
	int m_iSegments;
	
	int m_nPathIndex;
};

#define WALK_THRU_PROP_DOORS		0x01
#define WALK_THRU_FUNC_DOORS		0x02
#define WALK_THRU_DOORS				(WALK_THRU_PROP_DOORS | WALK_THRU_FUNC_DOORS)
#define WALK_THRU_BREAKABLES		0x04
#define WALK_THRU_TOGGLE_BRUSHES	0x08
#define WALK_THRU_EVERYTHING		(WALK_THRU_DOORS | WALK_THRU_BREAKABLES | WALK_THRU_TOGGLE_BRUSHES)

extern bool IsEntityWalkable(CBaseEntity *pEntity, unsigned int flags);

class ShortestPathCost : public IPathCost
{
public:
	float operator() (INavMeshArea *toArea, INavMeshArea *fromArea, INavMeshLadder *ladder)
	{
		if(fromArea == NULL)
		{
			// first area in path, no cost
			return 0.0f;
		}
		else
		{
			float fDist;
			if(ladder != NULL)
				fDist = ladder->GetLength();
			else
				fDist = (toArea->GetCenter() - fromArea->GetCenter()).Length();

			float fCost = fDist + toArea->GetCostSoFar();

			const float fCrouchPenalty = 20.0f;
			const float fJumpPenalty = 5.0f;

			if(toArea->GetAttributes() & NAV_MESH_CROUCH)
				fCost += fCrouchPenalty * fDist;

			if(toArea->GetAttributes() & NAV_MESH_JUMP)
				fCost += fJumpPenalty * fDist;

			return fCost;
		}
	}
};

#endif // __RCBOT_NAVMESH_H__