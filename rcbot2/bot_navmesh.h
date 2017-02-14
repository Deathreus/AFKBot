#ifndef __RCBOT_NAVMESH_H__
#define __RCBOT_NAVMESH_H__

#include "bot_navigator.h"
#include "bot_getprop.h"
#include "server_class.h"
#include "engine_wrappers.h"
#include "..\NavMesh\List.h"
#include "..\NavMesh\NavMesh.h"
#include "..\NavMesh\NavMeshArea.h"
#include "..\NavMesh\NavMeshGrid.h"
#include "..\NavMesh\NavMeshLoader.h"

const float StepHeight = 18.0f;					/*< if delta Z is greater than this, we have to jump to get up*/
const float JumpHeight = 41.8f;					/*< if delta Z is less than this, we can jump up on it*/
const float JumpCrouchHeight = 58.0f;			/*< (48) if delta Z is less than or equal to this, we can jumpcrouch up on it*/

const float DeathDrop = 200.0f;					/*< (300) distance at which we will die if we fall - should be about 600, and pay attention to fall damage during pathfind*/

const float HalfHumanWidth = 16.0f;
const float HalfHumanHeight = 36.0f;
const float HumanHeight = 72.0f;

const float GridCellSize = 300.0f;				/* defines the extent for a single grid blocks size, used for collecting nav areas*/

typedef IList<INavMeshArea*> SurroundingList;
typedef IList<INavMeshArea*> AdjacentList;
typedef IList<INavMeshLadder*> LadderList;
typedef IList<INavMeshArea*> GridList;

enum eNavAttrib
{
	NAV_MESH_CROUCH = 0x0001,			/*< must crouch to use this node/area*/
	NAV_MESH_JUMP = 0x0002,				/*< must jump to traverse this area*/
	NAV_MESH_PRECISE = 0x0004,			/*< do not adjust for obstacles, just move along area*/
	NAV_MESH_NO_JUMP = 0x0008,			/*< inhibit discontinuity jumping*/
	NAV_MESH_STOP = 0x0010,				/*< must stop when entering this area*/
	NAV_MESH_RUN = 0x0020,				/*< must run to traverse this area*/
	NAV_MESH_WALK = 0x0040,				/*< must walk to traverse this area*/
	NAV_MESH_AVOID = 0x0080,			/*< avoid this area unless alternatives are too dangerous*/
	NAV_MESH_TRANSIENT = 0x0100,		/*< area may become blocked, and should be periodically checked*/
	NAV_MESH_DONT_HIDE = 0x0200,		/*< area should not be considered for hiding spot generation*/
	NAV_MESH_STAND = 0x0400,			/*< bots hiding in this area should stand*/
	NAV_MESH_NO_HOSTAGES = 0x0800,		/*< hostages shouldn't use this area*/ /*< used in the extension to determine special lifts*/
};

enum eHidingAttrib
{
	IN_COVER = 0x01,					/*< in a corner with good hard cover nearby*/
	GOOD_SNIPER_SPOT = 0x02,			/*< had at least one decent sniping corridor*/
	IDEAL_SNIPER_SPOT = 0x04,			/*< can see either very far, or a large area, or both*/
	EXPOSED = 0x08						/*< spot in the open, usually on a ledge or cliff*/
};

enum eRouteType
{
	FASTEST_ROUTE,
	SAFEST_ROUTE,
};

extern IServerGameEnts *gameents;

extern INavMesh *m_pNavMesh;

// Most of this converted from work by Kit o' Rifty and the Source SDK
class CNavMeshNavigator : public INavMeshNavigator
{
public:
	CNavMeshNavigator(CBot *pBot)
	{
		m_pBot = pBot;
		m_CurrWaypoint = NULL;
		m_GoalWaypoint = NULL;
		m_PrevWaypoint = NULL;
		m_NextWaypoint = NULL;
		m_bWorkingRoute = false;
		m_iBeliefTeam = -1;
		m_bLoadBelief = true;
		m_bBeliefChanged = false;
		Q_memset(&m_fBelief, 0, sizeof(float)*m_pNavMesh->GetAreas()->Size());
		Q_memset(&m_fDanger, 0, sizeof(float)*m_pNavMesh->GetAreas()->Size());
	}
	~CNavMeshNavigator()
	{
		m_pBot = NULL;
		Q_memset(&m_fBelief, 0, sizeof(float)*m_pNavMesh->GetAreas()->Size());
		Q_memset(&m_fDanger, 0, sizeof(float)*m_pNavMesh->GetAreas()->Size());
	}

	static bool Init(char *error, size_t maxlen);
	static void FreeMemory();
	void FreeMapMemory();

	bool WorkRoute(Vector vFrom, Vector vTo, bool *bFail, bool bRestart = true, bool bNoInterruptions = false, INavMeshArea *goal = NULL, int iConditions = 0, int iDangerId = -1);

	INavMeshArea *ChooseBestFromBelief(IList<INavMeshArea*> *goals, bool bHighDanger = false, int iSearchFlags = 0, int iTeam = 0);
	INavMeshArea *ChooseBestFromBeliefBetweenAreas(IList<INavMeshArea*> *goals, bool bHighDanger = false, bool bIgnoreBelief = false);

	bool GetNextRoutePoint(Vector *vPoint);

	void Clear();

	Vector GetNextPoint();

	void UpdatePosition();

	void FailMove();

	bool HasNextPoint();

	bool NextPointIsOnLadder();

	bool CanGetTo(Vector vOrigin);

	bool RouteFound();

	void RollBackPosition();

	float DistanceTo(Vector vOrigin);
	float DistanceTo(INavMeshArea *area);

	// nearest cover position to vOrigin only
	bool GetCoverPosition(Vector vCoverOrigin, Vector *vCover);
	// nearest cover postion to both vectors
	bool GetHideSpotPosition(Vector vCoverOrigin, Vector *vCover);

	inline INavMeshArea *GetCurrentWaypoint() { return m_CurrWaypoint; }
	inline INavMeshArea *GetCurrentGoal() { return m_GoalWaypoint; }

	int GetCurrentFlags();

	bool BeliefLoad();
	bool BeliefSave(bool bOverride = false);
	bool WantToSaveBelief();
	void Belief(Vector origin, Vector vOther, float fBelief, float fStrength, BotBelief iType);
	void BeliefOne(int iAreaID, BotBelief iBeliefType, float fDist);
	float GetCurrentBelief();
	float GetBelief(int iAreaID);

	bool CalculateAimVector(QAngle *vAim);

	static INavMeshArea *RandomGoalNearestArea(unsigned int iFlags, int iTeam, int iArea, bool bForceArea, CBot *pBot, bool bHighDanger, Vector *origin, int iIgnore, bool bIgnoreBelief, int iWpt);
	static INavMeshArea *RandomGoalBetweenAreas(unsigned int iFlags, int iTeam, int iArea, bool bForceArea, CBot *pBot, bool bHighDanger, Vector *org1, Vector *org2, bool bIgnoreBelief, int iWpt1, int iWpt2);

	template< typename CostFunctor >
	bool BuildPath(INavMeshArea *startArea, INavMeshArea *goalArea, const Vector *vGoalPos, CostFunctor &func, INavMeshArea **closestArea = NULL, float fMaxPathLength = 0.0f, float fMaxAng = 0.0f, bool bIgnoreBlockedNav = false);

	INavMeshArea *GetArea(const Vector &vPos, float fBeneathLimit = 120.0f);
	INavMeshArea *GetNearestArea(const Vector &vPos, bool bAnyZ = false, float fMaxDist = 10000.0f, bool bCheckLOS = false, bool bCheckGround = true, int iTeam = -2);
	INavMeshArea *GetAreaByID(const unsigned int iAreaIndex);
	
	int WorldToGridX(float flWX);
	int WorldToGridY(float flWY);

	static Vector GetExtentLow(INavMeshArea *area);
	static Vector GetExtentHigh(INavMeshArea *area);
	static Vector GetCenter(INavMeshArea *area);
	static Vector GetClosestPointOnArea(INavMeshArea *area, const Vector &vPos);
	static bool GetGroundHeight(const Vector vPos, float *fHeight, Vector *vNormal);

	bool IsConnected(INavMeshArea *fromArea, INavMeshArea *toArea);
	
	SurroundingList *CollectSurroundingAreas(INavMeshArea *fromArea, float fTravelDistanceLimit = 1500.0f, float fMaxStepUpLimit = StepHeight, float fMaxDropDownLimit = DeathDrop);
	AdjacentList *GetAdjacentList(INavMeshArea *fromArea, eNavDir iDirection);
	LadderList *GetLadderList(INavMeshArea *fromArea, eNavLadderDir iLadderDirection);
	GridList *GetAreasOnGrid(int x, int y);
	
	bool IsOverlapping(INavMeshArea *area, const Vector vPos, float fTolerance = 0.0f);
	bool IsOverlapping(INavMeshArea *fromArea, INavMeshArea *toArea);
	bool Contains(INavMeshArea *area, const Vector vPos);
	bool IsEdge(INavMeshArea *area, eNavDir iDirection);

	static float GetZ(INavMeshArea *area, const Vector &vPos);
	static float GetZ(INavMeshArea *area, float flX, float flY);

	bool ComputePortal(INavMeshArea *fromArea, INavMeshArea *toArea, eNavDir iDirection, Vector *vCenter, float *fHalfWidth);
	bool ComputeClosestPointInPortal(INavMeshArea *fromArea, INavMeshArea *toArea, eNavDir iNavDirection, const Vector &vFromPos, Vector *vClosestPos);
	bool ComputeNormal(INavMeshArea *area, Vector *vNormal, bool bAlternate);
	bool ComputeDirection(INavMeshArea *area, const Vector &vPos, eNavDir *iDirection);

	float GetLightIntensity(INavMeshArea *area, const Vector &vPos);

	float ComputeAdjacentAreaHeightChange(INavMeshArea *fromArea, INavMeshArea *toArea);
	eNavDir OppositeDirection(eNavDir iDirection);
	float DirectionToAngle(eNavDir iDirection);
	eNavDir AngleToDirection(float fAngle);

	bool IsWalkableTraceLineClear(Vector vFrom, Vector vTo, unsigned int iFlags);

	void AddAreaToOpenList(INavMeshArea *area, INavMeshArea *parent, const Vector &vStart, float fMaxRange = -1.0f);

	template< typename CostFunctor >
	float TravelDistance(const Vector &vStart, const Vector &vGoal, CostFunctor &func);
	template< typename CostFunctor >
	float TravelDistance(INavMeshArea *fromArea, INavMeshArea *toArea, CostFunctor &func);

private:
	CBot *m_pBot;

	INavMeshArea *m_CurrWaypoint;
	INavMeshArea *m_PrevWaypoint;
	INavMeshArea *m_NextWaypoint;
	INavMeshArea *m_GoalWaypoint;

	float *m_fBelief;
	float *m_fDanger;

	bool m_bWorkingRoute;

	IList<Vector> *m_vGoals;

	Vector m_vOffset;
	bool m_bOffsetApplied;
};

/**
* Functor used with CNavMeshNavigator::BuildPath()
*/
class ShortestPathCost
{
public:
	float operator() (INavMeshArea *area, INavMeshArea *fromArea, INavMeshLadder *ladder)
	{
		if (fromArea == NULL)
		{
			// first area in path, no cost
			return 0.0f;
		}
		else
		{
			float fDist; // compute distance travelled along path so far
			if (ladder != NULL)
				fDist = ladder->GetLength();
			else
				fDist = (CNavMeshNavigator::GetCenter(area) - CNavMeshNavigator::GetCenter(fromArea)).Length();

			float flCost = fDist + area->GetCostSoFar();

			// if this is a "crouch" area, add penalty
			if (area->GetFlags() & NAV_MESH_CROUCH)
			{
				const float flCrouchPenalty = 20.0f;
				flCost += flCrouchPenalty * fDist;
			}

			// if this is a "jump" area, add penalty
			if (area->GetFlags() & NAV_MESH_JUMP)
			{
				const float flJumpPenalty = 5.0f;
				flCost += flJumpPenalty * fDist;
			}

			return flCost;
		}
	}
};

#define WALK_THRU_PROP_DOORS		0x01
#define WALK_THRU_FUNC_DOORS		0x02
#define WALK_THRU_DOORS				(WALK_THRU_PROP_DOORS | WALK_THRU_FUNC_DOORS)
#define WALK_THRU_BREAKABLES		0x04
#define WALK_THRU_TOGGLE_BRUSHES	0x08
#define WALK_THRU_EVERYTHING		(WALK_THRU_DOORS | WALK_THRU_BREAKABLES | WALK_THRU_TOGGLE_BRUSHES)

inline bool IsEntityWalkable(CBaseEntity *pEntity, unsigned int flags)
{
	const char *name = gamehelpers->GetEntityClassname(pEntity);
	edict_t *pEdict = gameents->BaseEntityToEdict(pEntity);

	if (!strcmp(name, "worldspawn"))
		return false;

	if (!strcmp(name, "player"))
		return false;

	// if we hit a door, assume its walkable because it will open when we touch it
	if (!strstr(name, "prop_door") || !strstr(name, "func_door"))
		return (flags & WALK_THRU_DOORS) ? true : false;

	// if we hit a clip brush, ignore it if it is not BRUSHSOLID_ALWAYS
	if (!strcmp(name, "func_brush"))
	{
		int solidity = CEntData::GetEntData(pEdict, "m_iSolidity");
		switch (solidity)
		{
			case 2:	// SOLID_ALWAYS
				return false;
			case 1:	// SOLID_NEVER
				return true;
			case 0:	// SOLID_TOGGLE
				return (flags & WALK_THRU_TOGGLE_BRUSHES) ? true : false;
		}
	}

	// if we hit a breakable object, assume its walkable because we will shoot it when we touch it
	if (!strcmp(name, "func_breakable"))
	{
		if (CEntData::GetEntData(pEdict, "m_iHealth") && CEntData::GetEntData(pEdict, "m_takedamage") == DAMAGE_YES)
			return (flags & WALK_THRU_BREAKABLES) ? true : false;
	}

	if (!strcmp(name, "func_playerinfected_clip"))
	{
		return true;
	}

	return false;
}

class CTraceFilterNoPlayers : public CTraceFilter
{
public:
	CTraceFilterNoPlayers(const IHandleEntity *pEntity, int collisionGroup)
		: m_pPassEnt(pEntity), m_collisionGroup(collisionGroup) {}

	virtual bool ShouldHitEntity(IHandleEntity *pEntity, int contentsMask)
	{
		edict_t *pEdict = INDEXENT(pEntity->GetRefEHandle().GetEntryIndex());
		if (!pEdict || pEdict->IsFree())
			return false;

		if (pEntity == m_pPassEnt)
			return false;

		IGamePlayer *pPlayer = playerhelpers->GetGamePlayer(pEdict);
		IPlayerInfo *pInfo = pPlayer->GetPlayerInfo();
		if (pPlayer != NULL || (pInfo != NULL && pInfo->IsPlayer()))
			return false;

		return true;
	}

private:
	const IHandleEntity *m_pPassEnt;
	int m_collisionGroup;
};

class CTraceFilterWalkableEntities : public CTraceFilterNoPlayers
{
public:
	CTraceFilterWalkableEntities(const IHandleEntity *passentity, int collisionGroup, unsigned int flags)
		: CTraceFilterNoPlayers(passentity, collisionGroup), m_flags(flags) {}

	virtual bool ShouldHitEntity(IHandleEntity *pEntity, int contentsMask)
	{
		if (CTraceFilterNoPlayers::ShouldHitEntity(pEntity, contentsMask))
		{
			edict_t *pEdict = INDEXENT(pEntity->GetRefEHandle().GetEntryIndex());
			CBaseEntity *pEntity = gameents->EdictToBaseEntity(pEdict);
			return (!IsEntityWalkable(pEntity, m_flags));
		}

		return false;
	}

private:
	unsigned int m_flags;
};

class CTraceFilterCustom : public ITraceFilter
{
public:
	CTraceFilterCustom() {}

	bool ShouldHitEntity(IHandleEntity *entity, int contentmask)
	{
		edict_t *pEdict = INDEXENT(entity->GetRefEHandle().GetEntryIndex());
		if (pEdict == NULL || pEdict->IsFree())
			return false;

		if (ENTINDEX(pEdict)-1 > 0 || ENTINDEX(pEdict)-1 <= gpGlobals->maxClients)
			return false;

		IServerNetworkable *pNetwork = pEdict->GetNetworkable();
		if (pNetwork == NULL)
			return false;

		ServerClass *pClass = pNetwork->GetServerClass();
		if (pClass == NULL)
			return false;

		const char *name = pClass->GetName();
		if (!strcmp(name, "CBaseDoor") || !strcmp(name, "CTFBaseBoss") || !strcmp(name, "CFuncRespawnRoomVisualizer"))
			return false;

		return true;
	}
	virtual TraceType_t GetTraceType() const
	{
		return TRACE_EVERYTHING_FILTER_PROPS;
	}
};

#endif
