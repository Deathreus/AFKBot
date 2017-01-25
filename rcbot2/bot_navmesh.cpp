/*
 */

#include "bot_base.h"
#include "bot_profile.h"
#include "bot_globals.h"
#include "bot_schedule.h"
#include "bot_navmesh.h"

extern IVDebugOverlay *debugoverlay;
extern IServerTools *servertools;

extern ConVar bot_belief_fade;

INavMesh *m_pNavMesh;

void CNavMeshNavigator::FreeMapMemory()
{
	BeliefSave(true);
	m_vGoals->Resize(0);
}
void CNavMeshNavigator::FreeMemory()
{
	delete m_pNavMesh;
	m_pNavMesh = NULL;
}

bool CNavMeshNavigator::Init(char *error, size_t maxlen)
{
	INavMeshLoader *loader = new CNavMeshLoader(CBotGlobals::GetMapName());
	m_pNavMesh = loader->Load(error, maxlen);

	return m_pNavMesh != NULL;
}

bool CNavMeshNavigator::BeliefLoad()
{
	int iSize;
	int iDesiredSize;
	register unsigned short int i;
	register unsigned short int num;
	unsigned short int *filebelief = NULL;
	char filename[1024];

	m_bLoadBelief = false;
	m_iBeliefTeam = m_pBot->GetTeam();

	smutils->BuildPath(Path_SM, filename, sizeof(filename), "data\\afkbot\\nav\\%s%d.%s", CBotGlobals::GetMapName(), m_iBeliefTeam, BOT_WAYPOINT_BELIEF_EXTENTION);

	FILE *bfp = CBotGlobals::OpenFile(filename, "rb");

	if (bfp == NULL)
	{
		Msg(" *** Can't open Waypoint belief array for reading!\n");
		return false;
	}

	fseek(bfp, 0, SEEK_END); // seek at end

	iSize = ftell(bfp); // Get file size
	iDesiredSize = m_pNavMesh->GetAreas()->Size()*sizeof(unsigned short int);

	// size not right, return false to re workout table
	if (iSize != iDesiredSize)
	{
		fclose(bfp);
		return false;
	}

	fseek(bfp, 0, SEEK_SET); // seek at start

	memset(filebelief, 0, sizeof(unsigned short int)*m_pNavMesh->GetAreas()->Size());

	fread(filebelief, sizeof(unsigned short int), m_pNavMesh->GetAreas()->Size(), bfp);

	// convert from short int to float

	num = (unsigned short int)m_pNavMesh->GetAreas()->Size();

	// quick loop
	for (i = 0; i < num; i++)
	{
		m_fBelief[i] = (((float)filebelief[i]) / 32767) * MAX_BELIEF;
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

	if ((m_pBot->GetTeam() == m_iBeliefTeam) && !bOverride)
		return false;

	memset(filebelief, 0, sizeof(unsigned short int)*m_pNavMesh->GetAreas()->Size());

	// m_iBeliefTeam is the team we've been using -- we might have changed team now
	// so would need to change files if a different team
	// stick to the current team we've been using
	smutils->BuildPath(Path_SM, filename, sizeof(filename), "data\\afkbot\\nav\\%s%d.%s", CBotGlobals::GetMapName(), m_iBeliefTeam, BOT_WAYPOINT_BELIEF_EXTENTION);

	FILE *bfp = CBotGlobals::OpenFile(filename, "rb");

	if (bfp != NULL)
	{
		fseek(bfp, 0, SEEK_END); // seek at end

		iSize = ftell(bfp); // Get file size
		iDesiredSize = m_pNavMesh->GetAreas()->Size()*sizeof(unsigned short int);

		// size not right, return false to re workout table
		if (iSize != iDesiredSize)
		{
			fclose(bfp);
		}
		else
		{
			fseek(bfp, 0, SEEK_SET); // seek at start

			if (bfp)
				fread(filebelief, sizeof(unsigned short int), m_pNavMesh->GetAreas()->Size(), bfp);

			fclose(bfp);
		}
	}

	bfp = CBotGlobals::OpenFile(filename, "wb");

	if (bfp == NULL)
	{
		m_bLoadBelief = true;
		m_iBeliefTeam = m_pBot->GetTeam();
		Msg(" *** Can't open Waypoint Belief array for writing!\n");
		return false;
	}

	// convert from short int to float

	num = (unsigned short int)m_pNavMesh->GetAreas()->Size();

	// quick loop
	for (i = 0; i < num; i++)
	{
		filebelief[i] = (filebelief[i] / 2) + ((unsigned short int)((m_fBelief[i] / MAX_BELIEF) * 16383));
	}

	fseek(bfp, 0, SEEK_SET); // seek at start

	fwrite(filebelief, sizeof(unsigned short int), num, bfp);

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
	if (iBeliefType == BELIEF_SAFETY)
	{
		if (m_fBelief[iAreaID] > 0)
			m_fBelief[iAreaID] *= bot_belief_fade.GetFloat();
		if (m_fBelief[iAreaID] < 0)
			m_fBelief[iAreaID] = 0;
	}
	else // danger	
	{
		if (m_fBelief[iAreaID] < MAX_BELIEF)
			m_fBelief[iAreaID] += (2048.0f / fDist);
		if (m_fBelief[iAreaID] > MAX_BELIEF)
			m_fBelief[iAreaID] = MAX_BELIEF;
	}

	m_bBeliefChanged = true;
}

// Get belief nearest to current origin using waypoints to store belief
void CNavMeshNavigator::Belief(Vector vOrigin, Vector vOther, float fBelief,
	float fStrength, BotBelief iType)
{
	static int iAreaID;
	INavMeshArea *pArea;

	SurroundingList *list = CollectSurroundingAreas(GetNearestArea(vOrigin));

	for (unsigned int i = 0; i < list->Size(); i++)
	{
		pArea = list->At(i);
		iAreaID = pArea->GetID();

		if (iType == BELIEF_SAFETY)
		{
			if (m_fBelief[iAreaID] > 0)
				m_fBelief[iAreaID] *= bot_belief_fade.GetFloat();
			if (m_fBelief[iAreaID] < 0)
				m_fBelief[iAreaID] = 0;
		}
		else if (iType == BELIEF_DANGER)
		{
			if (m_fBelief[iAreaID] < MAX_BELIEF)
				m_fBelief[iAreaID] += (fStrength / (vOrigin - GetCenter(pArea)).Length())*fBelief;
			if (m_fBelief[iAreaID] > MAX_BELIEF)
				m_fBelief[iAreaID] = MAX_BELIEF;
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
	if (m_CurrWaypoint != NULL)
	{
		return m_fBelief[m_CurrWaypoint->GetID()];
	}

	return 0;
}

bool CNavMeshNavigator::WorkRoute(Vector vFrom, Vector vTo, bool *bFail, bool bRestart, bool bNoInterruptions, INavMeshArea *goal, int iConditions, int iDangerId)
{
	extern ConVar bot_debug_show_route;
	ShortestPathCost cost;

	if (bRestart)
	{
		if (WantToSaveBelief())
			BeliefSave();
		if (WantToLoadBelief())
			BeliefLoad();

		*bFail = false;

		m_bWorkingRoute = true;

		if (goal != NULL)
			m_GoalWaypoint = goal;
		else
			m_GoalWaypoint = GetNearestArea(vTo);

		if (m_GoalWaypoint == NULL)
		{
			*bFail = true;
			m_bWorkingRoute = false;
			return true;
		}

		m_vPreviousPoint = vFrom;

		Vector vIgnore;
		float fIgnoreSize;
		bool bIgnore = m_pBot->GetIgnoreBox(&vIgnore, &fIgnoreSize) && TravelDistance(GetCenter(m_GoalWaypoint), vFrom, cost) > (fIgnoreSize * 2);

		m_CurrWaypoint = GetNearestArea(vFrom);

		if (m_CurrWaypoint == NULL)
		{
			*bFail = true;
			m_bWorkingRoute = false;
			return true;
		}

		CNavMeshArea::ClearSearchList();

		m_CurrWaypoint->AddToOpenList();
		m_CurrWaypoint->SetTotalCost(m_pBot->DistanceFrom(vTo));

		m_vGoals->Resize(0);
	}

	if (m_GoalWaypoint == NULL || m_CurrWaypoint == NULL)
	{
		*bFail = true;
		m_bWorkingRoute = false;
		return true;
	}

	float fBeliefSensitivity = 1.5f;
	if (iConditions & CONDITION_COVERT)
		fBeliefSensitivity = 2.0f;

	INavMeshArea *pClosest;
	Vector vCenter, vCenterPortal, vClosest;
	if (BuildPath(m_CurrWaypoint, m_GoalWaypoint, NULL, cost, &pClosest))
	{
		INavMeshArea *pTempArea = pClosest;
		INavMeshArea *pTempParent = pClosest->GetParent();
		eNavDir iDirection;
		float flHalfWidth;

		m_vGoals->Push(vTo);

		while (pTempArea != NULL)
		{
			vCenter = GetCenter(pTempArea);
			ComputeDirection(pTempArea, vCenter, &iDirection);
			ComputePortal(pTempArea, pTempParent, iDirection, &vCenterPortal, &flHalfWidth);
			ComputeClosestPointInPortal(pTempArea, pTempParent, iDirection, vCenterPortal, &vClosest);

			vClosest.z = GetZ(pTempArea, vClosest);

			m_vGoals->Push(vClosest);

			pTempArea = pTempParent;
			pTempParent = pTempArea->GetParent();
		}
	}

	if (m_vGoals->Size() > 0)
	{
		m_vGoal = m_vGoals->Tail();

#ifdef	_WINDOWS
		if (bot_debug_show_route.GetBool())
		{
			for (int j = m_vGoals->Size() - 1; j > 0; j--)
			{
				Vector vFromPos, vToPos;
				vFromPos = m_vGoals->At(j);
				vToPos = m_vGoals->At(j - 1);

				debugoverlay->AddLineOverlay(vFromPos, vToPos, 0, 255, 30, true, 0.2);
			}
		}
#endif


	}
}

INavMeshArea *CNavMeshNavigator::ChooseBestFromBelief(IList<INavMeshArea*> *goals, bool bHighDanger, int iSearchFlags, int iTeam)
{
	INavMeshArea *ret = NULL;
	INavMeshArea *checkarea;
	ShortestPathCost cost;
	float fBelief = 0;
	float fSelect;
	float bBeliefFactor = 1.0f;

	if (goals->Size() <= 1)
		return goals->At(0);

	for (unsigned int i = 0; i < goals->Size(); i++)
	{
		INavMeshArea *area = goals->At(i);
		bBeliefFactor = 1.0f;

		if (iSearchFlags & WPT_SEARCH_AVOID_SENTRIES)
		{
			for (int j = gpGlobals->maxClients; j > 0; j--)
			{
				edict_t *pSentry = CTeamFortress2Mod::GetSentryGun(j - 1);
				Vector vOrigin = CBotGlobals::EntityOrigin(pSentry);

				if (pSentry != NULL)
				{
					if (TravelDistance(GetCenter(area), vOrigin, cost) < 200.0f)
					{
						bBeliefFactor *= 0.1f;
					}
				}
			}
		}

		if (iSearchFlags & WPT_SEARCH_AVOID_SNIPERS)
		{
			for (int j = gpGlobals->maxClients; j > 0; j--)
			{
				edict_t *pPlayer = INDEXENT(j);
				Vector vOrigin = CBotGlobals::EntityOrigin(pPlayer);

				if ((pPlayer != NULL) && !pPlayer->IsFree() && (CClassInterface::GetTF2Class(pPlayer) == TF_CLASS_SNIPER))
				{
					if ((iTeam == 0) || (iTeam == CClassInterface::GetTeam(pPlayer)))
					{
						if (TravelDistance(GetCenter(area), vOrigin, cost) < 200.0f)
						{
							bBeliefFactor *= 0.1f;
						}
					}
				}
			}
		}

		if (iSearchFlags & WPT_SEARCH_AVOID_TEAMMATE)
		{
			for (int j = gpGlobals->maxClients; j > 0; j--)
			{
				edict_t *pPlayer = INDEXENT(j);
				Vector vOrigin = CBotGlobals::EntityOrigin(pPlayer);

				if ((pPlayer != NULL) && !pPlayer->IsFree())
				{
					if ((iTeam == 0) || (iTeam == CClassInterface::GetTeam(pPlayer)))
					{
						if (TravelDistance(GetCenter(area), vOrigin, cost) < 200.0f)
						{
							bBeliefFactor *= 0.1f;
						}
					}
				}
			}
		}

		if (bHighDanger)
		{
			fBelief += bBeliefFactor * (1.0f + (m_fBelief[area->GetID()]));
		}
		else
		{
			fBelief += bBeliefFactor * (1.0f + (MAX_BELIEF - (m_fBelief[area->GetID()])));
		}
	}

	fSelect = RandomFloat(0, fBelief);

	fBelief = 0;

	for (unsigned int i = 0; i < goals->Size(); i++)
	{
		checkarea = goals->At(i);

		bBeliefFactor = 1.0f;

		if (iSearchFlags & WPT_SEARCH_AVOID_SENTRIES)
		{
			for (int j = gpGlobals->maxClients; j > 0; j--)
			{
				edict_t *pSentry = CTeamFortress2Mod::GetSentryGun(j);
				Vector vOrigin = CBotGlobals::EntityOrigin(pSentry);

				if (pSentry != NULL)
				{
					if (TravelDistance(GetCenter(checkarea), vOrigin, cost) < 200.0f)
					{
						bBeliefFactor *= 0.1f;
					}
				}
			}
		}

		if (iSearchFlags & WPT_SEARCH_AVOID_SNIPERS)
		{
			for (int j = gpGlobals->maxClients; j > 0; j--)
			{
				edict_t *pPlayer = INDEXENT(j);
				Vector vOrigin = CBotGlobals::EntityOrigin(pPlayer);

				if ((pPlayer != NULL) && !pPlayer->IsFree() && (CClassInterface::GetTF2Class(pPlayer) == TF_CLASS_SNIPER))
				{
					if (TravelDistance(GetCenter(checkarea), vOrigin, cost) < 200.0f)
					{
						bBeliefFactor *= 0.1f;
					}
				}
			}
		}

		if (iSearchFlags & WPT_SEARCH_AVOID_TEAMMATE)
		{
			for (int j = gpGlobals->maxClients; j > 0; j--)
			{
				edict_t *pPlayer = INDEXENT(i);
				Vector vOrigin = CBotGlobals::EntityOrigin(pPlayer);

				if ((pPlayer != NULL) && !pPlayer->IsFree())
				{
					if (TravelDistance(GetCenter(checkarea), vOrigin, cost) < 200.0f)
					{
						bBeliefFactor *= 0.1f;
					}
				}
			}
		}

		if (bHighDanger)
		{
			fBelief += bBeliefFactor * (1.0f + (m_fBelief[checkarea->GetID()]));
		}
		else
		{
			fBelief += bBeliefFactor * (1.0f + (MAX_BELIEF - (m_fBelief[checkarea->GetID()])));
		}

		if (fSelect <= fBelief)
		{
			ret = checkarea;
			break;
		}
	}

	if (ret == NULL)
		ret = goals->At(RandomInt(0, goals->Size()));

	return ret;
}

INavMeshArea *CNavMeshNavigator::ChooseBestFromBeliefBetweenAreas(IList<INavMeshArea*> *goals, bool bHighDanger, bool bIgnoreBelief)
{
	INavMeshArea *ret = NULL;
	float fBelief = 0;
	float fSelect;

	if (goals->Size() <= 1)
		return goals->At(0);

	for (unsigned int i = 0; i < goals->Size(); i++)
	{
		INavMeshArea *area = goals->At(i);

		if (bIgnoreBelief)
		{
			if (bHighDanger)
				fBelief += area->GetTotalCost();
			else
				fBelief += (131072.0f - area->GetTotalCost());
		}
		else if (bHighDanger)
			fBelief += m_fBelief[area->GetID()] + area->GetTotalCost();
		else
			fBelief += MAX_BELIEF - m_fBelief[area->GetID()] + (131072.0f - area->GetTotalCost());
	}

	fSelect = RandomFloat(0, fBelief);

	fBelief = 0;

	for (unsigned int i = 0; i < goals->Size(); i++)
	{
		INavMeshArea *area = goals->At(i);

		if (bIgnoreBelief)
		{
			if (bHighDanger)
				fBelief += area->GetTotalCost();
			else
				fBelief += (131072.0f - area->GetTotalCost());
		}
		else if (bHighDanger)
			fBelief += m_fBelief[area->GetID()] + area->GetTotalCost();
		else
			fBelief += MAX_BELIEF - m_fBelief[area->GetID()] + (131072.0f - area->GetTotalCost());

		if (fSelect <= fBelief)
		{
			ret = area;
			break;
		}
	}

	if (ret == NULL)
		ret = goals->At(RandomInt(0, goals->Size()));

	return ret;
}

// if bot has a current position to walk to return the boolean
bool CNavMeshNavigator::HasNextPoint()
{
	return m_CurrWaypoint != NULL;
}

bool CNavMeshNavigator::GetNextRoutePoint(Vector *point)
{
	if (m_vGoals->Size() > 0)
	{
		Vector vGoal = m_vGoals->Tail();

		if (vGoal.IsValid())
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
	return GetCenter(m_CurrWaypoint);
}

bool CNavMeshNavigator::CanGetTo(Vector vOrigin)
{
	ShortestPathCost cost;

	INavMeshArea *fromArea = GetNearestArea(m_pBot->GetOrigin());
	INavMeshArea *toArea = GetNearestArea(vOrigin);

	return BuildPath(fromArea, toArea, NULL, cost);
}

void CNavMeshNavigator::UpdatePosition()
{
	static Vector vWptOrigin;
	static float fRadius;
	static float fPrevBelief, fBelief;

	static Vector v_dynamic;

	static QAngle qAim;
	static Vector vAim;

	static bool bTouched;

	fPrevBelief = 0;
	fBelief = 0;

	if (m_CurrWaypoint == NULL) // invalid
	{
		m_pBot->StopMoving();
		m_bOffsetApplied = false;
		return;
	}

	INavMeshArea *area = m_CurrWaypoint; // Shorten the pointer name

	fRadius = (area->GetSWCornerZ() - area->GetNECornerZ());

	vWptOrigin = GetCenter(area);

	v_dynamic = (GetCenter(area) + m_vOffset);

	if (!m_bWorkingRoute)
	{
		bool movetype_ok = CClassInterface::IsMoveType(m_pBot->GetEdict(), MOVETYPE_LADDER) || CClassInterface::IsMoveType(m_pBot->GetEdict(), MOVETYPE_FLYGRAVITY);

		bTouched = (m_pBot->GetOrigin() - v_dynamic).Length() < m_pBot->GetTouchDistance();

		if (bTouched)
		{
			int iWaypointID = area->GetID();
			int iWaypointFlagsPrev = 0;

			fPrevBelief = GetBelief(iWaypointID);

			// Bot passed into this waypoint safely, update belief

			bot_statistics_t *stats = m_pBot->GetStats();

			if (stats)
			{
				if ((stats->stats.m_iEnemiesVisible > stats->stats.m_iTeamMatesVisible) && (stats->stats.m_iEnemiesInRange > 0))
					BeliefOne(iWaypointID, BELIEF_DANGER, 100.0f);
				else if ((stats->stats.m_iTeamMatesVisible > 0) && (stats->stats.m_iTeamMatesInRange > 0))
					BeliefOne(iWaypointID, BELIEF_SAFETY, 100.0f);
			}

			m_bOffsetApplied = false;

			m_bDangerPoint = false;


			if (m_vGoals->Size() <= 0) // reached goal!!
			{
				// fix: bots jumping at wrong positions
				m_pBot->TouchedWpt(area, -1);


				m_vPreviousPoint = m_pBot->GetOrigin();
				m_PrevWaypoint = m_CurrWaypoint;
				m_CurrWaypoint = NULL;

				if (m_pBot->GetSchedule()->IsCurrentSchedule(SCHED_RUN_FOR_COVER) ||
					m_pBot->GetSchedule()->IsCurrentSchedule(SCHED_GOOD_HIDE_SPOT))
					m_pBot->ReachedCoverSpot(area->GetFlags());
			}
			else
			{
				iWaypointFlagsPrev = GetCurrentFlags();
				m_vPreviousPoint = m_pBot->GetOrigin();
				m_PrevWaypoint = m_CurrWaypoint;
				m_CurrWaypoint = GetNearestArea(m_vGoals->Tail(), false, 400.0f, true);
				m_vGoals->PopList();

				// fix: bots jumping at wrong positions
				m_pBot->TouchedWpt(area);


				// fix : update pWaypoint as Current Waypoint
				area = m_CurrWaypoint;

				fBelief = GetBelief(m_CurrWaypoint->GetID());
			}
		}
	}
	else
		m_bOffsetApplied = false;

	m_pBot->WalkingTowardsWaypoint(area, &m_bOffsetApplied, m_vOffset);

	m_pBot->SetMoveTo(vWptOrigin + m_vOffset);

	CalculateAimVector(&qAim);
	AngleVectors(qAim, &vAim);
	if (vAim.IsValid())
		m_pBot->SetAiming(vWptOrigin + (vAim * 1024));

	/*if ( !m_pBot->HasEnemy() && (fBelief >= (fPrevBelief+10.0f)) )
		m_pBot->setLookAtTask(LOOK_LAST_ENEMY);
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

	if (m_CurrWaypoint == NULL)
		m_CurrWaypoint = GetNearestArea(m_pBot->GetOrigin(), false, 400.0f, true);
}

float CNavMeshNavigator::DistanceTo(Vector vOrigin)
{
	ShortestPathCost cost;

	if (m_CurrWaypoint == NULL)
		m_CurrWaypoint = GetNearestArea(m_pBot->GetOrigin());

	if (m_CurrWaypoint != NULL)
		return TravelDistance(GetCenter(m_CurrWaypoint), vOrigin, cost);

	return m_pBot->DistanceFrom(vOrigin);
}

float CNavMeshNavigator::DistanceTo(INavMeshArea *area)
{
	if (area != NULL)
	{
		return DistanceTo(GetCenter(area));
	}

	return 0.0f;
}

bool CNavMeshNavigator::GetHideSpotPosition(Vector vCoverOrigin, Vector *vCover)
{
	INavMeshArea *area;

	if (m_pBot->HasGoal())
		area = GetNearestArea(*m_pBot->GetGoalOrigin());
	else
		area = GetNearestArea(m_pBot->GetOrigin());

	if (area == NULL)
		return false;

	IList<INavMeshHidingSpot*> *hidingspots = area->GetHidingSpots();
	if (hidingspots->Size() <= 0)
		return false;

	INavMeshHidingSpot *spot = hidingspots->At(RandomInt(0, hidingspots->Size()-1));
	if (spot == NULL)
		return false;

	*vCover = Vector(spot->GetX(), spot->GetY(), spot->GetZ());

	return true;
}

int CNavMeshNavigator::GetCurrentFlags()
{
	if (m_CurrWaypoint != NULL)
		return m_CurrWaypoint->GetFlags();

	return 0;
}

bool CNavMeshNavigator::RouteFound()
{
	return (m_vGoals->Size() > 0);
}

bool CNavMeshNavigator::CalculateAimVector(QAngle *qAim)
{
	if (CTeamFortress2Mod::IsMapType(TF_MAP_CTF) || CTeamFortress2Mod::IsMapType(TF_MAP_MVM) || CTeamFortress2Mod::IsMapType(TF_MAP_SD))
	{
		CBaseEntity *pFlag = NULL;
		if ((pFlag = servertools->FindEntityByClassnameNearest("item_teamflag", m_pBot->GetOrigin(), 2500.0f)) != NULL)
		{
			edict_t *pEdict = gameents->BaseEntityToEdict(pFlag);
			if (!pEdict || pEdict->IsFree())
				return false;

			qAim->x = 0.0f;
			qAim->z = 0.0f;
			qAim->y = CBotGlobals::YawAngleFromEdict(m_pBot->GetEdict(), CBotGlobals::EntityOrigin(pEdict));

			return true;
		}
	}

	if (CTeamFortress2Mod::IsMapType(TF_MAP_CP) || CTeamFortress2Mod::IsMapType(TF_MAP_KOTH))
	{
		CBaseEntity *pPoint = NULL;
		if ((pPoint = servertools->FindEntityByClassnameNearest("team_control_point", m_pBot->GetOrigin(), 2500.0f)) != NULL)
		{
			edict_t *pEdict = gameents->BaseEntityToEdict(pPoint);
			if (!pEdict || pEdict->IsFree())
				return false;

			qAim->x = 0.0f;
			qAim->z = 0.0f;
			qAim->y = CBotGlobals::YawAngleFromEdict(m_pBot->GetEdict(), CBotGlobals::EntityOrigin(pEdict));

			return true;
		}
	}

	if (CTeamFortress2Mod::IsMapType(TF_MAP_CART) || CTeamFortress2Mod::IsMapType(TF_MAP_CARTRACE))
	{
		CBaseEntity *pCart = NULL;
		if ((pCart = servertools->FindEntityByClassnameNearest("mapobj_cart_dispenser", m_pBot->GetOrigin(), 2500.0f)) != NULL)
		{
			edict_t *pEdict = gameents->BaseEntityToEdict(pCart);
			if (!pEdict || pEdict->IsFree())
				return false;

			qAim->x = 0.0f;
			qAim->z = 0.0f;
			qAim->y = CBotGlobals::YawAngleFromEdict(m_pBot->GetEdict(), CBotGlobals::EntityOrigin(pEdict));

			return true;
		}
	}

	qAim->Invalidate();
	return false;
}

template< typename CostFunctor >
bool CNavMeshNavigator::BuildPath(INavMeshArea *startArea, INavMeshArea *goalArea, const Vector *vGoalPos, CostFunctor &costFunctor, INavMeshArea **closestArea, float fMaxPathLength, float fMaxAng, bool bIgnoreBlockedNav)
{
	if (m_pNavMesh == NULL)
	{
		smutils->LogError(myself, "Couldn't build a path because the nav mesh doesn't exist!");
		return false;
	}

	if (closestArea != NULL)
	{
		*closestArea = startArea;
	}

	if (startArea == NULL)
	{
		smutils->LogError(myself, "Couldn't build a path because the start area doesn't exist!");
		return false;
	}

	if (!bIgnoreBlockedNav && (goalArea != NULL && goalArea->IsBlocked()))
	{
		return false;
	}

	startArea->SetParent(NULL);
	startArea->SetParentHow(NAV_TRAVERSE_COUNT);

	if (goalArea == NULL && vGoalPos == NULL)
	{
		smutils->LogError(myself, "Couldn't build a path because the goal area doesn't exist!");
		return false;
	}

	if (startArea == goalArea)
	{
		goalArea->SetParent(NULL);
		return true;
	}

	CNavMeshArea::ClearSearchList();

	Vector vActualPos = (vGoalPos) ? *vGoalPos : GetCenter(goalArea);

	Vector vStartPos = GetCenter(startArea);
	float fStartTotalCost = vStartPos.DistToSqr(vActualPos);
	startArea->SetTotalCost(fStartTotalCost);

	float fClosestAreaDist = fStartTotalCost;

	float fInitCost = costFunctor(startArea, NULL, NULL);
	if (fInitCost < 0.0f)
		return false;

	startArea->SetCostSoFar(fInitCost);
	startArea->AddToOpenList();

	while (!CNavMeshArea::IsOpenListEmpty())
	{
		INavMeshArea *area = CNavMeshArea::PopOpenList();

		if (!bIgnoreBlockedNav && area->IsBlocked())
			continue;

		if (area == goalArea || (!goalArea && vGoalPos && Contains(area, *vGoalPos)))
		{
			if (closestArea)
				*closestArea = area;

			return true;
		}

		static int SEARCH_FLOOR = 0, SEARCH_LADDERS = 1;

		int iSearchWhere = SEARCH_FLOOR;
		int iSearchDir = NAV_DIR_NORTH;

		AdjacentList *floorlist = GetAdjacentList(startArea, (eNavDir)iSearchDir);

		bool bLadderUp = true;
		LadderList *ladderlist = NULL;
		int iLadderTopDir = 0;

		while (true)
		{
			INavMeshArea *newArea = NULL;
			eNavTraverse iTraverseHow = NAV_TRAVERSE_COUNT;
			INavMeshLadder *ladder = NULL;

			if (iSearchWhere == SEARCH_FLOOR)
			{
				if (floorlist->Size() <= 0)
				{
					iSearchDir++;

					if (iSearchDir == NAV_DIR_COUNT)
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

				newArea = floorlist->Tail();
				iTraverseHow = (eNavTraverse)iSearchDir;
				floorlist->PopList();
			}
			else if (iSearchWhere == SEARCH_LADDERS)
			{
				if (ladderlist->Size() <= 0)
				{
					if (!bLadderUp)
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

				ladder = ladderlist->Tail();
				ladderlist->PopList();

				if (bLadderUp)
				{
					switch (iLadderTopDir)
					{
						case 0: newArea = GetAreaByID(ladder->GetTopForwardAreaID());
						case 1: newArea = GetAreaByID(ladder->GetTopLeftAreaID());
						case 2: newArea = GetAreaByID(ladder->GetTopRightAreaID());
						default:
						{
							iLadderTopDir = 0;
							continue;
						}
					}

					iTraverseHow = GO_LADDER_UP;
					iLadderTopDir++;
				}
				else
				{
					newArea = GetAreaByID(ladder->GetBottomAreaID());
					iTraverseHow = GO_LADDER_DOWN;
				}

				if (newArea == NULL)
					continue;

				// don't backtrack
				if (newArea == area)
					continue;

				if (newArea == startArea)
					continue;

				if (newArea->IsBlocked())
					continue;

				Vector vNewAreaCenter = GetCenter(newArea);
				if (fMaxAng != 0.0)
				{
					Vector vAreaCenter = GetCenter(area);
					//We don't have to look for the step size, if we go off the clip.
					if (vAreaCenter.z <= vNewAreaCenter.z)
					{
						float flH = vNewAreaCenter.DistToSqr(vAreaCenter);
						Vector vFakePoint(vNewAreaCenter.x, vNewAreaCenter.y, vAreaCenter.z);
						float flA = vFakePoint.DistToSqr(vAreaCenter);
						if (fMaxAng < (180 * (cos(flA / flH)) / M_PI))
							continue;
					}
				}

				// check if cost functor says this area is a dead-end
				float fNewCost = costFunctor(newArea, area, ladder);
				if (fNewCost <= 0) continue;

				if (fMaxPathLength != 0.0)
				{
					Vector vAreaCenter = GetCenter(area);

					float fDeltaLength = vNewAreaCenter.DistToSqr(vAreaCenter);
					float fNewLengthSoFar = area->GetLengthSoFar() + fDeltaLength;
					if (fNewLengthSoFar > fMaxPathLength)
					{
						// we've hit the max distance we can travel
						continue;
					}

					newArea->SetLengthSoFar(fNewLengthSoFar);
				}

				if ((newArea->IsOpen() || newArea->IsClosed()) && newArea->GetCostSoFar() <= fNewCost)
				{
					// this is a worse path - skip it
					continue;
				}
				else
				{
					float fNewRemainingCost = vNewAreaCenter.DistToSqr(vActualPos);

					// track closest area to goal in case path fails
					if (closestArea && fNewRemainingCost < fClosestAreaDist)
					{
						*closestArea = newArea;
						fClosestAreaDist = fNewRemainingCost;
					}

					newArea->SetCostSoFar(fNewCost);
					newArea->SetTotalCost(fNewCost + fNewRemainingCost);

					if (newArea->IsClosed())
						newArea->RemoveFromClosedList();

					if (newArea->IsOpen())
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
	if (m_pNavMesh == NULL)
	{
		smutils->LogError(myself, "Can't retrieve area because the navmesh doesn't exist!");
		return NULL;
	}

	INavMeshGrid *grid = m_pNavMesh->GetGrid();
	if (grid == NULL)
	{
		smutils->LogError(myself, "Can't retrieve area because the grid doesn't exist!");
		return NULL;
	}

	int x = WorldToGridX(vPos.x);
	int y = WorldToGridY(vPos.y);

	IList<INavMeshArea*> *areas = GetAreasOnGrid(x, y);

	INavMeshArea *useArea = NULL;
	float fUseZ = -99999999.9f;
	Vector vTestPos = vPos + Vector(0.0f, 0.0f, 5.0f);

	if (areas)
	{
		while (areas->Size() > 0)
		{
			INavMeshArea *area = areas->Tail();
			areas->PopList();
			
			if (IsOverlapping(area, vPos))
			{
				float z = GetZ(area, vTestPos);

				if (z > vTestPos.z)
					continue;

				if (z < vPos.z - fBeneathLimit)
					continue;

				if (z > fUseZ)
				{
					useArea = area;
					fUseZ = z;
				}
			}
		}
	}

	return useArea;
}

INavMeshArea *CNavMeshNavigator::GetNearestArea(const Vector &vPos, bool bAnyZ, float fMaxDist, bool bCheckLOS, bool bCheckGround, int iTeam)
{
	if (m_pNavMesh == NULL)
	{
		smutils->LogError(myself, "Can't retrieve area because the navmesh doesn't exist!");
		return NULL;
	}

	INavMeshGrid *grid = m_pNavMesh->GetGrid();
	if (grid == NULL)
	{
		smutils->LogError(myself, "Can't retrieve area because the grid doesn't exist!");
		return NULL;
	}

	INavMeshArea *closest = NULL;
	float fClosest = fMaxDist * fMaxDist;
	if (!bCheckLOS && !bCheckGround)
	{
		closest = GetArea(vPos);
		if (closest) return closest;
	}

	float fHeight;
	if (!GetGroundHeight(vPos, &fHeight, NULL))
	{
		if (!bCheckGround)
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

	int iGridX = WorldToGridX(vPos.x);
	int iGridY = WorldToGridY(vPos.y);

	int iShiftLimit = static_cast<int>(fMaxDist / GridCellSize);
	for (int iShift = 0; iShift <= iShiftLimit; iShift++)
	{
		for (int x = (iGridX - iShift); x <= (iGridX + iShift); x++)
		{
			if (x < 0 || x > grid->GetGridSizeX())
				continue;

			for (int y = (iGridY - iShift); y <= (iGridY + iShift); y++)
			{
				if (y < 0 || y > grid->GetGridSizeY())
					continue;

				if (x > (iGridX - iShift) &&
					x < (iGridX + iShift) &&
					y > (iGridY - iShift) &&
					y < (iGridY + iShift))
				{
					continue;
				}

				IList<INavMeshArea*> *areas = GetAreasOnGrid(x, y);
				if (areas)
				{
					while (areas->Size() > 0)
					{
						INavMeshArea *area = areas->Tail();
						areas->PopList();

						if (area->IsMarked())
							continue;	// we've already visited this area

						area->Mark();

						Vector vAreaPos = GetClosestPointOnArea(area, vSource);
						float fDistSqr = (vAreaPos - vSource).LengthSqr();
						if (fDistSqr < fClosest)
						{
							if (bCheckLOS)
							{
								Vector vSafePos;
								CTraceFilterCustom filter;
								CBotGlobals::TraceLine(vSource, vAreaPos + Vector(0.0f, 0.0f, HalfHumanHeight), MASK_PLAYERSOLID_BRUSHONLY, &filter);
								trace_t *trace = CBotGlobals::GetTraceResult();

								if (trace->fraction == 0.0f)
								{
									vSafePos = trace->endpos + Vector(0.0f, 0.0f, 1.0f);
								}
								else
								{
									vSafePos = vPos;
								}

								float fHeightDelta = fabs(vAreaPos.z - vSafePos.z);
								if (fHeightDelta > StepHeight)
								{
									Vector vStartPos(vAreaPos.x, vAreaPos.y, vAreaPos.z + StepHeight);
									trace->endpos = vAreaPos;
									CBotGlobals::TraceLine(vSource, trace->endpos, MASK_PLAYERSOLID_BRUSHONLY, &filter);
									trace = CBotGlobals::GetTraceResult();
									
									if (trace->fraction != 1.0f)
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
	}

	return closest;
}

INavMeshArea *CNavMeshNavigator::GetAreaByID(const unsigned int iAreaIndex)
{
	if (m_pNavMesh == NULL)
	{
		smutils->LogError(myself, "Can't retrieve area because the navmesh doesn't exist!");
		return NULL;
	}

	IList<INavMeshArea*> *areas = m_pNavMesh->GetAreas();
	for (unsigned int i = 0; i < areas->Size(); i++)
	{
		INavMeshArea *area = areas->At(i);
		if (area->GetID() == iAreaIndex)
			return area;
	}

	return NULL;
}

int CNavMeshNavigator::WorldToGridX(float fWX)
{
	INavMeshGrid *grid = m_pNavMesh->GetGrid();
	int x = (int)((fWX - grid->GetExtentLowX()) / GridCellSize);

	if (x < 0)
		x = 0;
	else if (x >= grid->GetGridSizeX())
		x = grid->GetGridSizeX() - 1;

	return x;
}

int CNavMeshNavigator::WorldToGridY(float fWY)
{
	INavMeshGrid *grid = m_pNavMesh->GetGrid();
	int y = (int)((fWY - grid->GetExtentLowY()) / GridCellSize);

	if (y < 0)
		y = 0;
	else if (y >= grid->GetGridSizeY())
		y = grid->GetGridSizeY() - 1;

	return y;
}

Vector CNavMeshNavigator::GetExtentLow(INavMeshArea *area)
{
	if (area == NULL)
		return Vector(0.0f, 0.0f, 0.0f);

	Vector extent;
	extent.x = area->GetNWExtentX();
	extent.y = area->GetNWExtentY();
	extent.z = area->GetNWExtentZ();

	return extent;
}

Vector CNavMeshNavigator::GetExtentHigh(INavMeshArea *area)
{
	if (area == NULL)
		return Vector(0.0f, 0.0f, 0.0f);

	Vector extent;
	extent.x = area->GetSEExtentX();
	extent.y = area->GetSEExtentY();
	extent.z = area->GetSEExtentZ();

	return extent;
}

Vector CNavMeshNavigator::GetCenter(INavMeshArea *area)
{
	if (area == NULL)
		return Vector(0.0f, 0.0f, 0.0f);

	Vector center;
	Vector top = GetExtentHigh(area);
	Vector bottom = GetExtentLow(area);

	center.x = (top.x + bottom.x) / 2.0;
	center.y = (top.y + bottom.y) / 2.0;
	center.z = (top.z + bottom.z) / 2.0;

	return center;
}

Vector CNavMeshNavigator::GetClosestPointOnArea(INavMeshArea *area, const Vector &vPos)
{
	if (area == NULL)
		return Vector(0.0f, 0.0f, 0.0f);

	Vector vExtLo = GetExtentLow(area);
	Vector vExtHi = GetExtentHigh(area);
	Vector vClosest;

	if (vPos.x < vExtLo.x)
	{
		if (vPos.y < vExtLo.y)
		{
			// position is north-west of area
			return vExtLo;
		}
		else if (vPos.y > vExtHi.y)
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
	else if (vPos.x > vExtHi.x)
	{
		if (vPos.y < vExtLo.y)
		{
			// position is north-east of area
			vClosest.x = vExtHi.x;
			vClosest.y = vExtLo.y;
		}
		else if (vPos.y > vExtHi.y)
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
	else if (vPos.y < vExtLo.y)
	{
		// position is north of area
		vClosest.x = vPos.x;
		vClosest.y = vExtLo.y;
	}
	else if (vPos.y > vExtHi.y)
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
	CTraceFilterCustom filter;

	static float fMaxOffset = 100.0;

	Vector vTo, vFrom;
	vTo.Init(vPos.x, vPos.y, vPos.z - 10000.0);
	vFrom.Init(vPos.x, vPos.y, vPos.z + HalfHumanHeight + 0.001);

	while (vTo.z - vPos.z < fMaxOffset)
	{
		CBotGlobals::TraceLine(vFrom, vTo, MASK_NPCSOLID_BRUSHONLY, &filter);
		trace_t *trace = CBotGlobals::GetTraceResult();

		float fFraction = trace->fraction;
		Vector vPlaneNormal = trace->plane.normal;
		Vector vEndPos = trace->endpos;

		if (fFraction == 1.0 || ((vFrom.z - vEndPos.z) >= HalfHumanHeight))
		{
			*fHeight = vEndPos.z;
			vNormal->y = vPlaneNormal.x;
			vNormal->y = vPlaneNormal.y;
			vNormal->z = vPlaneNormal.z;
			return true;
		}

		vTo.z = (fFraction == 0.0) ? vFrom.z : vEndPos.z;
		vFrom.z = vTo.z + HalfHumanHeight + 0.001;
	}

	*fHeight = 0.0;
	vNormal->x = 0.0;
	vNormal->y = 0.0;
	vNormal->z = 1.0;

	return false;
}

bool CNavMeshNavigator::IsConnected(INavMeshArea *fromArea, INavMeshArea *toArea)
{
	IList<INavMeshConnection*> *connections = fromArea->GetConnections();
	for (unsigned int i = 0; i < connections->Size(); i++)
	{
		INavMeshConnection *connection = connections->At(i);
		if (connection->GetConnectingAreaID() == toArea->GetID())
			return true;
	}

	return false;
}

SurroundingList *CNavMeshNavigator::CollectSurroundingAreas(INavMeshArea *fromArea, float fTravelDistanceLimit, float fMaxStepUpLimit, float fMaxDropDownLimit)
{
	if (m_pNavMesh == NULL)
	{
		smutils->LogError(myself, "Could not collect surrounding areas because the navmesh doesn't exist!");
		return NULL;
	}

	if (fromArea == NULL)
	{
		smutils->LogError(myself, "Could not colect surrounding areas because the starting area doesn't exist!");
		return NULL;
	}

	SurroundingList *ret = new CList<INavMeshArea*>();

	CNavMeshArea::ClearSearchList();

	fromArea->AddToOpenList();
	fromArea->SetTotalCost(0.0f);
	fromArea->SetCostSoFar(0.0f);
	fromArea->SetParent(NULL);
	fromArea->SetParentHow(NAV_TRAVERSE_COUNT);

	while (!CNavMeshArea::IsOpenListEmpty())
	{
		INavMeshArea *area = CNavMeshArea::PopOpenList();
		if (fTravelDistanceLimit > 0.0f && area->GetCostSoFar() > fTravelDistanceLimit)
			continue;

		INavMeshArea *parent = area->GetParent();
		if (parent)
		{
			float DeltaZ = ComputeAdjacentAreaHeightChange(parent, area);
			if (DeltaZ > fMaxStepUpLimit) continue;
			if (DeltaZ < -fMaxDropDownLimit) continue;
		}

		ret->Push(area);
		area->Mark();

		for (int iNavDir = 0; iNavDir < (int)NAV_DIR_COUNT; iNavDir++)
		{
			IList<INavMeshArea*> *connections = GetAdjacentList(area, (eNavDir)iNavDir);
			if (connections)
			{
				while (connections->Size() > 0)
				{
					INavMeshArea *adjacent = connections->Tail();
					connections->PopList();

					if (adjacent->IsBlocked())
						continue;

					if (!adjacent->IsMarked())
					{
						adjacent->SetTotalCost(0.0f);
						adjacent->SetParent(area);
						adjacent->SetParentHow((eNavTraverse)iNavDir);

						float fDistAlong = area->GetCostSoFar();
						Vector vAreaCenter = GetCenter(area);
						Vector vAdjacentCenter = GetCenter(adjacent);

						fDistAlong += vAdjacentCenter.DistToSqr(vAreaCenter);
						adjacent->SetCostSoFar(fDistAlong);
						adjacent->AddToOpenList();
					}
				}
			}
		}
	}

	return ret;
}

AdjacentList *CNavMeshNavigator::GetAdjacentList(INavMeshArea *fromArea, eNavDir iDirection)
{
	if (m_pNavMesh == NULL)
		return NULL;

	AdjacentList *ret = new CList<INavMeshArea*>();

	IList<INavMeshConnection*> *connections = fromArea->GetConnections();
	for (unsigned int i = 0; i < connections->Size(); i++)
	{
		INavMeshConnection *connection = connections->At(i);
		if (connection->GetDirection() == iDirection)
		{
			IList<INavMeshArea*> *areas = m_pNavMesh->GetAreas();
			for (unsigned int ii = 0; ii < areas->Size(); ii++)
			{
				INavMeshArea *area = areas->At(ii);
				if (connection->GetConnectingAreaID() == area->GetID())
					ret->Push(area);
			}
		}
	}

	return ret;
}

LadderList *CNavMeshNavigator::GetLadderList(INavMeshArea *fromArea, eNavLadderDir iDirection)
{
	if (m_pNavMesh == NULL)
		return NULL;

	LadderList *ret = new CList<INavMeshLadder*>();

	IList<INavMeshLadderConnection*> *laddercons = fromArea->GetLadderConnections();
	for (unsigned int i = 0; i < laddercons->Size(); i++)
	{
		INavMeshLadderConnection *laddercon = laddercons->At(i);
		if (laddercon->GetDirection() == iDirection)
		{
			IList<INavMeshLadder*> *ladders = m_pNavMesh->GetLadders();
			for (unsigned int ii = 0; ii < ladders->Size(); ii++)
			{
				INavMeshLadder *ladder = ladders->At(i);
				if (laddercon->GetConnectingLadderID() == ladder->GetID())
					ret->Push(ladder);
			}
		}
	}

	return ret;
}

GridList *CNavMeshNavigator::GetAreasOnGrid(int x, int y)
{
	if (m_pNavMesh == NULL)
	{
		smutils->LogError(myself, "Could not get grid areas because the navmesh doesn't exist!");
		return NULL;
	}

	INavMeshGrid *grid = m_pNavMesh->GetGrid();
	if (grid == NULL)
	{
		smutils->LogError(myself, "Could not get grid areas because the grid doesn't exist!");
		return NULL;
	}

	int iGridIndex = x + y * grid->GetGridSizeX();
	int iListStartIndex = GetArrayCell(grid->GetGridAreas(), iGridIndex, NavMeshGrid_ListStartIndex);
	int iListEndIndex = GetArrayCell(grid->GetGridAreas(), iGridIndex, NavMeshGrid_ListEndIndex);

	if (iListStartIndex == -1)
		return NULL;

	GridList *ret = new CList<INavMeshArea*>();

	IList<INavMeshArea*> *areas = m_pNavMesh->GetAreas();
	for (int i = iListStartIndex; i <= iListEndIndex; i++)
	{
		INavMeshArea *area = areas->At(GetArrayCell(grid->GetGridList(), i, NavMeshGridList_AreaIndex));
		if (area != NULL)
			ret->Push(area);
	}

	return ret;
}

bool CNavMeshNavigator::IsOverlapping(INavMeshArea *area, const Vector vPos, float flTolerance)
{
	if (m_pNavMesh == NULL)
		return false;

	Vector vExtLo = GetExtentLow(area);
	Vector vExtHi = GetExtentHigh(area);

	if (vPos.x + flTolerance >= vExtLo.x && vPos.x - flTolerance <= vExtHi.x &&
		vPos.y + flTolerance >= vExtLo.y && vPos.y - flTolerance <= vExtHi.y) {
		return true;
	}

	return false;
}

bool CNavMeshNavigator::IsOverlapping(INavMeshArea *fromArea, INavMeshArea *toArea)
{
	if (m_pNavMesh == NULL)
		return false;

	Vector vFromExtLo = GetExtentLow(fromArea);
	Vector vFromExtHi = GetExtentHigh(fromArea);

	Vector vToExtLo = GetExtentLow(toArea);
	Vector vToExtHi = GetExtentHigh(toArea);

	if (vToExtLo.x < vFromExtHi.x && vToExtHi.x > vFromExtLo.x &&
		vToExtLo.y < vFromExtHi.y && vToExtHi.y > vFromExtLo.y) {
		return true;
	}

	return false;
}

bool CNavMeshNavigator::Contains(INavMeshArea *area, const Vector vPos)
{
	if (m_pNavMesh == NULL)
		return false;

	if (!IsOverlapping(area, vPos))
		return false;

	float fMyZ = GetZ(area, vPos);
	if ((fMyZ - StepHeight) > vPos.z)
		return false;

	IList<INavMeshArea*> *areas = m_pNavMesh->GetAreas();
	for (unsigned int i = 0; i < areas->Size(); i++)
	{
		INavMeshArea *hisarea = areas->At(i);
		if (area == hisarea)
			continue;

		if (!IsOverlapping(hisarea, vPos))
			continue;

		float fTheirZ = GetZ(hisarea, vPos);
		if ((fTheirZ - StepHeight) > vPos.z)
			continue;

		if (fTheirZ > fMyZ)
		{
			return false;
		}
	}

	return true;
}

bool CNavMeshNavigator::IsEdge(INavMeshArea *area, eNavDir iDirection)
{
	IList<INavMeshArea*> *areas = GetAdjacentList(area, iDirection);
	if (areas == NULL || areas->Size() <= 0)
	{
		return true;
	}

	return false;
}

float CNavMeshNavigator::GetZ(INavMeshArea *area, const Vector &vPos)
{
	if (m_pNavMesh == NULL)
		return 0.0f;

	if (!vPos.IsValid())
		return 0.0f;

	if (area == NULL)
		return 0.0f;

	Vector vExtLo = GetExtentLow(area);
	Vector vExtHi = GetExtentHigh(area);

	float dx = vExtHi.x - vExtLo.x;
	float dy = vExtHi.y - vExtLo.y;

	// Catch divide by zero
	if (dx == 0.0f || dy == 0.0f)
		return area->GetNECornerZ();

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
	if (m_pNavMesh == NULL)
		return false;

	if (fromArea == NULL || toArea == NULL)
		return false;

	Vector vFromExtLo = GetExtentLow(fromArea);
	Vector vFromExtHi = GetExtentHigh(fromArea);

	Vector vToExtLo = GetExtentLow(toArea);
	Vector vToExtHi = GetExtentHigh(toArea);

	if (iDirection == NAV_DIR_NORTH || iDirection == NAV_DIR_SOUTH)
	{
		if (iDirection == NAV_DIR_NORTH)
		{
			vCenter->y = vFromExtLo.y;
		}
		else
		{
			vCenter->y = vFromExtHi.y;
		}

		float fLeft = vFromExtLo.x > vToExtLo.x ? vFromExtLo.x : vToExtLo.x;
		float fRight = vFromExtHi.x < vToExtHi.x ? vFromExtHi.x : vToExtHi.x;

		if (fLeft < vFromExtLo.x) fLeft = vFromExtLo.x;
		else if (fLeft > vFromExtHi.x) fLeft = vFromExtHi.x;

		if (fRight < vFromExtLo.x) fRight = vFromExtLo.x;
		else if (fRight > vFromExtHi.x) fRight = vFromExtHi.x;

		vCenter->x = (fLeft + fRight) / 2.0;
		*fHalfWidth = (fRight - fLeft) / 2.0;
	}
	else
	{
		if (iDirection == NAV_DIR_WEST)
		{
			vCenter->x = vFromExtLo.x;
		}
		else
		{
			vCenter->x = vFromExtHi.x;
		}

		float fTop = vFromExtLo.y > vToExtLo.y ? vFromExtLo.y : vToExtLo.y;
		float fBottom = vFromExtHi.y < vToExtHi.y ? vFromExtHi.y : vToExtHi.y;

		if (fTop < vFromExtLo.y) fTop = vFromExtLo.y;
		else if (fTop > vFromExtHi.y) fTop = vFromExtHi.y;

		if (fBottom < vFromExtLo.y) fBottom = vFromExtLo.y;
		else if (fBottom > vFromExtHi.y) fBottom = vFromExtHi.y;

		vCenter->y = (fTop + fBottom) / 2.0;
		*fHalfWidth = (fBottom - fTop) / 2.0;
	}

	vCenter->z = GetZ(fromArea, vCenter->x, vCenter->y);

	return true;
}

bool CNavMeshNavigator::ComputeClosestPointInPortal(INavMeshArea *fromArea, INavMeshArea *toArea, eNavDir iDirection, const Vector &vFromPos, Vector *vClosestPos)
{
	if (m_pNavMesh == NULL)
		return false;

	static float fMargin = 25.0;

	Vector vFromExtLo = GetExtentLow(fromArea);
	Vector vFromExtHi = GetExtentHigh(fromArea);

	Vector vToExtLo = GetExtentLow(toArea);
	Vector vToExtHi = GetExtentHigh(toArea);

	if (iDirection == NAV_DIR_NORTH || iDirection == NAV_DIR_SOUTH)
	{
		if (iDirection == NAV_DIR_NORTH)
		{
			vClosestPos->y = vFromExtLo.y;
		}
		else
		{
			vClosestPos->y = vFromExtHi.y;
		}

		float fLeft = fmax(vFromExtLo.x, vToExtLo.x);
		float fRight = fmin(vFromExtHi.x, vToExtHi.x);

		float fLeftMargin = IsEdge(toArea, NAV_DIR_WEST) ? (fLeft + fMargin) : fLeft;
		float fRightMargin = IsEdge(toArea, NAV_DIR_EAST) ? (fRight - fMargin) : fRight;

		if (fLeftMargin > fRightMargin)
		{
			float fMid = (fLeft + fRight) / 2.0;
			fLeftMargin = fMid;
			fRightMargin = fMid;
		}

		if (vFromPos.x < fLeftMargin)
		{
			vClosestPos->x = fLeftMargin;
		}
		else if (vFromPos.x > fRightMargin)
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
		if (iDirection == NAV_DIR_WEST)
		{
			vClosestPos->x = vFromExtLo.x;
		}
		else
		{
			vClosestPos->x = vFromExtHi.x;
		}

		float fTop = fmax(vFromExtLo.y, vToExtLo.y);
		float fBottom = fmin(vFromExtHi.y, vToExtHi.y);

		float fTopMargin = IsEdge(toArea, NAV_DIR_NORTH) ? (fTop + fMargin) : fTop;
		float fBottomMargin = IsEdge(toArea, NAV_DIR_SOUTH) ? (fBottom - fMargin) : fBottom;

		if (fTopMargin > fBottomMargin)
		{
			float fMid = (fTop + fBottom) / 2.0;
			fTopMargin = fMid;
			fBottomMargin = fMid;
		}

		if (vFromPos.y < fTopMargin)
		{
			vClosestPos->y = fTopMargin;
		}
		else if (vFromPos.y > fBottomMargin)
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
	if (!vNormal)
		return false;

	Vector vExtLo = GetExtentLow(area);
	Vector vExtHi = GetExtentHigh(area);

	Vector u, v;
	if (!bAlternate)
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
	if (m_pNavMesh == NULL)
	{
		*iDirection = NAV_DIR_COUNT;
		return false;
	}

	if (!iDirection)
		return false;

	Vector vExtLo = GetExtentLow(area);
	Vector vExtHi = GetExtentHigh(area);

	if (vPos.x >= vExtLo.x && vPos.x <= vExtHi.x)
	{
		if (vPos.y < vExtLo.y)
		{
			*iDirection = NAV_DIR_NORTH;
			return true;
		}
		else if (vPos.y > vExtHi.y)
		{
			*iDirection = NAV_DIR_SOUTH;
			return true;
		}
	}
	else if (vPos.y >= vExtLo.y && vPos.y <= vExtHi.y)
	{
		if (vPos.x < vExtLo.x)
		{
			*iDirection = NAV_DIR_WEST;
			return true;
		}
		else if (vPos.x > vExtHi.x)
		{
			*iDirection = NAV_DIR_EAST;
			return true;
		}
	}

	Vector vCenter = GetCenter(area);

	Vector vTo;
	VectorSubtract(vPos, vCenter, vTo);

	if (fabs(vTo.x) > fabs(vTo.y))
	{
		if (vTo.x > 0.0)
		{
			*iDirection = NAV_DIR_EAST;
			return true;
		}

		*iDirection = NAV_DIR_WEST;
		return true;
	}
	else
	{
		if (vTo.y > 0.0)
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
	if (m_pNavMesh == NULL)
		return 0.0f;

	Vector vExtLo = GetExtentLow(area);
	Vector vExtHi = GetExtentHigh(area);

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

	IList<INavMeshCornerLightIntensity*> *cornerlights = area->GetCornerLightIntensities();
	for (unsigned int i = 0; i < cornerlights->Size(); i++)
	{
		INavMeshCornerLightIntensity *intensity = cornerlights->At(i);
		if (intensity->GetCornerType() == NAV_CORNER_NORTH_WEST)
			fCornerLightIntensityNW = intensity->GetLightIntensity();
		else if (intensity->GetCornerType() == NAV_CORNER_NORTH_EAST)
			fCornerLightIntensityNE = intensity->GetLightIntensity();
		else if (intensity->GetCornerType() == NAV_CORNER_SOUTH_WEST)
			fCornerLightIntensitySW = intensity->GetLightIntensity();
		else if (intensity->GetCornerType() == NAV_CORNER_SOUTH_EAST)
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

	IList<INavMeshConnection*> *connections = fromArea->GetConnections();
	for (unsigned int i = 0; i < connections->Size(); i++)
	{
		INavMeshConnection *connection = connections->At(i);
		if (connection->GetConnectingAreaID() == toArea->GetID())
		{
			bFoundArea = true;
			iNavDirection = connection->GetDirection();
		}

		if (bFoundArea) break;
	}

	if (!bFoundArea) return 99999999.9f;

	Vector vMyCenter;
	float fHalfWidth;
	ComputePortal(fromArea, toArea, iNavDirection, &vMyCenter, &fHalfWidth);

	Vector vHisCenter;
	ComputePortal(fromArea, toArea, OppositeDirection(iNavDirection), &vHisCenter, &fHalfWidth);

	return vHisCenter.z - vMyCenter.z;
}

eNavDir CNavMeshNavigator::OppositeDirection(eNavDir iNavDirection)
{
	switch (iNavDirection)
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
	switch (iNavDirection)
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
	while (fAngle < 0.0f)
		fAngle += 360.0f;

	while (fAngle > 360.0f)
		fAngle -= 360.0f;

	if (fAngle < 45 || fAngle > 315)
		return NAV_DIR_EAST;

	if (fAngle >= 45 && fAngle < 135)
		return NAV_DIR_SOUTH;

	if (fAngle >= 135 && fAngle < 225)
		return NAV_DIR_WEST;

	return NAV_DIR_NORTH;
}

bool CNavMeshNavigator::IsWalkableTraceLineClear(Vector vFrom, Vector vTo, unsigned int iFlags)
{
	CTraceFilterWalkableEntities filter(NULL, COLLISION_GROUP_NONE, iFlags);
	Vector vUseFrom = vFrom;
	float fFraction = 0.0f;

	for (short int t = 0; t < 50; t++)
	{
		CBotGlobals::TraceLine(vUseFrom, vTo, MASK_PLAYERSOLID, &filter);
		trace_t *trace = CBotGlobals::GetTraceResult();
		fFraction = trace->fraction;

		// if we hit a walkable entity, try again
		if (fFraction != 1.0f && IsEntityWalkable(trace->m_pEnt, iFlags))
		{
			// start from just beyond where we hit
			Vector vDir = vTo - vFrom;
			vDir.NormalizeInPlace();
			vUseFrom = trace->endpos + 5.0f * vDir;
		}
		else break;
	}

	if (fFraction == 1.0f) return true;

	return false;
}

void CNavMeshNavigator::AddAreaToOpenList(INavMeshArea *area, INavMeshArea *parent, const Vector &vStart, float fMaxRange)
{
	if (area == NULL)
		return;

	if (parent == NULL)
		return;

	if (!area->IsMarked())
	{
		area->Mark();
		area->SetTotalCost(0.0f);
		area->SetParent(parent);

		if (fMaxRange > 0.0f)
		{
			// make sure this area overlaps range
			Vector closePos = GetClosestPointOnArea(area, vStart);;
			if ((closePos - vStart).AsVector2D().IsLengthLessThan(fMaxRange))
			{
				// compute approximate distance along path to limit travel range, too
				float fDistAlong = parent->GetCostSoFar();
				fDistAlong += (GetCenter(area) - GetCenter(parent)).Length();
				area->SetCostSoFar(fDistAlong);

				// allow for some fudge due to large size areas
				if (fDistAlong <= 1.5f * fMaxRange)
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

template <typename CostFunctor>
float CNavMeshNavigator::TravelDistance(const Vector &vStart, const Vector &vGoal, CostFunctor &func)
{
	INavMeshArea *fromArea = GetNearestArea(vStart);
	if (fromArea == NULL)
	{
		return -1.0f;
	}

	// start pos and goal are too near eachother, consider it already reached
	if ((vGoal - vStart).IsLengthLessThan(2.5f))
	{
		return 0.0f;
	}

	// compute path between areas using given cost heuristic
	INavMeshArea *toArea = NULL;
	if (!BuildPath(fromArea, NULL, &vGoal, func, &toArea))
	{
		return -1.0f;
	}

	if (toArea->GetParent() == NULL)
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
		fDistance = (vGoal - GetCenter(area)).Length();

		for (; area->GetParent(); area = area->GetParent())
		{
			fDistance += (GetCenter(area) - GetCenter(area->GetParent())).Length();
		}

		// add in distance to startPos
		fDistance += (vStart - GetCenter(area)).Length();

		return fDistance;
	}
}

template<typename CostFunctor>
float CNavMeshNavigator::TravelDistance(INavMeshArea *fromArea, INavMeshArea *toArea, CostFunctor &func)
{
	if (fromArea == NULL)
		return -1.0f;

	if (toArea == NULL)
		return -1.0f;

	if (fromArea == toArea)
		return 0.0f;

	// compute path between areas using given cost heuristic
	if (!BuildPath(fromArea, toArea, NULL, func))
		return -1.0f;

	// compute distance along path
	float fDistance = 0.0f;
	for (INavMeshArea *area = toArea; area->GetParent(); area = area->GetParent())
	{
		fDistance += (GetCenter(area) - GetCenter(area->GetParent())).Length();
	}

	return fDistance;
}

/*void CNavMeshNavigator::PopulateAmmo()
{
	edict_t *pAmmo = NULL;
	CBaseEntity *pEnt = NULL;
	Vector vOrigin;

	while ((pEnt = servertools->FindEntityByClassname(pEnt, "ammo*")) != NULL)
	{
		pAmmo = gameents->BaseEntityToEdict(pEnt);
		vOrigin = CBotGlobals::EntityOrigin(pAmmo);
	}
}

void CNavMeshNavigator::PopulateHealth()
{
	edict_t *pHealth = NULL;
	CBaseEntity *pEnt = NULL;
	Vector vOrigin;

	while ((pEnt = servertools->FindEntityByClassname(pEnt, "healthkit*")) != NULL)
	{
		pHealth = gameents->BaseEntityToEdict(pEnt);
		vOrigin = CBotGlobals::EntityOrigin(pHealth);
	}
}*/
