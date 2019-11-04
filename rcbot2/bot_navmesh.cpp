//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//
//=============================================================================//
// Navigation Path encapsulation
// Author: Michael S. Booth (mike@turtlerockstudios.com), November 2003
#if defined USE_NAVMESH

#include "bot_base.h"
#include "bot_profile.h"
#include "bot_globals.h"
#include "bot_schedule.h"
#include "bot_navmesh.h"
#include "server_class.h"
#include "engine_wrappers.h"
#include "tier0/vprof.h"

#ifndef __linux__
#include <ndebugoverlay.h>
#endif

#ifdef GetClassName
 #undef GetClassName
#endif

#pragma warning(disable:6001)

extern IServerTools *servertools;
extern IServerGameEnts *gameents;
extern IPlayerInfoManager *playerinfomanager;
extern IEngineTrace *engtrace;

extern ConVar bot_belief_fade;

#define IN_DUCK (1 << 2)

INavMesh *g_pNavMesh;

//static CNavMeshNavigator s_Navigator;
inline bool FuzzyStringMatches(const char *pszNameOrWildcard, const char *pszNameToMatch)
{
	// No string? Well shucks
	if(!pszNameToMatch || *pszNameToMatch == 0)
		return (*pszNameOrWildcard == '*');

	// If the pointers are identical, we're identical
	if(pszNameToMatch == pszNameOrWildcard)
		return true;

	while(*pszNameToMatch && *pszNameOrWildcard)
	{
		unsigned char cName = *pszNameToMatch;
		unsigned char cQuery = *pszNameOrWildcard;
		// simple ascii case conversion
		if(cName == cQuery)
			;
		else if(cName - 'A' <= (unsigned char)'Z' - 'A' && cName - 'A' + 'a' == cQuery)
			;
		else if(cName - 'a' <= (unsigned char)'z' - 'a' && cName - 'a' + 'A' == cQuery)
			;
		else
			break;
		++pszNameToMatch;
		++pszNameOrWildcard;
	}

	if(*pszNameOrWildcard == 0 && *pszNameToMatch == 0)
		return true;

	if(*pszNameOrWildcard == '*')
		return true;

	return false;
}

inline bool IsEntityWalkable(CBaseEntity *pEntity, unsigned int flags)
{
	edict_t *pEdict = gameents->BaseEntityToEdict(pEntity);
	const char *name = pEdict->GetClassName();

	if(!strcmp(name, "worldspawn"))
		return false;

	if(!strcmp(name, "player"))
		return false;

	// if we hit a door, assume its walkable because it will open when we touch it
	if(FuzzyStringMatches("prop_door*", name) || FuzzyStringMatches("func_door*", name))
		return (flags & WALK_THRU_DOORS) ? true : false;

	// if we hit a clip brush, ignore it if it is not BRUSHSOLID_ALWAYS
	if(!strcmp(name, "func_brush"))
	{
		int solidity = GetEntData<int>(pEdict, "m_iSolidity");
		switch(solidity)
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
	if(!strcmp(name, "func_breakable"))
	{
		if(GetEntData<int>(pEdict, "m_iHealth") && GetEntData<byte>(pEdict, "m_takedamage") == DAMAGE_YES)
			return (flags & WALK_THRU_BREAKABLES) ? true : false;
	}

	if(!strcmp(name, "func_playerinfected_clip"))
	{
		return true;
	}

	return false;
}

class CTraceFilterNoPlayers : public ITraceFilter
{
public:
	CTraceFilterNoPlayers(const IHandleEntity *pEntity, int collisionGroup)
		: m_pPassEnt(pEntity), m_collisionGroup(collisionGroup)
	{
	}

	virtual bool ShouldHitEntity(IHandleEntity *pEntity, int contentsMask)
	{
		if(pEntity == m_pPassEnt)
			return false;

		edict_t *pEdict = INDEXENT(pEntity->GetRefEHandle().GetEntryIndex());
		if(!pEdict || pEdict->IsFree())
			return false;

		if(!strcmp(pEdict->GetClassName(), "player"))
			return false;

		return true;
	}

	TraceType_t GetTraceType() const
	{
		return TRACE_EVERYTHING_FILTER_PROPS;
	}

private:
	const IHandleEntity *m_pPassEnt;
	int m_collisionGroup;
};

class CTraceFilterWalkableEntities : public CTraceFilterNoPlayers
{
public:
	CTraceFilterWalkableEntities(const IHandleEntity *passentity, int collisionGroup, unsigned int flags)
		: CTraceFilterNoPlayers(passentity, collisionGroup), m_flags(flags)
	{
	}

	bool ShouldHitEntity(IHandleEntity *pEntity, int contentsMask)
	{
		if(CTraceFilterNoPlayers::ShouldHitEntity(pEntity, contentsMask))
		{
			edict_t *pEdict = INDEXENT(pEntity->GetRefEHandle().GetEntryIndex());
			return (!IsEntityWalkable(gameents->EdictToBaseEntity(pEdict), m_flags));
		}

		return false;
	}

private:
	unsigned int m_flags;
};

class CTraceFilterCustom : public ITraceFilter
{
public:
	bool ShouldHitEntity(IHandleEntity *entity, int contentmask)
	{
		int index = entity->GetRefEHandle().GetEntryIndex();
		edict_t *pEdict = INDEXENT(index);
		if(pEdict == NULL || pEdict->IsFree())
			return false;

		if(index > 0 && index <= gpGlobals->maxClients)
			return false;

		IServerNetworkable *pNetwork = pEdict->GetNetworkable();
		if(pNetwork == NULL)
			return false;

		ServerClass *pClass = pNetwork->GetServerClass();
		if(pClass == NULL)
			return false;

		const char *name = pClass->GetName();
		if(!strcmp(name, "CBaseDoor") || !strcmp(name, "CTFBaseBoss") || !strcmp(name, "CFuncRespawnRoomVisualizer"))
			return false;

		return true;
	}

	TraceType_t GetTraceType() const
	{
		return TRACE_ENTITIES_ONLY;
	}
};

class PathCost : public IPathCost
{
public:
	PathCost(CBot *pBot, eRouteType route, int iConditions)
		: m_pBot(pBot), m_route(route), m_iConditions(iConditions)
	{
	}

	float operator() (INavMeshArea *toArea, INavMeshArea *fromArea, INavMeshLadder *ladder)
	{
		if(m_pBot == NULL)
		{
			// no bot? that'll put a damper on things
			return 0.0f;
		}
		else if(fromArea == NULL)
		{
			// first area
			return m_pBot->GetNavigator()->GetBelief(toArea->GetID());
		}
		else
		{
			if((fromArea->GetAttributes() & NAV_MESH_JUMP) && (toArea->GetAttributes() & NAV_MESH_JUMP))
			{
				// don't walk between jump points
				return -1.0f;
			}

		#if(SOURCE_ENGINE == SE_TF2)
			if(toArea->GetTFAttribs() & (BLUE_ONE_WAY_DOOR|BLUE_SETUP_GATE))
			{
				// we can't (usually) enter their spawn until the round is over
				if(m_pBot->GetTeam() != TF2_TEAM_BLUE && CGameRulesObject::GetProperty("m_iRoundState") > RoundState_RoundRunning)
					return -1.0f;
			}
			else if(toArea->GetTFAttribs() & (RED_ONE_WAY_DOOR|RED_SETUP_GATE))
			{
				// we can't (usually) enter their spawn until the round is over
				if(m_pBot->GetTeam() != TF2_TEAM_RED && CGameRulesObject::GetProperty("m_iRoundState") > RoundState_RoundRunning)
					return -1.0f;
			}
		#endif

			// how safe do I reckon this area is?
			float fBelief = m_pBot->GetNavigator()->GetBelief(toArea->GetID());
			// how much do I care?
			const float fAggressiveness = 1.0f - (0.8f * m_pBot->GetProfile()->m_fBraveness) * 100.0f;

			// crouching is slow, but sometimes we have to make compromises
			const float fCrouchPenalty = (m_route == FASTEST_ROUTE) ? 20.0f : 5.0f;

			const float fJumpPenalty = 1.2f;

			const float fAvoidPenalty = 20.0f;

			if(m_iConditions & CONDITION_COVERT)
				fBelief *= 2.0f;

			float fDist;
			if(ladder != NULL)
				fDist = ladder->GetLength();
			else
				fDist = (toArea->GetCenter() - fromArea->GetCenter()).Length();

			float fCost = fDist + toArea->GetCostSoFar();

			// add cost of "jump down" pain
			if(!((CNavMeshNavigator *)m_pBot->GetNavigator())->IsConnected(toArea, fromArea))
			{
				const float fFallDistance = -((CNavMeshNavigator *)m_pBot->GetNavigator())->ComputeAdjacentAreaHeightChange(toArea, fromArea);

				const float fFallDamage = min(0.0f, 0.2f * fFallDistance - 20.0f);

				if(fFallDamage > 0.0f)
				{
					// let's not commit suicide
					if(fFallDamage + 5.0f >= m_pBot->GetHealth())
						return -1.0f;

					// am I a maniac? then fall damage is of no concern
					const float painTolerance = 15.0f * m_pBot->GetProfile()->m_fBraveness + 10.0f;
					if(m_route != FASTEST_ROUTE || fFallDamage > painTolerance)
					{
						// 10 points - not a big deal, 50 points - ouch!
						fCost += 100.0f * fFallDamage * fFallDamage;
					}
				}
			}

			if(m_route == SAFEST_ROUTE)
				fCost += fDist * fBelief * fAggressiveness;

			// crouching is so slow
			if(toArea->GetAttributes() & NAV_MESH_CROUCH)
				fCost += fCrouchPenalty * fDist;

			// jumping requires setup
			if(toArea->GetAttributes() & NAV_MESH_JUMP)
				fCost += fJumpPenalty * fDist;

			// this is an area to avoid
			if(toArea->GetAttributes() & NAV_MESH_AVOID)
				fCost += fAvoidPenalty * fDist;

			return fCost;
		}
	}

private:
	CBot *m_pBot;
	eRouteType m_route;
	int m_iConditions;
};

void CNavMeshNavigator::FreeMapMemory()
{
	BeliefSave(true);
	Q_memset(&m_Path, 0, sizeof(m_Path));
}
void CNavMeshNavigator::FreeAllMemory()
{
	Q_memset(&m_Path, 0, sizeof(m_Path));

	delete m_fBelief;
	delete m_fDanger;
}


void CNavMeshNavigator::Init()
{
	if(!g_pNavMesh)
	{
		smutils->LogError(myself, "Error initializing bot navigation: Nav Mesh doesn't exist.");
		return;
	}

	m_CurrWaypoint = NULL;
	m_GoalWaypoint = NULL;
	m_PrevWaypoint = NULL;

	m_bWorkingRoute = false;

	m_iBeliefTeam = -1;

	m_nPathIndex = 0;

	Q_memset(&m_Path, 0, sizeof(m_Path));

	m_fBelief = (float *)malloc(sizeof(float)*RAND_MAX);
	m_fDanger = (float *)malloc(sizeof(float)*RAND_MAX);
}

void CNavMeshNavigator::Clear()
{
	FreeMapMemory();
}

bool CNavMeshNavigator::BeliefLoad()
{
	int iSize;
	int iDesiredSize;
	register unsigned short int i;
	register unsigned short int num;
	unsigned short int *filebelief = NULL;
	char filename[MAX_PATH];

	m_bLoadBelief = false;
	m_iBeliefTeam = m_pBot->GetTeam();

	smutils->BuildPath(Path_SM, filename, sizeof(filename), "data\\afkbot\\nav\\%s%d.rcb", CBotGlobals::GetMapName(), m_iBeliefTeam);

	FILE *bfp = CBotGlobals::OpenFile(filename, "rb");

	if(bfp == NULL)
	{
		Msg(" *** Can't open Waypoint belief array for reading!\n");
		return false;
	}

	fseek(bfp, 0, SEEK_END); // seek at end

	iSize = ftell(bfp); // Get file size
	iDesiredSize = g_pNavMesh->GetAreas()->Count()*sizeof(unsigned short);
	filebelief = new unsigned short[iDesiredSize];

	// size not right, return false to re workout table
	if(iSize != iDesiredSize)
	{
		fclose(bfp);
		return false;
	}

	fseek(bfp, 0, SEEK_SET); // seek at start

	memset(filebelief, 0, sizeof(unsigned short)*iDesiredSize);

	fread(filebelief, sizeof(unsigned short), iDesiredSize, bfp);

	// convert from short int to float

	num = (unsigned short)iDesiredSize;

	// quick loop
	for(i = 0; i < num; i++)
	{
		m_fBelief[i] = (filebelief[i] / 32767) * MAX_BELIEF;
	}

	fclose(bfp);

	return true;
}
// update belief array with averaged belief for this team
bool CNavMeshNavigator::BeliefSave(bool bOverride)
{
	int iSize;
	int iDesiredSize;
	register unsigned short int i;
	register unsigned short int num;
	unsigned short int *filebelief = NULL;
	char filename[1024];

	if((m_pBot->GetTeam() == m_iBeliefTeam) && !bOverride)
		return false;

	// m_iBeliefTeam is the team we've been using -- we might have changed team now
	// so would need to change files if a different team
	// stick to the current team we've been using
	smutils->BuildPath(Path_SM, filename, sizeof(filename), "data\\afkbot\\nav\\%s%d.rcb", CBotGlobals::GetMapName(), m_iBeliefTeam);

	FILE *bfp = CBotGlobals::OpenFile(filename, "rb");

	if(bfp != NULL)
	{
		fseek(bfp, 0, SEEK_END); // seek at end

		iSize = ftell(bfp); // Get file size
		iDesiredSize = g_pNavMesh->GetAreas()->Count()*sizeof(unsigned short);
		filebelief = new unsigned short[iDesiredSize];
		memset(filebelief, 0, sizeof(unsigned short)*iDesiredSize);

		// size not right, return false to re workout table
		if(iSize != iDesiredSize)
		{
			fclose(bfp);
		}
		else
		{
			fseek(bfp, 0, SEEK_SET); // seek at start

			if(bfp)
				fread(filebelief, sizeof(unsigned short), iDesiredSize, bfp);

			fclose(bfp);
		}
	}

	bfp = CBotGlobals::OpenFile(filename, "wb");

	if(bfp == NULL)
	{
		m_bLoadBelief = true;
		m_iBeliefTeam = m_pBot->GetTeam();
		Msg(" *** Can't open Waypoint Belief array for writing!\n");
		return false;
	}

	// convert from short int to float

	num = (unsigned short)iDesiredSize;

	// quick loop
	for(i = 0; i < num; i++)
	{
		filebelief[i] = (filebelief[i] / 2) + (unsigned short)((m_fBelief[i] / MAX_BELIEF) * 16383);
	}

	fseek(bfp, 0, SEEK_SET); // seek at start

	fwrite(filebelief, sizeof(unsigned short), num, bfp);

	fclose(bfp);

	// new team -- load belief 
	m_iBeliefTeam = m_pBot->GetTeam();
	m_bLoadBelief = true;
	m_bBeliefChanged = false; // saved

	return true;
}

bool CNavMeshNavigator::WantToSaveBelief()
{
	// playing on this map for more than a normal load time
	return (m_bBeliefChanged && (m_iBeliefTeam != m_pBot->GetTeam()));
}

void CNavMeshNavigator::BeliefOne(int iAreaID, BotBelief iBeliefType, float fDist)
{
	if(iBeliefType == BELIEF_SAFETY)
	{
		if(m_fBelief[iAreaID] > 0)
			m_fBelief[iAreaID] *= bot_belief_fade.GetFloat();
		if(m_fBelief[iAreaID] < 0)
			m_fBelief[iAreaID] = 0;
	}
	else // danger	
	{
		if(m_fBelief[iAreaID] < MAX_BELIEF)
			m_fBelief[iAreaID] += (2048.0f / fDist);
		if(m_fBelief[iAreaID] > MAX_BELIEF)
			m_fBelief[iAreaID] = MAX_BELIEF;
	}

	m_bBeliefChanged = true;
}

void CNavMeshNavigator::Belief(Vector vOrigin, Vector vOther, float fBelief, float fStrength, BotBelief iType)
{
	static int iAreaID;

	CList<INavMeshArea*> list = CollectSurroundingAreas(GetNearestArea(vOrigin), (vOrigin - vOther).Length());

	for(INavMeshArea *pArea : list)
	{
		iAreaID = pArea->GetID();
		CFmtStrN<32> fmt;
		if(IsAreaVisibleToEnemy(pArea, m_pBot->GetTeam()))
		{
			if(iType == BELIEF_SAFETY)
			{
				if(m_fBelief[iAreaID] > 0)
					m_fBelief[iAreaID] *= bot_belief_fade.GetFloat();
				if(m_fBelief[iAreaID] < 0)
					m_fBelief[iAreaID] = 0;

				NDebugOverlay::EntityTextAtPosition(pArea->GetCenter(), 0, "Safety", 5.0f, 0, 150, 0, 200);
			}
			else if(iType == BELIEF_DANGER)
			{
				if(m_fBelief[iAreaID] < MAX_BELIEF)
					m_fBelief[iAreaID] += (fStrength / (vOrigin - pArea->GetCenter()).Length())*fBelief;
				if(m_fBelief[iAreaID] > MAX_BELIEF)
					m_fBelief[iAreaID] = MAX_BELIEF;

				NDebugOverlay::EntityTextAtPosition(pArea->GetCenter(), 0, fmt.sprintf("Danger %0.2f", m_fBelief[iAreaID]), 5.0f, 255, 0, 0, 200);
			}
		}
		else
		{
			// this waypoint is safer from this danger
			if(iType == BELIEF_DANGER)
			{
				if(m_fBelief[iAreaID] > 0)
					m_fBelief[iAreaID] *= 0.9;

				NDebugOverlay::EntityTextAtPosition(pArea->GetCenter(), 1, "Safety INV", 5.0f, 0, 150, 0, 200);
			}
			else if(iType == BELIEF_SAFETY)
			{
				if(m_fBelief[iAreaID] < MAX_BELIEF)
					m_fBelief[iAreaID] += (fStrength / (vOrigin - pArea->GetCenter()).Length())*fBelief*0.5f;
				if(m_fBelief[iAreaID] > MAX_BELIEF)
					m_fBelief[iAreaID] = MAX_BELIEF;

				NDebugOverlay::EntityTextAtPosition(pArea->GetCenter(), 1, fmt.sprintf("Danger INV %0.2f", m_fBelief[iAreaID]), 5.0f, 255, 0, 0, 200);
			}
		}
	}

	m_bBeliefChanged = true;
}

float CNavMeshNavigator::GetBelief(int iAreaID)
{
	return m_fBelief[iAreaID];
}

float CNavMeshNavigator::GetCurrentBelief()
{
	if(m_CurrWaypoint != NULL)
	{
		return m_fBelief[m_CurrWaypoint->GetID()];
	}

	return 0;
}

bool CNavMeshNavigator::IsAreaVisibleToEnemy(INavMeshArea *area, int iTeam)
{
	Vector vCenter = area->GetCenter();
	vCenter.z += HalfHumanHeight;

	IGamePlayer *pPlayer = NULL;
	edict_t *pEdict = NULL;
	for(short int i = 0; i < MAX_PLAYERS; i++)
	{
		pPlayer = playerhelpers->GetGamePlayer(i);
		if(pPlayer && pPlayer->IsInGame() && !pPlayer->IsSourceTV() && !pPlayer->IsReplay())
		{
			pEdict = pPlayer->GetEdict();
			if(!pEdict || pEdict->IsFree())
				continue;

			if(CBotGlobals::GetTeam(pEdict) == iTeam)
				continue;

			Vector vOther = CBotGlobals::EntityOrigin(pEdict);
			vOther.z += HalfHumanHeight;

			if(CBotGlobals::QuickTraceline(pEdict, vOther, vCenter) >= 0.98f)
				return true;
		}
	}

	return false;
}

void CNavMeshNavigator::DrawPath(void)
{
	if(!RouteFound())
		return;

	for(int i=1; i<m_iSegments; ++i)
	{
		const Vector vFromPos = m_Path[i-i].pos;
		const Vector vToPos = m_Path[i].pos;
		NDebugOverlay::Line(vFromPos, vToPos, 255, 75, 0, false, 0.4);
	}

	Vector vClose;
	if(FindOurPositionOnPath(&vClose, true) >= 0)
	{
		const Vector vOrigin = m_pBot->GetOrigin();
		NDebugOverlay::HorzArrow(vOrigin, vClose, 0.6f, 20, 240, 0, 192, false, 0.4);
	}
}

bool CNavMeshNavigator::WorkRoute(Vector vFrom, Vector vTo, bool *bFail, bool bRestart, bool bNoInterruptions, int iGoalID, int iConditions, int iDangerId)
{
	return WorkRoute(vFrom, vTo, bFail, bRestart, bNoInterruptions, GetAreaByID(iGoalID), iConditions, iDangerId);
}

bool CNavMeshNavigator::WorkRoute(Vector vFrom, Vector vTo, bool *bFail, bool bRestart, bool bNoInterruptions, INavMeshArea *goal, int iConditions, int iDangerId)
{ // I'm lazy, got this and others below from https://github.com/VSES/SourceEngine2007/blob/master/src_main/game/server/cstrike/bot/cs_bot_pathfind.cpp
	extern ConVar bot_debug_show_route;

	if(bRestart)
	{
		if(WantToSaveBelief())
			BeliefSave();
		if(WantToLoadBelief())
			BeliefLoad();

		*bFail = false;
		m_bWorkingRoute = true;

		if(goal)
			m_GoalWaypoint = goal;
		else
			m_GoalWaypoint = GetNearestArea(vTo);

		if(m_GoalWaypoint == NULL)
		{
			*bFail = true;
			m_bWorkingRoute = false;
			return true;
		}

		m_vPreviousPoint = vFrom;

		m_CurrWaypoint = GetNearestArea(vFrom);

		if(m_CurrWaypoint == NULL)
		{
			*bFail = true;
			m_bWorkingRoute = false;
			return true;
		}

		CNavMeshArea::ClearSearchList();

		m_CurrWaypoint->AddToOpenList();
		m_CurrWaypoint->SetTotalCost(m_pBot->DistanceFrom(vTo));

		Q_memset(&m_Path, 0, sizeof(Segment));
		m_iSegments = 0;
	}

	if(!m_GoalWaypoint || !m_CurrWaypoint)
	{
		*bFail = true;
		m_bWorkingRoute = false;
		return true;
	}

	eRouteType route;
	if((iConditions & CONDITION_COVERT) || (iConditions & CONDITION_PARANOID))
		route = SAFEST_ROUTE;
	if((iConditions & CONDITION_PUSH) || (iConditions & CONDITION_RUN))
		route = FASTEST_ROUTE;

	PathCost cost(m_pBot, route, iConditions);
	INavMeshArea *pClosest = nullptr;
	bool bPathExist = BuildPath(m_CurrWaypoint, m_GoalWaypoint, &vTo, cost, &pClosest);

	int iCount;
	INavMeshArea *pGoal = bPathExist ? m_GoalWaypoint : pClosest;
	for(INavMeshArea *pTempArea = pGoal; pTempArea; pTempArea = pTempArea->GetParent())
		iCount++;

	if(iCount == 0)
	{
		*bFail = true;
		m_bWorkingRoute = false;
		return true;
	}

	if(iCount == 1)
	{
		m_Path[0].area = m_CurrWaypoint;
		m_Path[0].pos = vFrom;
		m_Path[0].pos.z = GetZ(m_CurrWaypoint, vFrom);

		m_Path[1].area = m_CurrWaypoint;
		m_Path[1].pos = vTo;
		m_Path[1].pos.z = GetZ(m_CurrWaypoint, vTo);

		m_iSegments = 2;
	}

	if(iCount >= MAX_SEGMENT_COUNT)
		iCount = MAX_SEGMENT_COUNT-1;

	m_iSegments = iCount;
	for(INavMeshArea *pTempArea = pGoal; iCount && pTempArea; pTempArea = pTempArea->GetParent())
	{
		iCount--;
		m_Path[iCount].area = pTempArea;
	}

	if(!ComputePathPositions())
	{
		*bFail = true;
		m_bWorkingRoute = false;
		return true;
	}

	m_Path[m_iSegments].area = pGoal;
	m_Path[m_iSegments].pos = vTo;
	m_Path[m_iSegments].pos.z = GetZ(pGoal, vTo);
	m_iSegments++;

	if(m_iSegments > 0)
	{
		m_vGoal = m_Path[m_iSegments - 1].pos;

		*bFail = false;
		m_bWorkingRoute = false;
		return true;
	}

	return false;
}

bool CNavMeshNavigator::ComputePathPositions()
{
	if(m_iSegments == 0)
		return false;

	// start in first area's center
	m_Path[0].pos = m_Path[0].area->GetCenter();
	m_Path[0].ladder = NULL;

	const float stepInDist = 5.0f; // how far to "step into" an area - must be less than min area size

	const float pushDist = 75.0f;

	for(int i = 1; i < m_iSegments; ++i)
	{
		const Segment *from = &m_Path[i-1];
		Segment *to = &m_Path[i];

		eNavTraverse howTo = to->area->GetParentHow();

		if(howTo <= GO_WEST)		// walk along the floor to the next area
		{
			to->ladder = NULL;

			// compute next point, keeping path as straight as possible
			ComputeClosestPointInPortal(from->area, to->area, (eNavDir)howTo, from->pos, &to->pos);

			// move goal position into the goal area a bit
			AddDirectionVector(&to->pos, (eNavDir)howTo, stepInDist);

			// we need to walk out of "from" area, so keep Z where we can reach it
			to->pos.z = GetZ(from->area, to->pos);

			// if this is a "jump down" connection, we must insert an additional point on the path
			if(!IsConnected(to->area, from->area))
			{
				// this is a "jump down" link

				// compute direction of path just prior to "jump down"
				Vector dir;
				DirectionToVector((eNavDir)howTo, &dir);

				// shift top of "jump down" out a bit to "get over the ledge"
				to->pos.x += pushDist * dir.x;
				to->pos.y += pushDist * dir.y;

				// insert a duplicate node to represent the bottom of the fall
				if(m_iSegments < MAX_SEGMENT_COUNT-1)
				{
					// copy nodes down
					for(int j = m_iSegments; j > i; --j)
						m_Path[j] = m_Path[j-1];

					// path is one node longer
					++m_iSegments;

					// move index ahead into the new node we just duplicated
					++i;

					m_Path[i].pos.x = to->pos.x;
					m_Path[i].pos.y = to->pos.y;

					// put this one at the bottom of the fall
					m_Path[i].pos.z = GetZ(to->area, m_Path[i].pos);
				}
			}
		}
		else if(howTo == GO_LADDER_UP)		// to get to next area, must go up a ladder
		{
			// find our ladder
			const CList<INavMeshLadder*> list = GetLadderList(from->area, NAV_LADDER_DIR_UP);
			if(!list || list.Empty())
				return false;

			for(INavMeshLadder *ladder : list)
			{
				// can't use "behind" area when ascending...
				if(ladder->GetTopForwardAreaID() == to->area->GetID()
				|| ladder->GetTopLeftAreaID() == to->area->GetID()
				|| ladder->GetTopRightAreaID() == to->area->GetID())
				{
					to->ladder = ladder;
					to->pos = ladder->GetBottom() + GetLadderNormal(ladder) * 2.0f * HalfHumanWidth;
					break;
				}
			}
		}
		else if(howTo == GO_LADDER_DOWN)		// to get to next area, must go down a ladder
		{
			// find our ladder
			const CList<INavMeshLadder*> list = GetLadderList(from->area, NAV_LADDER_DIR_DOWN);
			if(!list || list.Empty())
				return false;

			for(INavMeshLadder *ladder : list)
			{
				if(ladder->GetBottomAreaID() == to->area->GetID())
				{
					to->ladder = ladder;
					to->pos = ladder->GetTop() - GetLadderNormal(ladder) * 2.0f * HalfHumanWidth;

					CTraceFilterWalkableEntities filter(NULL, COLLISION_GROUP_NONE, WALK_THRU_EVERYTHING);
					CBotGlobals::TraceLine(ladder->GetTop(), to->pos, MASK_PLAYERSOLID_BRUSHONLY, &filter);
					const trace_t *result = CBotGlobals::GetTraceResult();

					if(result->fraction < 1.0f)
					{
						to->pos = ladder->GetTop() + GetLadderNormal(ladder) * 2.0f * HalfHumanWidth;
						to->pos = GetClosestPointOnArea(from->area, to->pos);
					}
					break;
				}
			}
		}
	}

	return true;
}

void CNavMeshNavigator::SetPathIndex(const int iIndex)
{
	m_nPathIndex = min(iIndex, m_iSegments-1);
	if(m_Path[m_nPathIndex].ladder)
	{
		//SetupLadderMovement();
	}
}

bool CNavMeshNavigator::FindClosestPointOnPath(const Vector &vPos, int startIndex, int endIndex, Vector *vClose)
{
	if(!RouteFound() || vClose == NULL)
		return false;

	Vector along, toWorldPos;
	Vector pos;
	const Vector *from, *to;
	float length;
	float closeLength;
	float closeDistSq = 9999999999.9;
	float distSq;

	for(int i=startIndex; i<=endIndex; ++i)
	{
		from = &m_Path[i-1].pos;
		to = &m_Path[i].pos;

		// compute ray along this path segment
		along = *to - *from;

		// make it a unit vector along the path
		length = along.NormalizeInPlace();

		// compute vector from start of segment to our point
		toWorldPos = vPos - *from;

		// find distance of closest point on ray
		closeLength = DotProduct(toWorldPos, along);

		// constrain point to be on path segment
		if(closeLength <= 0.0f)
			pos = *from;
		else if(closeLength >= length)
			pos = *to;
		else
			pos = *from + closeLength * along;

		distSq = (pos - vPos).LengthSqr();

		// keep the closest point so far
		if(distSq < closeDistSq)
		{
			closeDistSq = distSq;
			*vClose = pos;
		}
	}

	return true;
}

int CNavMeshNavigator::FindOurPositionOnPath(Vector *vClose, bool bLocal)
{
	if(!RouteFound())
		return -1;

	Vector along, toFeet;
	Vector feet = m_pBot->GetOrigin();
	Vector eyes = feet + Vector(0, 0, HalfHumanHeight);	// in case we're crouching
	Vector pos;
	const Vector *from, *to;
	float length;
	float closeLength;
	float closeDistSq = 9999999999.9;
	int closeIndex = -1;
	float distSq;

	int start, end;

	if(bLocal)
	{
		start = m_iSegments - 3;
		if(start < 1)
			start = 1;

		end = m_iSegments + 3;
		if(end > m_iSegments)
			end = m_iSegments;
	}
	else
	{
		start = 1;
		end = m_iSegments;
	}

	for(int i=start; i<end; ++i)
	{
		from = &m_Path[i-1].pos;
		to = &m_Path[i].pos;

		// compute ray along this path segment
		along = *to - *from;

		// make it a unit vector along the path
		length = along.NormalizeInPlace();

		// compute vector from start of segment to our point
		toFeet = feet - *from;

		// find distance of closest point on ray
		closeLength = DotProduct(toFeet, along);

		// constrain point to be on path segment
		if(closeLength <= 0.0f)
			pos = *from;
		else if(closeLength >= length)
			pos = *to;
		else
			pos = *from + closeLength * along;

		distSq = (pos - feet).LengthSqr();

		// keep the closest point so far
		if(distSq < closeDistSq)
		{
			// don't use points we cant see
			Vector probe = pos + Vector(0, 0, HalfHumanHeight);
			if(!IsWalkableTraceLineClear(eyes, probe, WALK_THRU_DOORS | WALK_THRU_BREAKABLES))
				continue;

			closeDistSq = distSq;
			if(vClose)
				*vClose = pos;
			closeIndex = i-1;
		}
	}

	return closeIndex;
}

int CNavMeshNavigator::FindPathPoint(float fAheadRange, Vector *vPoint, int *prevIndex)
{
	Vector myOrigin = m_pBot->GetOrigin() + Vector(0, 0, HalfHumanHeight);

	// find path index just past aheadRange
	int afterIndex;

	// finds the closest point on local area of path, and returns the path index just prior to it
	Vector close;
	int startIndex = FindOurPositionOnPath(&close, true);

	if(prevIndex)
		*prevIndex = startIndex;

	if(startIndex <= 0)
	{
		// went off the end of the path
		// or next point in path is unwalkable (ie: jump-down)
		// keep same point
		return m_nPathIndex;
	}

	// if we are crouching, just follow the path exactly
	if(m_pBot->GetButtons() & IN_DUCK)
	{
		// we want to move to the immediately next point along the path from where we are now
		int index = startIndex+1;
		if(index >= m_iSegments)
			index = m_iSegments-1;

		*vPoint = m_Path[index].pos;

		// if we are very close to the next point in the path, skip ahead to the next one to avoid wiggling
		// we must do a 2D check here, in case the goal point is floating in space due to jump down, etc
		const float closeEpsilon = 20.0f;	// 10
		while((*vPoint - close).AsVector2D().IsLengthLessThan(closeEpsilon))
		{
			++index;

			if(index >= m_iSegments)
			{
				index = m_iSegments-1;
				break;
			}

			*vPoint = m_Path[index].pos;
		}

		return index;
	}

	// make sure we use a node a minimum distance ahead of us, to avoid wiggling 
	while(startIndex < m_iSegments-1)
	{
		const Vector pos = m_Path[startIndex+1].pos;

		// we must do a 2D check here, in case the goal point is floating in space due to jump down, etc
		const float closeEpsilon = 20.0f;
		if((pos - close).AsVector2D().IsLengthLessThan(closeEpsilon))
		{
			++startIndex;
		}
		else
		{
			break;
		}
	}

	// if we hit a ladder, stop, or jump area, must stop (dont use ladder behind us)
	if(startIndex > m_nPathIndex && startIndex < m_iSegments 
	&& (m_Path[startIndex].ladder || (m_Path[startIndex].area->GetAttributes() & (NAV_MESH_JUMP|NAV_MESH_STOP))))
	{
		*vPoint = m_Path[startIndex].pos;
		return startIndex;
	}

	// we need the point just *ahead* of us
	++startIndex;
	if(startIndex >= m_iSegments)
		startIndex = m_iSegments-1;

	// if we hit a ladder, stop, or jump area, must stop
	if(startIndex < m_iSegments 
	&& (m_Path[startIndex].ladder || (m_Path[startIndex].area->GetAttributes() & (NAV_MESH_JUMP|NAV_MESH_STOP))))
	{
		*vPoint = m_Path[startIndex].pos;
		return startIndex;
	}

	// note direction of path segment we are standing on
	Vector initDir = m_Path[startIndex].pos - m_Path[startIndex-1].pos;
	initDir.NormalizeInPlace();

	const Vector feet = m_pBot->GetOrigin();
	const Vector eyes = feet + Vector(0, 0, HalfHumanHeight);
	float rangeSoFar = 0;

	// this flag is true if our ahead point is visible
	bool visible = true;

	Vector prevDir = initDir;

	// step along the path until we pass aheadRange
	bool isCorner = false;
	int i;
	for(i=startIndex; i<m_iSegments; ++i)
	{
		const Vector pos = m_Path[i].pos;
		const Vector to = pos - m_Path[i-1].pos;
		Vector dir = to;
		dir.NormalizeInPlace();

		// don't allow path to double-back from our starting direction (going upstairs, down curved passages, etc)
		if(DotProduct(dir, initDir) < 0.0f) // -0.25f
		{
			--i;
			break;
		}

		// if the path turns a corner, we want to move towards the corner, not into the wall/stairs/etc
		if(DotProduct(dir, prevDir) < 0.5f)
		{
			isCorner = true;
			--i;
			break;
		}
		prevDir = dir;

		// don't use points we cant see
		const Vector probe = pos + Vector(0, 0, HalfHumanHeight);
		if(!IsWalkableTraceLineClear(eyes, probe, WALK_THRU_BREAKABLES))
		{
			// presumably, the previous point is visible, so we will interpolate
			visible = false;
			break;
		}

		// if we encounter a ladder or jump area, we must stop
		if(i < m_iSegments && (m_Path[i].ladder || (m_Path[i].area->GetAttributes() & NAV_MESH_JUMP)))
			break;

		Vector along = (i == startIndex) ? (pos - feet) : (pos - m_Path[i-1].pos);
		rangeSoFar += along.Length2D();

		// stop if we have gone farther than aheadRange
		if(rangeSoFar >= fAheadRange)
			break;
	}

	if(i < startIndex)
		afterIndex = startIndex;
	else if(i < m_iSegments)
		afterIndex = i;
	else
		afterIndex = m_iSegments-1;


	// compute point on the path at aheadRange
	if(afterIndex == 0)
	{
		*vPoint = m_Path[0].pos;
	}
	else
	{
		// interpolate point along path segment
		const Vector *afterPoint = &m_Path[afterIndex].pos;
		const Vector *beforePoint = &m_Path[afterIndex-1].pos;

		Vector to = *afterPoint - *beforePoint;
		float length = to.Length2D();

		float t = 1.0f - ((rangeSoFar - fAheadRange) / length);

		if(t < 0.0f)
			t = 0.0f;
		else if(t > 1.0f)
			t = 1.0f;

		*vPoint = *beforePoint + t * to;

		// if afterPoint wasn't visible, slide point backwards towards beforePoint until it is
		if(!visible)
		{
			const float sightStepSize = 25.0f;
			const float dt = sightStepSize / length;

			const Vector probe = *vPoint + Vector(0, 0, HalfHumanHeight);
			while(t > 0.0f && !IsWalkableTraceLineClear(eyes, probe, WALK_THRU_BREAKABLES))
			{
				t -= dt;
				*vPoint = *beforePoint + t * to;
			}

			if(t <= 0.0f)
				*vPoint = *beforePoint;
		}
	}

	// if position found is too close to us, or behind us, force it farther down the path so we don't stop and wiggle
	if(!isCorner)
	{
		const float epsilon = 50.0f;
		Vector2D toPoint;
		toPoint.x = vPoint->x - myOrigin.x;
		toPoint.y = vPoint->y - myOrigin.y;
		if(DotProduct2D(toPoint, initDir.AsVector2D()) < 0.0f || toPoint.IsLengthLessThan(epsilon))
		{
			int i;
			for(i=startIndex; i<m_iSegments; ++i)
			{
				toPoint.x = m_Path[i].pos.x - myOrigin.x;
				toPoint.y = m_Path[i].pos.y - myOrigin.y;
				if(m_Path[i].ladder || (m_Path[i].area->GetAttributes() & NAV_MESH_JUMP) || toPoint.IsLengthGreaterThan(epsilon))
				{
					*vPoint = m_Path[i].pos;
					startIndex = i;
					break;
				}
			}

			if(i == m_iSegments)
			{
				*vPoint = m_vGoal;
				startIndex = m_iSegments-1;
			}
		}
	}

	// m_pathIndex should always be the next point on the path, even if we're not moving directly towards it
	return startIndex;
}

const bool CNavMeshNavigator::IsNearJump() const
{
	if(m_nPathIndex == 0 || m_nPathIndex >= m_iSegments)
		return false;

	for(int i=m_nPathIndex-1; i<m_nPathIndex; ++i)
	{
		if(m_Path[i].area->GetAttributes() & NAV_MESH_JUMP)
		{
			float dz = m_Path[i+1].pos.z - m_Path[i].pos.z;

			if(dz > 0.0f)
				return true;
		}
	}

	return false;
}

INavMeshArea *CNavMeshNavigator::ChooseBestFromBelief(CList<INavMeshArea*> *goals, bool bHighDanger, int iSearchFlags, int iTeam)
{
	INavMeshArea *ret = NULL;
	INavMeshArea *checkarea;
	ShortestPathCost cost;
	float fBelief = 0;
	float fSelect;
	float bBeliefFactor = 1.0f;

	if(goals->Count() <= 1)
		return goals->Element(0);

	for(int i = 0, iSize = goals->Count(); i < iSize; i++)
	{
		checkarea = goals->Element(i);
		bBeliefFactor = 1.0f;

		if(iSearchFlags & WPT_SEARCH_AVOID_SENTRIES)
		{
			for(int j = gpGlobals->maxClients; j > 0; j--)
			{
				edict_t *pSentry = CTeamFortress2Mod::GetSentryGun(j - 1);
				Vector vOrigin = CBotGlobals::EntityOrigin(pSentry);

				if(pSentry != NULL)
				{
					if(TravelDistance(checkarea->GetCenter(), vOrigin, cost) < 200.0f)
					{
						bBeliefFactor *= 0.1f;
					}
				}
			}
		}

		if(iSearchFlags & WPT_SEARCH_AVOID_SNIPERS)
		{
			for(int j = gpGlobals->maxClients; j > 0; j--)
			{
				edict_t *pPlayer = INDEXENT(j);
				Vector vOrigin = CBotGlobals::EntityOrigin(pPlayer);

				if((pPlayer != NULL) && !pPlayer->IsFree() && (CClassInterface::GetTF2Class(pPlayer) == TF_CLASS_SNIPER))
				{
					if((iTeam == 0) || (iTeam == CClassInterface::GetTeam(pPlayer)))
					{
						if(TravelDistance(checkarea->GetCenter(), vOrigin, cost) < 200.0f)
						{
							bBeliefFactor *= 0.1f;
						}
					}
				}
			}
		}

		if(iSearchFlags & WPT_SEARCH_AVOID_TEAMMATE)
		{
			for(int j = gpGlobals->maxClients; j > 0; j--)
			{
				edict_t *pPlayer = INDEXENT(j);
				Vector vOrigin = CBotGlobals::EntityOrigin(pPlayer);

				if((pPlayer != NULL) && !pPlayer->IsFree())
				{
					if((iTeam == 0) || (iTeam == CClassInterface::GetTeam(pPlayer)))
					{
						if(TravelDistance(checkarea->GetCenter(), vOrigin, cost) < 200.0f)
						{
							bBeliefFactor *= 0.1f;
						}
					}
				}
			}
		}

		if(bHighDanger)
		{
			fBelief += bBeliefFactor * (1.0f + (m_fBelief[checkarea->GetID()]));
		}
		else
		{
			fBelief += bBeliefFactor * (1.0f + (MAX_BELIEF - (m_fBelief[checkarea->GetID()])));
		}
	}

	fSelect = RandomFloat(0, fBelief);

	fBelief = 0;

	for(int i = 0, iSize = goals->Count(); i < iSize; i++)
	{
		checkarea = goals->Element(i);

		bBeliefFactor = 1.0f;

		if(iSearchFlags & WPT_SEARCH_AVOID_SENTRIES)
		{
			for(int j = gpGlobals->maxClients; j > 0; j--)
			{
				edict_t *pSentry = CTeamFortress2Mod::GetSentryGun(j);
				Vector vOrigin = CBotGlobals::EntityOrigin(pSentry);

				if(pSentry != NULL)
				{
					if(TravelDistance(checkarea->GetCenter(), vOrigin, cost) < 200.0f)
					{
						bBeliefFactor *= 0.1f;
					}
				}
			}
		}

		if(iSearchFlags & WPT_SEARCH_AVOID_SNIPERS)
		{
			for(int j = gpGlobals->maxClients; j > 0; j--)
			{
				edict_t *pPlayer = INDEXENT(j);
				Vector vOrigin = CBotGlobals::EntityOrigin(pPlayer);

				if((pPlayer != NULL) && !pPlayer->IsFree() && (CClassInterface::GetTF2Class(pPlayer) == TF_CLASS_SNIPER))
				{
					if(TravelDistance(checkarea->GetCenter(), vOrigin, cost) < 200.0f)
					{
						bBeliefFactor *= 0.1f;
					}
				}
			}
		}

		if(iSearchFlags & WPT_SEARCH_AVOID_TEAMMATE)
		{
			for(int j = gpGlobals->maxClients; j > 0; j--)
			{
				edict_t *pPlayer = INDEXENT(j);
				Vector vOrigin = CBotGlobals::EntityOrigin(pPlayer);

				if((pPlayer != NULL) && !pPlayer->IsFree())
				{
					if(TravelDistance(checkarea->GetCenter(), vOrigin, cost) < 200.0f)
					{
						bBeliefFactor *= 0.1f;
					}
				}
			}
		}

		if(bHighDanger)
		{
			fBelief += bBeliefFactor * (1.0f + (m_fBelief[checkarea->GetID()]));
		}
		else
		{
			fBelief += bBeliefFactor * (1.0f + (MAX_BELIEF - (m_fBelief[checkarea->GetID()])));
		}

		if(fSelect <= fBelief)
		{
			ret = checkarea;
			break;
		}
	}

	if(ret == NULL)
		ret = goals->Element(RandomInt(0, goals->Count()-1));

	return ret;
}

INavMeshArea *CNavMeshNavigator::ChooseBestFromBeliefBetweenAreas(CList<INavMeshArea*> *goals, bool bHighDanger, bool bIgnoreBelief)
{
	INavMeshArea *ret = NULL;
	INavMeshArea *checkarea;
	float fBelief = 0;
	float fSelect;

	if(goals->Count() <= 1)
		return goals->Element(0);

	for(int i = 0, iSize = goals->Count(); i < iSize; i++)
	{
		checkarea = goals->Element(i);
		if(bIgnoreBelief)
		{
			if(bHighDanger)
				fBelief += checkarea->GetTotalCost();
			else
				fBelief += (131072.0f - checkarea->GetTotalCost());
		}
		else if(bHighDanger)
			fBelief += m_fBelief[checkarea->GetID()] + checkarea->GetTotalCost();
		else
			fBelief += MAX_BELIEF - m_fBelief[checkarea->GetID()] + (131072.0f - checkarea->GetTotalCost());
	}

	fSelect = RandomFloat(0, fBelief);

	fBelief = 0;

	for(int i = 0, iSize = goals->Count(); i < iSize; i++)
	{
		checkarea = goals->Element(i);
		if(bIgnoreBelief)
		{
			if(bHighDanger)
				fBelief += checkarea->GetTotalCost();
			else
				fBelief += (131072.0f - checkarea->GetTotalCost());
		}
		else if(bHighDanger)
			fBelief += m_fBelief[checkarea->GetID()] + checkarea->GetTotalCost();
		else
			fBelief += MAX_BELIEF - m_fBelief[checkarea->GetID()] + (131072.0f - checkarea->GetTotalCost());

		if(fSelect <= fBelief)
		{
			ret = checkarea;
			break;
		}
	}

	if(ret == NULL)
		ret = goals->Element(RandomInt(0, goals->Count()-1));

	return ret;
}

// if bot has a current position to walk to return the boolean
bool CNavMeshNavigator::HasNextPoint()
{
	return m_CurrWaypoint != NULL;
}

bool CNavMeshNavigator::NextPointIsOnLadder()
{
	return m_Path[m_nPathIndex].ladder != NULL;
}

bool CNavMeshNavigator::GetNextRoutePoint(Vector *point)
{
	if(m_iSegments)
	{
		Vector vGoal = m_Path[m_nPathIndex].pos;

		if(vGoal.IsValid())
		{
			*point = vGoal;
			return true;
		}
	}

	return false;
}

// return the vector of the next point
Vector CNavMeshNavigator::GetNextPoint()
{
	return GetClosestPointOnArea(m_CurrWaypoint, m_pBot->GetOrigin());
}

float CNavMeshNavigator::GetNextYaw()
{
	return 0.0f;
}

bool CNavMeshNavigator::CanGetTo(Vector vOrigin)
{
	static ShortestPathCost cost;

	INavMeshArea *fromArea = GetNearestArea(m_pBot->GetOrigin());
	INavMeshArea *toArea = GetNearestArea(vOrigin);

	return BuildPath(fromArea, toArea, NULL, cost);
}

void CNavMeshNavigator::UpdatePosition()
{
	VPROF_BUDGET(__FUNCTION__, VPROF_BUDGETGROUP_NPCS);

	static Vector vWptOrigin;
	static float fPrevBelief, fBelief;

	static Vector v_dynamic;

	static QAngle qAim;
	static Vector vAim;

	static bool bTouched;

	fPrevBelief = 0;
	fBelief = 0;

	if(m_CurrWaypoint == NULL || !RouteFound()) // invalid
	{
		m_pBot->StopMoving();
		m_bOffsetApplied = false;
		return;
	}

	const bool movetype_ok = CClassInterface::IsMoveType(m_pBot->GetEdict(), MOVETYPE_LADDER) || CClassInterface::IsMoveType(m_pBot->GetEdict(), MOVETYPE_FLYGRAVITY);

	if(!m_bWorkingRoute)
	{
		int prevIndex = 0;	// closest index on path just prior to where we are now
		const float fAheadRange = 300.0f;
		int newIndex = FindPathPoint(fAheadRange, &vWptOrigin, &prevIndex);

		v_dynamic = (vWptOrigin + m_vOffset);

		bTouched = (m_pBot->GetOrigin() - v_dynamic).Length() < m_pBot->GetTouchDistance();

		// if we moved to a new node on the path, setup movement
		if(newIndex > m_nPathIndex)
		{
			SetPathIndex(newIndex);
		}

		Segment *currSegment = &m_Path[m_nPathIndex];

		if(bTouched)
		{
			int iWaypointID = m_CurrWaypoint->GetID();
			int iWaypointFlagsPrev = 0;

			fPrevBelief = GetBelief(iWaypointID);

			// Bot passed into this waypoint safely, update belief

			bot_statistics_t *stats = m_pBot->GetStats();

			if(stats)
			{
				if((stats->m_iEnemiesVisible > stats->m_iTeamMatesVisible) && (stats->m_iEnemiesInRange > 0))
					BeliefOne(iWaypointID, BELIEF_DANGER, 100.0f);
				else if((stats->m_iTeamMatesVisible > 0) && (stats->m_iTeamMatesInRange > 0))
					BeliefOne(iWaypointID, BELIEF_SAFETY, 100.0f);
			}

			m_bOffsetApplied = false;

			m_bDangerPoint = false;

			if(m_nPathIndex >= m_iSegments-1) // reached goal!!
			{
				m_pBot->TouchedWpt(m_CurrWaypoint);

				m_vPreviousPoint = m_pBot->GetOrigin();
				m_PrevWaypoint = m_CurrWaypoint;
				m_CurrWaypoint = NULL;

				m_nPathIndex = 0;

				if(m_pBot->GetSchedules()->IsCurrentSchedule(SCHED_RUN_FOR_COVER)
				|| m_pBot->GetSchedules()->IsCurrentSchedule(SCHED_GOOD_HIDE_SPOT))
				{
					m_pBot->ReachedCoverSpot(m_CurrWaypoint->GetAttributes());
				}
			}
			else
			{
				iWaypointFlagsPrev = GetCurrentFlags();
				m_vPreviousPoint = m_pBot->GetOrigin();
				m_PrevWaypoint = m_CurrWaypoint;
				m_CurrWaypoint = currSegment->area;

				m_pBot->TouchedWpt(m_CurrWaypoint);

				fBelief = GetBelief(m_CurrWaypoint->GetID());

				m_nPathIndex++;
			}
		}
	}
	else
		m_bOffsetApplied = false;

	m_pBot->WalkingTowardsWaypoint(m_CurrWaypoint, &m_bOffsetApplied, m_vOffset);

	if(!(m_CurrWaypoint->GetAttributes() & NAV_MESH_PRECISE) && !m_bOffsetApplied && !IsNearJump())
	{
		const float fYaw = CBotGlobals::YawAngleFromEdict(m_pBot->GetEdict(), vWptOrigin);
		Vector vDirection(TableCos(fYaw), TableSin(fYaw), 0);
		Vector vLateral(-vDirection.y, vDirection.x, 0);

		float fGround;
		Vector vNormal;
		Vector vOrigin = m_pBot->GetOrigin();
		if(GetSimpleGroundHeight(vOrigin + Vector(0, 0, StepHeight), &fGround, &vNormal))
		{
			const Vector vVelocity;
			CClassInterface::GetVelocity(m_pBot->GetEdict(), (Vector *)&vVelocity);
			Vector vFrom;
			Vector vTo;
			const float feelerOffset = 15.0f;
			const float feelerHeight = StepHeight + 0.1f;	// if obstacle is lower than StepHeight, we'll walk right over it
			const float feelerLength = RemapValClamped(vVelocity.Length(), 80.0f, 520.0f, 20.0f, 50.0f); // length proportional to our current velocity
			const float avoidRange = RemapValClamped(vVelocity.Length(), 80.0f, 520.0f, 150.0f, 300.0f);

			vDirection = CrossProduct(vLateral, vNormal);
			vLateral = CrossProduct(vDirection, vNormal);

			vOrigin.z += feelerHeight;

			vFrom = vOrigin + feelerOffset * vLateral;
			vTo = vFrom + feelerLength * vDirection;

			bool leftClear = IsWalkableTraceLineClear(vFrom, vTo, WALK_THRU_DOORS|WALK_THRU_BREAKABLES);

			vFrom = vOrigin - feelerOffset * vLateral;
			vTo = vFrom + feelerLength * vDirection;

			bool rightClear = IsWalkableTraceLineClear(vFrom, vTo, WALK_THRU_DOORS|WALK_THRU_BREAKABLES);

			if(!rightClear)
			{
				if(leftClear)
				{
					// right hit, left clear - veer left
					m_vOffset = avoidRange * vLateral;
				}
			}
			else if(!leftClear)
			{
				// right clear, left hit - veer right
				m_vOffset = -avoidRange * vLateral;
			}

			m_bOffsetApplied = true;
		}
	}

	m_pBot->SetMoveTo(vWptOrigin + m_vOffset);

	if(ComputeAimVector(&qAim))
	{
		AngleVectors(qAim, &vAim);
		m_pBot->SetAiming(vWptOrigin + (vAim * 1024));
	}

	/*if ( !m_pBot->HasEnemy() && (fBelief >= (fPrevBelief+10.0f)) )
		m_pBot->SetLookAtTask(LOOK_LAST_ENEMY);
	else if ( !m_pBot->HasEnemy() && (fPrevBelief > (fBelief+10.0f)) )
	{
		m_pBot->SetLookVector(pWaypoint->GetOrigin() + pWaypoint->ApplyRadius());
		m_pBot->SetLookAtTask(LOOK_VECTOR, RandomFloat(1.0f,2.0f));
	}*/
}

void CNavMeshNavigator::RollBackPosition()
{
	m_vPreviousPoint = m_pBot->GetOrigin();
	m_CurrWaypoint = GetNearestArea(m_vPreviousPoint, false, 400.0f, true);

	if(m_CurrWaypoint == NULL)
		m_CurrWaypoint = GetNearestArea(m_pBot->GetOrigin(), false, 400.0f, true);
}

float CNavMeshNavigator::DistanceTo(Vector vOrigin)
{
	static ShortestPathCost cost;

	if(m_CurrWaypoint == NULL)
		m_CurrWaypoint = GetNearestArea(m_pBot->GetOrigin());

	if(m_CurrWaypoint != NULL)
		return TravelDistance(m_CurrWaypoint->GetCenter(), vOrigin, cost);

	return m_pBot->DistanceFrom(vOrigin);
}

float CNavMeshNavigator::DistanceTo(INavMeshArea *area)
{
	if(area != NULL)
	{
		return DistanceTo(area->GetCenter());
	}

	return 0.0f;
}

bool CNavMeshNavigator::GetCoverPosition(Vector vCoverOrigin, Vector * vCover)
{
	return false;
}

bool CNavMeshNavigator::GetHideSpotPosition(Vector vCoverOrigin, Vector *vCover)
{
	INavMeshArea *area;

	if(m_pBot->HasGoal())
		area = GetNearestArea(*m_pBot->GetGoalOrigin());
	else
		area = GetNearestArea(m_pBot->GetOrigin());

	if(area == NULL)
		return false;

	CList<INavMeshHidingSpot*> *hidingspots = area->GetHidingSpots();
	if(hidingspots->Count() <= 0)
		return false;

	INavMeshHidingSpot *spot = hidingspots->Element(RandomInt(0, hidingspots->Count()-1));
	if(spot == NULL)
		return false;

	*vCover = Vector(spot->GetX(), spot->GetY(), spot->GetZ());

	return true;
}

int CNavMeshNavigator::GetCurrentFlags()
{
	if(m_CurrWaypoint != NULL)
		return m_CurrWaypoint->GetAttributes();

	return 0;
}

bool CNavMeshNavigator::RouteFound()
{
	return !(m_iSegments == 0);
}

bool CNavMeshNavigator::ComputeAimVector(QAngle *angAim)
{
	if(m_pBot->IsTF2())
	{
		if(CTeamFortress2Mod::IsMapType(TF_MAP_CTF) || CTeamFortress2Mod::IsMapType(TF_MAP_MVM) || CTeamFortress2Mod::IsMapType(TF_MAP_SD))
		{
			CBaseEntity *pFlag = NULL;
			if((pFlag = servertools->FindEntityByClassnameNearest("item_teamflag", m_pBot->GetOrigin(), 2048.0f)) != NULL)
			{
				edict_t *pEdict = gameents->BaseEntityToEdict(pFlag);
				if(!pEdict || pEdict->IsFree())
					return false;

				*angAim = QAngle(0.0f, CBotGlobals::YawAngleFromEdict(m_pBot->GetEdict(), CBotGlobals::EntityOrigin(pEdict)), 0.0f);

				return true;
			}
		}
		else if(CTeamFortress2Mod::IsMapType(TF_MAP_CP) || CTeamFortress2Mod::IsMapType(TF_MAP_KOTH))
		{
			CBaseEntity *pPoint = NULL;
			if((pPoint = servertools->FindEntityByClassnameNearest("team_control_point", m_pBot->GetOrigin(), 2048.0f)) != NULL)
			{
				edict_t *pEdict = gameents->BaseEntityToEdict(pPoint);
				if(!pEdict || pEdict->IsFree())
					return false;

				*angAim = QAngle(0.0f, CBotGlobals::YawAngleFromEdict(m_pBot->GetEdict(), CBotGlobals::EntityOrigin(pEdict)), 0.0f);

				return true;
			}
		}
		else if(CTeamFortress2Mod::IsMapType(TF_MAP_CART) || CTeamFortress2Mod::IsMapType(TF_MAP_CARTRACE))
		{
			CBaseEntity *pCart = NULL;
			if((pCart = servertools->FindEntityByClassnameNearest("mapobj_cart_dispenser", m_pBot->GetOrigin(), 2048.0f)) != NULL)
			{
				edict_t *pEdict = gameents->BaseEntityToEdict(pCart);
				if(!pEdict || pEdict->IsFree())
					return false;

				*angAim = QAngle(0.0f, CBotGlobals::YawAngleFromEdict(m_pBot->GetEdict(), CBotGlobals::EntityOrigin(pEdict)), 0.0f);

				return true;
			}
		}
		else // form a rough estimate of the epicentre of players, simulate predicting where the most players would be
		{
			Vector vAverage(0.0f);
			int8_t count = 0;

			for(short int i = 0; i < MAX_PLAYERS; i++)
			{
				edict_t *pEdict = INDEXENT(i);
				if(!pEdict || pEdict->IsFree())
					continue;

				IPlayerInfo *pPI = playerinfomanager->GetPlayerInfo(pEdict);
				if(!pPI || !pPI->IsConnected() || pPI->IsHLTV() || pPI->IsReplay())
					continue;

				vAverage += pPI->GetAbsOrigin();
				count++;
			}

			vAverage /= (float)count;
			*angAim = QAngle(0.0f, CBotGlobals::YawAngleFromEdict(m_pBot->GetEdict(), vAverage), 0.0f);

			return true;
		}
	}

	angAim->Invalidate();
	return false;
}

#if (SOURCE_ENGINE == SE_TF2)
INavMeshArea *CNavMeshNavigator::RandomGoalNearestArea(Vector &origin, bool bHighDanger, bool bIgnoreBelief, unsigned int iFlags, int iTeam, unsigned int tfFlags)
#else
INavMeshArea *CNavMeshNavigator::RandomGoalNearestArea(Vector &origin, bool bHighDanger, bool bIgnoreBelief, unsigned int iFlags, int iTeam)
#endif
{
	INavMeshArea *pArea = NULL;

	CList<INavMeshArea*> goals;

	for(INavMeshArea *area : CollectSurroundingAreas(GetNearestArea(origin)))
	{
		if(area)
		{
			if(!iFlags || (area->GetAttributes() & iFlags) == iFlags)
		#if (SOURCE_ENGINE == SE_TF2)
			if(!tfFlags || (area->GetTFAttribs() & tfFlags) == tfFlags)
		#endif
				goals.AddToTail(area);
		}
	}

	if(!goals.Empty())
	{
		pArea = ChooseBestFromBeliefBetweenAreas(&goals, bHighDanger, bIgnoreBelief);
	}

	return pArea;
}

#if (SOURCE_ENGINE == SE_TF2)
INavMeshArea *CNavMeshNavigator::RandomGoalBetweenAreas(Vector &org1, Vector &org2, bool bHighDanger, bool bIgnoreBelief, unsigned int iFlags, int iTeam, unsigned int tfFlags)
#else
INavMeshArea *CNavMeshNavigator::RandomGoalBetweenAreas(Vector &org1, Vector &org2, bool bHighDanger, bool bIgnoreBelief, unsigned int iFlags, int iTeam)
#endif
{
	INavMeshArea *pArea = NULL;
	CList<INavMeshArea*> goals;

	for(INavMeshArea *area : CollectSurroundingAreas(GetNearestArea(org1)))
	{
		if(area)
		{
			if(!iFlags || (area->GetAttributes() & iFlags) == iFlags)
		#if (SOURCE_ENGINE == SE_TF2)
			if(!tfFlags || (area->GetTFAttribs() & tfFlags) == tfFlags)
		#endif
				goals.AddToTail(area);
		}
	}

	for(INavMeshArea *area : CollectSurroundingAreas(GetNearestArea(org2)))
	{
		if(area)
		{
			if(!iFlags || (area->GetAttributes() & iFlags) == iFlags)
		#if (SOURCE_ENGINE == SE_TF2)
			if(!tfFlags || (area->GetTFAttribs() & tfFlags) == tfFlags)
		#endif
				goals.AddToTail(area);
		}
	}

	if(!goals.Empty())
	{
		pArea = ChooseBestFromBeliefBetweenAreas(&goals, bHighDanger, bIgnoreBelief);
	}

	return pArea;
}

enum { SEARCH_FLOOR = 0, SEARCH_LADDERS = 1 };
// game/server/nav_pathfind.h
bool CNavMeshNavigator::BuildPath(INavMeshArea *startArea, INavMeshArea *goalArea, const Vector *vGoalPos, IPathCost &costFunctor, INavMeshArea **closestArea, bool bIgnoreBlockedNav, float fMaxPathLength, float fMaxAng)
{
	if(g_pNavMesh == NULL)
	{
		smutils->LogError(myself, "Couldn't build a path because the nav mesh doesn't exist!");
		return false;
	}

	if(closestArea != NULL)
	{
		*closestArea = startArea;
	}

	if(startArea == NULL)
	{
		smutils->LogError(myself, "Couldn't build a path because the start area doesn't exist!");
		return false;
	}

	if(!bIgnoreBlockedNav && (goalArea != NULL && goalArea->IsBlocked()))
	{
		return false;
	}

	startArea->SetParent(NULL);
	startArea->SetParentHow(NAV_TRAVERSE_COUNT);

	if(goalArea == NULL && vGoalPos == NULL)
	{
		smutils->LogError(myself, "Couldn't build a path because the goal area doesn't exist!");
		return false;
	}

	if(startArea == goalArea)
	{
		goalArea->SetParent(NULL);
		return true;
	}

	CNavMeshArea::ClearSearchList();

	Vector vActualPos = (vGoalPos) ? *vGoalPos : goalArea->GetCenter();

	Vector vStartPos = startArea->GetCenter();
	float fStartTotalCost = vStartPos.DistToSqr(vActualPos);
	startArea->SetTotalCost(fStartTotalCost);

	float fClosestAreaDist = fStartTotalCost;

	float fInitCost = costFunctor(startArea, NULL, NULL);
	if(fInitCost < 0.0f)
		return false;

	startArea->SetCostSoFar(fInitCost);
	startArea->AddToOpenList();

	while(!CNavMeshArea::IsOpenListEmpty())
	{
		INavMeshArea *area = CNavMeshArea::PopOpenList();

		if(!bIgnoreBlockedNav && area->IsBlocked())
			continue;

		if(area == goalArea || (!goalArea && vGoalPos && AreaContains(area, *vGoalPos)))
		{
			if(closestArea)
				*closestArea = area;

			return true;
		}

		int iSearchWhere = SEARCH_FLOOR;
		int iSearchDir = NAV_DIR_NORTH;

		CList<INavMeshArea*> floorlist = GetAdjacentList(startArea, (eNavDir)iSearchDir);

		bool bLadderUp = true;
		CList<INavMeshLadder*> ladderlist;
		int iLadderTopDir = 0;

		while(true)
		{
			INavMeshArea *newArea = NULL;
			eNavTraverse iTraverseHow = NAV_TRAVERSE_COUNT;
			INavMeshLadder *ladder = NULL;

			if(iSearchWhere == SEARCH_FLOOR)
			{
				if(floorlist.Empty())
				{
					iSearchDir++;

					if(iSearchDir == NAV_DIR_COUNT)
					{
						iSearchWhere = SEARCH_LADDERS;

						ladderlist = GetLadderList(startArea, NAV_LADDER_DIR_UP);
						iLadderTopDir = 0;
					}
					else
					{
						floorlist = GetAdjacentList(startArea, (eNavDir)iSearchDir);
					}

					continue;
				}

				newArea = floorlist.Pop();
				iTraverseHow = (eNavTraverse)iSearchDir;
			}
			else if(iSearchWhere == SEARCH_LADDERS)
			{
				if(ladderlist.Empty())
				{
					if(!bLadderUp)
					{
						ladder = NULL;
						break;
					}
					else
					{
						bLadderUp = false;
						ladderlist = GetLadderList(startArea, NAV_LADDER_DIR_DOWN);
					}

					continue;
				}

				ladder = ladderlist.Pop();

				if(bLadderUp)
				{
					switch(iLadderTopDir)
					{
						case 0:
							newArea = GetAreaByID(ladder->GetTopForwardAreaID());
							break;
						case 1:
							newArea = GetAreaByID(ladder->GetTopLeftAreaID());
							break;
						case 2:
							newArea = GetAreaByID(ladder->GetTopRightAreaID());
							break;
						default:
							iLadderTopDir = 0;
							continue;
					}

					iTraverseHow = GO_LADDER_UP;
					iLadderTopDir++;
				}
				else
				{
					newArea = GetAreaByID(ladder->GetBottomAreaID());
					iTraverseHow = GO_LADDER_DOWN;
				}

				if(newArea == NULL)
					continue;

				// don't backtrack
				if(newArea == area->GetParent())
					continue;

				if(newArea == area)
					continue;

				if(newArea == startArea)
					continue;

				if(newArea->IsBlocked())
					continue;

				Vector vNewAreaCenter = newArea->GetCenter();
				if(fMaxAng != 0.0f)
				{
					Vector vAreaCenter = area->GetCenter();
					//We don't have to look for the step size, if we go off the clip.
					if(vAreaCenter.z <= vNewAreaCenter.z)
					{
						float flH = vNewAreaCenter.DistToSqr(vAreaCenter);
						Vector vFakePoint(vNewAreaCenter.x, vNewAreaCenter.y, vAreaCenter.z);
						float flA = vFakePoint.DistToSqr(vAreaCenter);
						if(fMaxAng < (180 * (cos(flA / flH)) / M_PI))
							continue;
					}
				}

				// check if cost functor says this area is a dead-end
				float fNewCost = costFunctor(newArea, area, ladder);
				// a large number is better than no number
				if(IS_NAN(fNewCost)) fNewCost = 1e30f;
				if(fNewCost <= 0.0f) continue;

				// floating point precision paranoia
				float minNewCost = area->GetCostSoFar() * 1.00001f + 0.00001f;
				fNewCost = max(fNewCost, minNewCost);

				if(fMaxPathLength >= 0.01f)
				{
					Vector vAreaCenter = area->GetCenter();

					float fDeltaLength = (vNewAreaCenter - vAreaCenter).Length();
					float fNewLengthSoFar = area->GetLengthSoFar() + fDeltaLength;
					if(fNewLengthSoFar > fMaxPathLength)
					{
						// we've hit the max distance we can travel
						continue;
					}

					newArea->SetLengthSoFar(fNewLengthSoFar);
				}

				if((newArea->IsOpen() || newArea->IsClosed()) && newArea->GetCostSoFar() <= fNewCost)
				{
					// this is a worse path - skip it
					continue;
				}
				else
				{
					float fDistSq = (vNewAreaCenter - vActualPos).LengthSqr();
					float fNewRemainingCost = (fDistSq > 0.0f) ? FastSqrt(fDistSq) : 0.0f;

					// track closest area to goal in case path fails
					if(closestArea && fNewRemainingCost < fClosestAreaDist)
					{
						*closestArea = newArea;
						fClosestAreaDist = fNewRemainingCost;
					}

					newArea->SetCostSoFar(fNewCost);
					newArea->SetTotalCost(fNewCost + fNewRemainingCost);

					if(newArea->IsClosed())
						newArea->RemoveFromClosedList();

					if(newArea->IsOpen())
						newArea->UpdateOnOpenList();
					else
						newArea->AddToOpenList();

					newArea->SetParent(area);
					newArea->SetParentHow(iTraverseHow);
				}
			}
		}

		// we have searched this area
		area->AddToClosedList();
	}

	return false;
}

INavMeshArea *CNavMeshNavigator::GetArea(const Vector &vPos, float fBeneathLimit)
{
	if(g_pNavMesh == NULL)
	{
		smutils->LogError(myself, "Can't retrieve area because the navmesh doesn't exist!");
		return NULL;
	}

	return g_pNavMesh->GetArea(vPos, fBeneathLimit);;
}

INavMeshArea *CNavMeshNavigator::GetNearestArea(const Vector &vPos, bool bAnyZ, float fMaxDist, bool bCheckLOS, bool bCheckGround, int iTeam)
{
	if(g_pNavMesh == NULL)
	{
		smutils->LogError(myself, "Can't retrieve area because the navmesh doesn't exist!");
		return NULL;
	}

	return g_pNavMesh->GetNearestArea(vPos, bAnyZ, fMaxDist, bCheckLOS, bCheckGround, iTeam);

	INavMeshGrid *grid = g_pNavMesh->GetGrid();
	if(grid == NULL)
	{
		smutils->LogError(myself, "Can't retrieve area because the grid doesn't exist!");
		return NULL;
	}

	INavMeshArea *closest = NULL;
	float fClosest = fMaxDist * fMaxDist;
	if(!bCheckLOS && !bCheckGround)
	{
		closest = GetArea(vPos);
		if(closest) return closest;
	}

	float fHeight;
	if(!GetGroundHeight(vPos, &fHeight))
	{
		if(!bCheckGround)
		{
			fHeight = vPos.z;
		}
		else
		{
			return NULL;
		}
	}

	fHeight += HalfHumanHeight;

	Vector vSource(vPos.x, vPos.y, fHeight);

	CNavMeshArea::MakeNewMarker();

	int iGridX = g_pNavMesh->WorldToGridX(vPos.x);
	int iGridY = g_pNavMesh->WorldToGridY(vPos.y);

	int iShiftLimit = Ceil2Int(fMaxDist / GridCellSize);
	for(int iShift = 0; iShift <= iShiftLimit; iShift++)
	{
		for(int x = (iGridX - iShift); x <= (iGridX + iShift); x++)
		{
			if(x < 0 || x > grid->GetGridSizeX())
				continue;

			for(int y = (iGridY - iShift); y <= (iGridY + iShift); y++)
			{
				if(y < 0 || y > grid->GetGridSizeY())
					continue;

				if(x > (iGridX - iShift) &&
				   x < (iGridX + iShift) &&
				   y >(iGridY - iShift) &&
				   y < (iGridY + iShift))
				{
					continue;
				}

				CList<INavMeshArea*> areas = *g_pNavMesh->GetAreasOnGrid(x, y);

				while(!areas.Empty())
				{
					INavMeshArea *area = areas.Pop();

					if(area->IsMarked())
						continue;	// we've already visited this area

					area->Mark();

					Vector vAreaPos = GetClosestPointOnArea(area, vSource);
					float fDistSqr = (vAreaPos - vSource).LengthSqr();
					if(fDistSqr < fClosest)
					{
						if(bCheckLOS)
						{
							Vector vSafePos;
							static CTraceFilterCustom filter;
							CBotGlobals::TraceLine(vSource, vAreaPos + Vector(0.0f, 0.0f, HalfHumanHeight), MASK_PLAYERSOLID_BRUSHONLY, &filter);
							trace_t *trace = CBotGlobals::GetTraceResult();

							if(trace->fraction <= 0.0f)
							{
								vSafePos = trace->endpos + Vector(0.0f, 0.0f, 1.0f);
							}
							else
							{
								vSafePos = vPos;
							}

							float fHeightDelta = fabs(vAreaPos.z - vSafePos.z);
							if(fHeightDelta > StepHeight)
							{
								Vector vStartPos(vAreaPos.x, vAreaPos.y, vAreaPos.z + StepHeight);
								trace->endpos = vAreaPos;
								CBotGlobals::TraceLine(vSource, trace->endpos, MASK_PLAYERSOLID_BRUSHONLY, &filter);
								trace = CBotGlobals::GetTraceResult();

								if(trace->fraction < 1.0f)
								{
									continue;
								}
							}
						}

						fClosest = fDistSqr;
						closest = area;
						iShiftLimit = iShift + 1;
					}
				}
			}
		}
	}

	return closest;
}

INavMeshArea *CNavMeshNavigator::GetAreaByID(const unsigned int iAreaIndex)
{
	if(g_pNavMesh == NULL)
	{
		smutils->LogError(myself, "Can't retrieve area because the navmesh doesn't exist!");
		return NULL;
	}

	return g_pNavMesh->GetAreaByID(iAreaIndex);
}

Vector CNavMeshNavigator::GetClosestPointOnArea(INavMeshArea *area, const Vector &vPos)
{
	if(area == NULL)
		return vPos;

	Vector vExtLo = area->GetExtentLow();
	Vector vExtHi = area->GetExtentHigh();
	Vector vClosest(0.0f);

	if(vPos.x < vExtLo.x)
	{
		if(vPos.y < vExtLo.y)
		{
			// position is north-west of area
			return vExtLo;
		}
		else if(vPos.y > vExtHi.y)
		{
			// position is south-west of area
			vClosest.x = vExtLo.x;
			vClosest.y = vExtHi.y;
		}
		else
		{
			// position is west of area
			vClosest.x = vExtLo.x;
			vClosest.y = vPos.y;
		}
	}
	else if(vPos.x > vExtHi.x)
	{
		if(vPos.y < vExtLo.y)
		{
			// position is north-east of area
			vClosest.x = vExtHi.x;
			vClosest.y = vExtLo.y;
		}
		else if(vPos.y > vExtHi.y)
		{
			// position is south-east of area
			vClosest = vExtHi;
		}
		else
		{
			// position is east of area
			vClosest.x = vExtHi.x;
			vClosest.y = vPos.y;
		}
	}
	else if(vPos.y < vExtLo.y)
	{
		// position is north of area
		vClosest.x = vPos.x;
		vClosest.y = vExtLo.y;
	}
	else if(vPos.y > vExtHi.y)
	{
		// position is south of area
		vClosest.x = vPos.x;
		vClosest.y = vExtHi.y;
	}
	else
	{
		// position is inside of area - it is the 'closest point' to itself
		vClosest = vPos;
	}

	vClosest.z = GetZ(area, vClosest);
	return vClosest;
}

bool CNavMeshNavigator::GetGroundHeight(const Vector vPos, float *fHeight, Vector *vNormal)
{
	static CTraceFilterCustom filter;

	static const float fMaxOffset = 100.0;

	Vector vTo, vFrom;
	vTo.Init(vPos.x, vPos.y, vPos.z - 1e4f);
	vFrom.Init(vPos.x, vPos.y, vPos.z + HalfHumanHeight + 0.001);

	while((vTo.z - vPos.z) < fMaxOffset)
	{
		CBotGlobals::TraceLine(vFrom, vTo, MASK_NPCSOLID_BRUSHONLY, &filter);
		trace_t *trace = CBotGlobals::GetTraceResult();

		float fFraction = trace->fraction;
		Vector vPlaneNormal = trace->plane.normal;
		Vector vEndPos = trace->endpos;

		if(fFraction >= 1.0 || ((vFrom.z - vEndPos.z) >= HalfHumanHeight))
		{
			*fHeight = vEndPos.z;
			if(vNormal)
				*vNormal = vPlaneNormal;

			if(m_PrevWaypoint && IsAreaOverlapping(m_PrevWaypoint, vPos))
				*fHeight = Max((*fHeight), GetZ(m_PrevWaypoint, vPos));

			return true;
		}

		vTo.z = (fFraction == 0.0) ? vFrom.z : vEndPos.z;
		vFrom.z = vTo.z + HalfHumanHeight + 0.001;
	}

	*fHeight = 0.0;
	if(vNormal)
		vNormal->Init(0.0f, 0.0f, 1.0f);

	return false;
}

bool CNavMeshNavigator::GetSimpleGroundHeight(const Vector vPos, float *fHeight, Vector *vNormal)
{
	static CTraceFilterCustom filter;
	// assumes vPos is playing nice
	Vector vTo;
	vTo.Init(vPos.x, vPos.y, vPos.z - 1e4f);

	CBotGlobals::TraceLine(vPos, vTo, MASK_NPCSOLID_BRUSHONLY, &filter);
	trace_t *trace = CBotGlobals::GetTraceResult();

	if(trace->startsolid)
		return false;

	*fHeight = trace->endpos.z;

	if(vNormal)
		*vNormal = trace->plane.normal;

	if(m_PrevWaypoint && IsAreaOverlapping(m_PrevWaypoint, vPos))
		*fHeight = Max(*fHeight, GetZ(m_PrevWaypoint, vPos));

	return true;
}

bool CNavMeshNavigator::IsConnected(INavMeshArea *fromArea, INavMeshArea *toArea)
{
	for(int dir = 0; dir < NAV_DIR_COUNT; dir++)
	{
		CList<INavMeshConnection*> *connections = fromArea->GetConnections((eNavDir)dir);
		for(int i = 0; i < connections->Count(); i++)
		{
			INavMeshConnection *connection = connections->Element(i);
			if(connection->GetConnectingAreaID() == toArea->GetID())
				return true;
		}
	}

	return false;
}

bool CNavMeshNavigator::IsUnderwater(INavMeshArea *area)
{
	Vector vPos = area->GetCenter();
	if(!GetGroundHeight(vPos, &vPos.z))
		return false;

	vPos.z += 2.f;
	return (engtrace->GetPointContents(vPos) & MASK_WATER) != 0;
}

CList<INavMeshArea*> CNavMeshNavigator::CollectSurroundingAreas(INavMeshArea *fromArea, float fTravelDistanceLimit, float fMaxStepUpLimit, float fMaxDropDownLimit)
{
	if(g_pNavMesh == NULL)
	{
		smutils->LogError(myself, "Could not collect surrounding areas because the navmesh doesn't exist!");
		return NULL;
	}

	if(fromArea == NULL)
	{
		smutils->LogError(myself, "Could not colect surrounding areas because the starting area doesn't exist!");
		return NULL;
	}

	CList<INavMeshArea*> ret;

	CNavMeshArea::ClearSearchList();

	fromArea->AddToOpenList();
	fromArea->SetTotalCost(0.0f);
	fromArea->SetCostSoFar(0.0f);
	fromArea->SetParent(NULL);
	fromArea->SetParentHow(NAV_TRAVERSE_COUNT);
	fromArea->Mark();

	while(!CNavMeshArea::IsOpenListEmpty())
	{
		INavMeshArea *area = CNavMeshArea::PopOpenList();
		if(fTravelDistanceLimit > 0.0f && area->GetCostSoFar() > fTravelDistanceLimit)
			continue;

		INavMeshArea *parent = area->GetParent();
		if(parent)
		{
			float DeltaZ = ComputeAdjacentAreaHeightChange(parent, area);
			if(DeltaZ > fMaxStepUpLimit) continue;
			if(DeltaZ < -fMaxDropDownLimit) continue;
		}

		ret.AddToTail(area);
		area->Mark();

		for(int dir = 0; dir < NAV_DIR_COUNT; dir++)
		{
			CList<INavMeshArea*> connections = GetAdjacentList(area, (eNavDir)dir);

			while(!connections.Empty())
			{
				INavMeshArea *adjacent = connections.Pop();

				if(adjacent->IsBlocked())
					continue;

				if(!adjacent->IsMarked())
				{
					adjacent->SetTotalCost(0.0f);
					adjacent->SetParent(area);
					adjacent->SetParentHow((eNavTraverse)dir);

					float fDistAlong = area->GetCostSoFar();
					Vector vAreaCenter = area->GetCenter();
					Vector vAdjacentCenter = adjacent->GetCenter();

					fDistAlong += vAdjacentCenter.DistToSqr(vAreaCenter);
					adjacent->SetCostSoFar(fDistAlong);
					adjacent->AddToOpenList();
				}
			}
		}
	}

	return ret;
}

CList<INavMeshArea*> CNavMeshNavigator::GetAdjacentList(INavMeshArea *fromArea, eNavDir iDirection)
{
	CList<INavMeshArea*> ret;
	if(g_pNavMesh == NULL)
		return ret;

	if(fromArea->GetConnections(iDirection)->Empty())
		return ret;

	for(INavMeshConnection *connection : *fromArea->GetConnections(iDirection))
	{
		for(INavMeshArea *area : *g_pNavMesh->GetAreas())
		{
			if(connection->GetConnectingAreaID() == area->GetID())
				ret.AddToTail(area);
		}
	}

	return ret;
}

CList<INavMeshLadder*> CNavMeshNavigator::GetLadderList(INavMeshArea *fromArea, eNavLadderDir iDirection)
{
	CList<INavMeshLadder*> ret;
	if(g_pNavMesh == NULL)
		return ret;

	if(fromArea->GetLadderConnections(iDirection)->Empty())
		return ret;

	for(INavMeshLadderConnection *laddercon : *fromArea->GetLadderConnections(iDirection))
	{
		for(INavMeshLadder *ladder : *g_pNavMesh->GetLadders())
		{
			if(laddercon->GetConnectingLadderID() == ladder->GetID())
				ret.AddToTail(ladder);
		}
	}

	return ret;
}

bool CNavMeshNavigator::IsAreaOverlapping(INavMeshArea *area, const Vector vPos, float flTolerance)
{
	if(g_pNavMesh == NULL)
		return false;

	Vector vExtLo = area->GetExtentLow();
	Vector vExtHi = area->GetExtentHigh();

	if(vPos.x + flTolerance >= vExtLo.x && vPos.x - flTolerance <= vExtHi.x &&
	   vPos.y + flTolerance >= vExtLo.y && vPos.y - flTolerance <= vExtHi.y)
	{
		return true;
	}

	return false;
}

bool CNavMeshNavigator::IsAreaOverlapping(INavMeshArea *fromArea, INavMeshArea *toArea)
{
	if(g_pNavMesh == NULL)
		return false;

	Vector vFromExtLo = fromArea->GetExtentLow();
	Vector vFromExtHi = fromArea->GetExtentHigh();

	Vector vToExtLo = toArea->GetExtentLow();
	Vector vToExtHi = toArea->GetExtentHigh();

	if(vToExtLo.x < vFromExtHi.x && vToExtHi.x > vFromExtLo.x &&
	   vToExtLo.y < vFromExtHi.y && vToExtHi.y > vFromExtLo.y)
	{
		return true;
	}

	return false;
}

bool CNavMeshNavigator::AreaContains(INavMeshArea *area, const Vector vPos)
{
	if(g_pNavMesh == NULL)
		return false;

	if(!IsAreaOverlapping(area, vPos))
		return false;

	if((GetZ(area, vPos) - StepHeight) > vPos.z)
		return false;

	return true;
}

bool CNavMeshNavigator::AreaIsEdge(INavMeshArea *area, eNavDir iDirection)
{
	CList<INavMeshArea*> areas = GetAdjacentList(area, iDirection);
	if(!areas || areas.Empty())
	{
		return true;
	}

	return false;
}

float CNavMeshNavigator::GetZ(INavMeshArea *area, const Vector &vPos)
{
	if(g_pNavMesh == NULL)
		return 0.0f;

	if(!vPos.IsValid())
		return 0.0f;

	if(area == NULL)
		return 0.0f;

	Vector vExtLo = area->GetExtentLow();
	Vector vExtHi = area->GetExtentHigh();

	float dx = vExtHi.x - vExtLo.x;
	float dy = vExtHi.y - vExtLo.y;

	// Catch divide by zero
	if(dx == 0.0f || dy == 0.0f)
		return area->GetNECornerZ();

	float u = clamp((vPos.x - vExtLo.x) / dx, 0.0f, 1.0f);
	float v = clamp((vPos.y - vExtLo.y) / dy, 0.0f, 1.0f);

	float fNorthZ = vExtLo.z + u * (area->GetNECornerZ() - vExtLo.z);
	float fSouthZ = area->GetSWCornerZ() + u * (vExtHi.z - area->GetSWCornerZ());

	return fNorthZ + v * (fSouthZ - fNorthZ);
}

float CNavMeshNavigator::GetZ(INavMeshArea *area, float flX, float flY)
{
	Vector vPos(flX, flY, 0.0f);
	return GetZ(area, vPos);
}

bool CNavMeshNavigator::ComputePortal(INavMeshArea *fromArea, INavMeshArea *toArea, eNavDir iDirection, Vector *vCenter, float *fHalfWidth)
{
	if(g_pNavMesh == NULL)
		return false;

	if(fromArea == NULL || toArea == NULL)
		return false;

	Vector vFromExtLo = fromArea->GetExtentLow();
	Vector vFromExtHi = fromArea->GetExtentHigh();

	Vector vToExtLo = toArea->GetExtentLow();
	Vector vToExtHi = toArea->GetExtentHigh();

	if(iDirection == NAV_DIR_NORTH || iDirection == NAV_DIR_SOUTH)
	{
		if(iDirection == NAV_DIR_NORTH)
		{
			vCenter->y = vFromExtLo.y;
		}
		else
		{
			vCenter->y = vFromExtHi.y;
		}

		float fLeft = vFromExtLo.x > vToExtLo.x ? vFromExtLo.x : vToExtLo.x;
		float fRight = vFromExtHi.x < vToExtHi.x ? vFromExtHi.x : vToExtHi.x;

		if(fLeft < vFromExtLo.x) fLeft = vFromExtLo.x;
		else if(fLeft > vFromExtHi.x) fLeft = vFromExtHi.x;

		if(fRight < vFromExtLo.x) fRight = vFromExtLo.x;
		else if(fRight > vFromExtHi.x) fRight = vFromExtHi.x;

		vCenter->x = (fLeft + fRight) / 2.0;
		if(fHalfWidth)
			*fHalfWidth = (fRight - fLeft) / 2.0;
	}
	else
	{
		if(iDirection == NAV_DIR_WEST)
		{
			vCenter->x = vFromExtLo.x;
		}
		else
		{
			vCenter->x = vFromExtHi.x;
		}

		float fTop = vFromExtLo.y > vToExtLo.y ? vFromExtLo.y : vToExtLo.y;
		float fBottom = vFromExtHi.y < vToExtHi.y ? vFromExtHi.y : vToExtHi.y;

		if(fTop < vFromExtLo.y) fTop = vFromExtLo.y;
		else if(fTop > vFromExtHi.y) fTop = vFromExtHi.y;

		if(fBottom < vFromExtLo.y) fBottom = vFromExtLo.y;
		else if(fBottom > vFromExtHi.y) fBottom = vFromExtHi.y;

		vCenter->y = (fTop + fBottom) / 2.0;
		if(fHalfWidth)
			*fHalfWidth = (fBottom - fTop) / 2.0;
	}

	vCenter->z = GetZ(fromArea, vCenter->x, vCenter->y);

	return true;
}

bool CNavMeshNavigator::ComputeClosestPointInPortal(INavMeshArea *fromArea, INavMeshArea *toArea, eNavDir iDirection, const Vector &vFromPos, Vector *vClosestPos)
{
	if(g_pNavMesh == NULL)
		return false;

	static float fMargin = 25.0;

	Vector vFromExtLo = fromArea->GetExtentLow();
	Vector vFromExtHi = fromArea->GetExtentHigh();

	Vector vToExtLo = toArea->GetExtentLow();
	Vector vToExtHi = toArea->GetExtentHigh();

	if(iDirection == NAV_DIR_NORTH || iDirection == NAV_DIR_SOUTH)
	{
		if(iDirection == NAV_DIR_NORTH)
		{
			vClosestPos->y = vFromExtLo.y;
		}
		else
		{
			vClosestPos->y = vFromExtHi.y;
		}

		float fLeft = fmax(vFromExtLo.x, vToExtLo.x);
		float fRight = fmin(vFromExtHi.x, vToExtHi.x);

		float fLeftMargin = AreaIsEdge(toArea, NAV_DIR_WEST) ? (fLeft + fMargin) : fLeft;
		float fRightMargin = AreaIsEdge(toArea, NAV_DIR_EAST) ? (fRight - fMargin) : fRight;

		if(fLeftMargin > fRightMargin)
		{
			float fMid = (fLeft + fRight) / 2.0;
			fLeftMargin = fMid;
			fRightMargin = fMid;
		}

		if(vFromPos.x < fLeftMargin)
		{
			vClosestPos->x = fLeftMargin;
		}
		else if(vFromPos.x > fRightMargin)
		{
			vClosestPos->x = fRightMargin;
		}
		else
		{
			vClosestPos->x = vFromPos.x;
		}
	}
	else
	{
		if(iDirection == NAV_DIR_WEST)
		{
			vClosestPos->x = vFromExtLo.x;
		}
		else
		{
			vClosestPos->x = vFromExtHi.x;
		}

		float fTop = fmax(vFromExtLo.y, vToExtLo.y);
		float fBottom = fmin(vFromExtHi.y, vToExtHi.y);

		float fTopMargin = AreaIsEdge(toArea, NAV_DIR_NORTH) ? (fTop + fMargin) : fTop;
		float fBottomMargin = AreaIsEdge(toArea, NAV_DIR_SOUTH) ? (fBottom - fMargin) : fBottom;

		if(fTopMargin > fBottomMargin)
		{
			float fMid = (fTop + fBottom) / 2.0;
			fTopMargin = fMid;
			fBottomMargin = fMid;
		}

		if(vFromPos.y < fTopMargin)
		{
			vClosestPos->y = fTopMargin;
		}
		else if(vFromPos.y > fBottomMargin)
		{
			vClosestPos->y = fBottomMargin;
		}
		else
		{
			vClosestPos->y = vFromPos.y;
		}
	}

	vClosestPos->z = GetZ(fromArea, vClosestPos->x, vClosestPos->y);

	return true;
}

bool CNavMeshNavigator::ComputeNormal(INavMeshArea *area, Vector *vNormal, bool bAlternate)
{
	if(!vNormal)
		return false;

	Vector vExtLo = area->GetExtentLow();
	Vector vExtHi = area->GetExtentHigh();

	Vector u, v;
	if(!bAlternate)
	{
		u.x = vExtHi.x - vExtLo.x;
		u.y = 0.0f;
		u.z = area->GetNECornerZ() - vExtLo.z;

		v.x = 0.0f;
		v.y = vExtHi.y - vExtLo.y;
		v.z = area->GetSWCornerZ() - vExtLo.z;
	}
	else
	{
		u.x = vExtLo.x - vExtHi.x;
		u.y = 0.0f;
		u.z = area->GetSWCornerZ() - vExtHi.z;

		v.x = 0.0f;
		v.y = vExtLo.y - vExtHi.y;
		v.z = area->GetNECornerZ() - vExtHi.z;
	}

	*vNormal = CrossProduct(u, v);
	vNormal->NormalizeInPlace();
	return true;
}

bool CNavMeshNavigator::ComputeDirection(INavMeshArea *area, const Vector &vPos, eNavDir *iDirection)
{
	if(g_pNavMesh == NULL)
	{
		*iDirection = NAV_DIR_COUNT;
		return false;
	}

	if(!iDirection)
		return false;

	Vector vExtLo = area->GetExtentLow();
	Vector vExtHi = area->GetExtentHigh();

	if(vPos.x >= vExtLo.x && vPos.x <= vExtHi.x)
	{
		if(vPos.y < vExtLo.y)
		{
			*iDirection = NAV_DIR_NORTH;
			return true;
		}
		else if(vPos.y > vExtHi.y)
		{
			*iDirection = NAV_DIR_SOUTH;
			return true;
		}
	}
	else if(vPos.y >= vExtLo.y && vPos.y <= vExtHi.y)
	{
		if(vPos.x < vExtLo.x)
		{
			*iDirection = NAV_DIR_WEST;
			return true;
		}
		else if(vPos.x > vExtHi.x)
		{
			*iDirection = NAV_DIR_EAST;
			return true;
		}
	}

	Vector vCenter = area->GetCenter();

	Vector vTo;
	VectorSubtract(vPos, vCenter, vTo);

	if(fabs(vTo.x) > fabs(vTo.y))
	{
		if(vTo.x > 0.0)
		{
			*iDirection = NAV_DIR_EAST;
			return true;
		}

		*iDirection = NAV_DIR_WEST;
		return true;
	}
	else
	{
		if(vTo.y > 0.0)
		{
			*iDirection = NAV_DIR_SOUTH;
			return true;
		}

		*iDirection = NAV_DIR_NORTH;
		return true;
	}

	return false;
}

float CNavMeshNavigator::GetLightIntensity(INavMeshArea *area, const Vector &vPos)
{
	if(g_pNavMesh == NULL)
		return 0.0f;

	Vector vExtLo = area->GetExtentLow();
	Vector vExtHi = area->GetExtentHigh();

	Vector vTestPos;
	vTestPos.x = clamp(vPos.x, vExtLo.x, vExtHi.x);
	vTestPos.y = clamp(vPos.y, vExtLo.y, vExtHi.y);
	vTestPos.z = vPos.z;

	float dX = (vTestPos.x - vExtLo.x) / (vExtHi.x - vExtLo.x);
	float dY = (vTestPos.y - vExtLo.y) / (vExtHi.y - vExtLo.y);

	float fCornerLightIntensityNW;
	float fCornerLightIntensityNE;
	float fCornerLightIntensitySW;
	float fCornerLightIntensitySE;

	CList<INavMeshCornerLightIntensity*> *intensities = area->GetCornerLightIntensities();
	for(int i = 0; i < intensities->Count(); i++)
	{
		INavMeshCornerLightIntensity *intensity = intensities->Element(i);
		if(intensity->GetCornerType() == NAV_CORNER_NORTH_WEST)
			fCornerLightIntensityNW = intensity->GetLightIntensity();
		else if(intensity->GetCornerType() == NAV_CORNER_NORTH_EAST)
			fCornerLightIntensityNE = intensity->GetLightIntensity();
		else if(intensity->GetCornerType() == NAV_CORNER_SOUTH_WEST)
			fCornerLightIntensitySW = intensity->GetLightIntensity();
		else if(intensity->GetCornerType() == NAV_CORNER_SOUTH_EAST)
			fCornerLightIntensitySE = intensity->GetLightIntensity();
	}

	float fNorthLight = fCornerLightIntensityNW * (1.0 - dX) + fCornerLightIntensityNE * dX;
	float fSouthLight = fCornerLightIntensitySW * (1.0 - dX) + fCornerLightIntensitySE * dX;

	return (fNorthLight * (1.0 - dY) + fSouthLight * dY);
}

float CNavMeshNavigator::ComputeAdjacentAreaHeightChange(INavMeshArea *fromArea, INavMeshArea *toArea)
{
	bool bFoundArea = false;
	eNavDir iNavDirection;

	for(int dir = 0; dir < NAV_DIR_COUNT; dir++)
	{
		CList<INavMeshConnection*> *connections = fromArea->GetConnections((eNavDir)dir);
		for(int i = 0; i < connections->Count(); i++)
		{
			INavMeshConnection *connection = connections->Element(i);
			if(connection->GetConnectingAreaID() == toArea->GetID())
			{
				bFoundArea = true;
				iNavDirection = connection->GetDirection();
			}

			if(bFoundArea) break;
		}
	}

	// the given areas aren't connected
	if(!bFoundArea) return 99999999.9f;

	Vector vMyCenter;
	ComputePortal(fromArea, toArea, iNavDirection, &vMyCenter, NULL);

	Vector vHisCenter;
	ComputePortal(fromArea, toArea, OppositeDirection(iNavDirection), &vHisCenter, NULL);

	return vHisCenter.z - vMyCenter.z;
}

eNavDir CNavMeshNavigator::OppositeDirection(eNavDir iNavDirection)
{
	switch(iNavDirection)
	{
		case NAV_DIR_NORTH: return NAV_DIR_SOUTH;
		case NAV_DIR_SOUTH: return NAV_DIR_NORTH;
		case NAV_DIR_EAST: return NAV_DIR_WEST;
		case NAV_DIR_WEST: return NAV_DIR_EAST;
		default: return NAV_DIR_NORTH;
	}
}

float CNavMeshNavigator::DirectionToAngle(eNavDir iNavDirection)
{
	switch(iNavDirection)
	{
		case NAV_DIR_NORTH:	return 270.0f;
		case NAV_DIR_SOUTH:	return 90.0f;
		case NAV_DIR_EAST:	return 0.0f;
		case NAV_DIR_WEST:	return 180.0f;
		default: return 0.0f;
	}
}

eNavDir CNavMeshNavigator::AngleToDirection(float fAngle)
{
	while(fAngle < 0.0f)
		fAngle += 360.0f;

	while(fAngle > 360.0f)
		fAngle -= 360.0f;

	if(fAngle < 45 || fAngle > 315)
		return NAV_DIR_EAST;

	if(fAngle >= 45 && fAngle < 135)
		return NAV_DIR_SOUTH;

	if(fAngle >= 135 && fAngle < 225)
		return NAV_DIR_WEST;

	return NAV_DIR_NORTH;
}

void CNavMeshNavigator::AddDirectionVector(Vector *v, eNavDir iDirection, float fAmount)
{
	switch(iDirection)
	{
		case NAV_DIR_NORTH: v->y -= fAmount; return;
		case NAV_DIR_SOUTH: v->y += fAmount; return;
		case NAV_DIR_EAST:  v->x += fAmount; return;
		case NAV_DIR_WEST:  v->x -= fAmount; return;
	}
}

void CNavMeshNavigator::DirectionToVector(eNavDir iDirection, Vector *v)
{
	v->Init();
	switch(iDirection)
	{
		case NAV_DIR_NORTH: v->x =  0.0f; v->y = -1.0f; break;
		case NAV_DIR_SOUTH: v->x =  0.0f; v->y =  1.0f; break;
		case NAV_DIR_EAST:  v->x =  1.0f; v->y =  0.0f; break;
		case NAV_DIR_WEST:  v->x = -1.0f; v->y =  0.0f; break;
	}
}

Vector CNavMeshNavigator::GetLadderNormal(INavMeshLadder *ladder)
{
	Vector normal;
	normal.Init();
	AddDirectionVector(&normal, ladder->GetDirection(), 1.0f);	// worst-case, we have the NavDirType as a normal

	Vector from = (ladder->GetTop() + ladder->GetBottom()) * 0.5f + normal * 5.0f;
	Vector to = from - normal * 32.0f;

	CTraceFilterWalkableEntities filter(NULL, COLLISION_GROUP_NONE, WALK_THRU_EVERYTHING);
	CBotGlobals::TraceLine(from, to, MASK_PLAYERSOLID_BRUSHONLY, &filter);
	trace_t *result = CBotGlobals::GetTraceResult();

	if(result->fraction != 1.0f)
	{
		if(result->contents & CONTENTS_LADDER)
		{
			normal = result->plane.normal;
		}
	}
}

bool CNavMeshNavigator::IsWalkableTraceLineClear(Vector vFrom, Vector vTo, unsigned int iFlags)
{
	CTraceFilterWalkableEntities filter(NULL, COLLISION_GROUP_NONE, iFlags);
	Vector vUseFrom = vFrom;
	float fFraction = 0.0f;

	for(short int t = 0; t < 50; t++)
	{
		CBotGlobals::TraceLine(vUseFrom, vTo, MASK_PLAYERSOLID, &filter);
		trace_t *trace = CBotGlobals::GetTraceResult();
		fFraction = trace->fraction;

		// if we hit a walkable entity, try again
		if(fFraction != 1.0f && IsEntityWalkable(trace->m_pEnt, iFlags))
		{
			// start from just beyond where we hit
			Vector vDir = vTo - vFrom;
			vDir.NormalizeInPlace();
			vUseFrom = trace->endpos + 5.0f * vDir;
		}
		else break;
	}

	if(fFraction == 1.0f) return true;

	return false;
}

void CNavMeshNavigator::AddAreaToOpenList(INavMeshArea *area, INavMeshArea *parent, const Vector &vStart, float fMaxRange)
{
	if(area == NULL)
		return;

	if(parent == NULL)
		return;

	if(!area->IsMarked())
	{
		area->Mark();
		area->SetTotalCost(0.0f);
		area->SetParent(parent);

		if(fMaxRange > 0.0f)
		{
			// make sure this area overlaps range
			Vector closePos = GetClosestPointOnArea(area, vStart);
			if((closePos - vStart).AsVector2D().IsLengthLessThan(fMaxRange))
			{
				// compute approximate distance along path to limit travel range, too
				float fDistAlong = parent->GetCostSoFar();
				fDistAlong += (area->GetCenter() - parent->GetCenter()).Length();
				area->SetCostSoFar(fDistAlong);

				// allow for some fudge due to large size areas
				if(fDistAlong <= 1.5f * fMaxRange)
					area->AddToOpenList();
			}
		}
		else
		{
			// infinite range
			area->AddToOpenList();
		}
	}
}

float CNavMeshNavigator::TravelDistance(const Vector &vStart, const Vector &vGoal, IPathCost &func)
{
	INavMeshArea *fromArea = GetNearestArea(vStart);
	if(fromArea == NULL)
		return -1.0f;

	// start pos and goal are too near eachother, consider it already reached
	if((vGoal - vStart).IsLengthLessThan(5.f))
		return 0.0f;

	// compute path between areas using given cost heuristic
	INavMeshArea *toArea = NULL;
	if(!BuildPath(fromArea, NULL, &vGoal, func, &toArea))
		return -1.0f;

	if(toArea->GetParent() == NULL)
	{
		// both points are in the same area - return euclidean distance
		return (vGoal - vStart).Length();
	}
	else
	{
		INavMeshArea *area;
		float fDistance;

		// goal is assumed to be inside goal area (or very close to it) - skip to next area
		area = toArea->GetParent();
		fDistance = (vGoal - area->GetCenter()).Length();

		while(area->GetParent() != NULL)
		{
			fDistance += (area->GetCenter() - area->GetParent()->GetCenter()).Length();
			area = area->GetParent();
		}

		// add in distance to startPos
		fDistance += (vStart - area->GetCenter()).Length();

		return fDistance;
	}
}

float CNavMeshNavigator::TravelDistance(INavMeshArea *fromArea, INavMeshArea *toArea, IPathCost &func)
{
	if(fromArea == NULL)
		return -1.0f;

	if(toArea == NULL)
		return -1.0f;

	if(fromArea == toArea)
		return 0.0f;

	// compute path between areas using given cost heuristic
	if(!BuildPath(fromArea, toArea, NULL, func))
		return -1.0f;

	// compute distance along path
	float fDistance = 0.0f;
	for(INavMeshArea *area = toArea; area->GetParent(); area = area->GetParent())
	{
		fDistance += (area->GetCenter() - area->GetParent()->GetCenter()).Length();
	}

	return fDistance;
}

#endif // USE_NAVMESH