#include "engine_wrappers.h"
#include "bot_base.h"
#include "bot_getprop.h"
#include "bot_fortress.h"
#include "bot_tf2_points.h"
#include "bot_globals.h"

#if defined USE_NAVMESH
#include "bot_navmesh.h"
#else
#include "bot_waypoint.h"
#include "bot_waypoint_locations.h"
#endif

#ifdef GetClassName
 #undef GetClassName
#endif

extern ConVar bot_const_point_offset;
extern ConVar bot_const_round_offset;
extern ConVar bot_tf2_autoupdate_point_time;
extern ConVar bot_tf2_payload_dist_retreat;
extern ConVar bot_defrate;

extern IServerGameEnts *gameents;

class CBotFuncResetAttackPoint : public IBotFunction
{
public:
	CBotFuncResetAttackPoint(int team) : m_iTeam(team) {}
	void Execute(CBot *pBot)
	{
		if (pBot->GetTeam() == m_iTeam)
			((CBotTF2*)pBot)->UpdateAttackPoints();
	}
private:
	int m_iTeam;
};

class CBotFuncResetDefendPoint : public IBotFunction
{
public:
	CBotFuncResetDefendPoint(int team) : m_iTeam(team) {}
	void Execute(CBot *pBot)
	{
		if (pBot->GetTeam() == m_iTeam)
			((CBotTF2*)pBot)->UpdateDefendPoints();
	}
private:
	int m_iTeam;
};

class CBotFuncPointsUpdated : public IBotFunction
{
public:
	void Execute(CBot *pBot)
	{
		((CBotTF2*)pBot)->PointsUpdated();
	}
};

bool CTeamControlPointRound::IsPointInRound(edict_t *point_ent)
{
	edict_t *pPoint;

	for(int i = 0; i < m_ControlPoints.Size(); i++)
	{
		CBaseHandle *hndl = &m_ControlPoints[i];

		if(hndl)
		{
			pPoint = INDEXENT(hndl->GetEntryIndex());

			if(pPoint)
			{
				CBaseEntity *test_ent = pPoint->GetUnknown()->GetBaseEntity();
				if(point_ent->GetUnknown()->GetBaseEntity() == test_ent)
					return true;
			}
		}
	}

	return false;
}

CTeamControlPointRound *CTeamControlPointMaster::GetCurrentRound()
{
	if(!m_ControlPointRounds.IsValidIndex(m_iCurrentRoundIndex))
		return NULL;

	CBaseEntity *pEnt = m_ControlPointRounds[m_iCurrentRoundIndex];

	return (CTeamControlPointRound*)((intptr_t)pEnt + bot_const_round_offset.GetInt());
}

float CTeamRoundTimer::GetSetupTime()
{
	Assert( m_Resource.IsValid() );
	return (float)GetEntSend<int>(m_Resource, "m_nSetupTimeLength");
}

float CTeamRoundTimer::GetEndTime()
{
	Assert( m_Resource.IsValid() );
	return GetEntSend<float>(m_Resource, "m_flTimerEndTime");
}

const bool CTeamRoundTimer::IsInSetup()
{
	Assert( m_Resource.IsValid() );
	return (GetEntSend<int>(m_Resource, "m_nState") == RT_STATE_SETUP);
}

void CTeamRoundTimer::Reset()
{
	Q_memset(this, 0, sizeof(CTeamRoundTimer));

	extern IServerTools *servertools;
	m_Resource = servertools->FindEntityByClassname(NULL, "team_round_timer");
}


const Vector CTFObjectiveResource::GetCPPosition(int index)
{
	Assert( m_Resource.IsValid() );
	AssertValidIndex(index);
	return GetEntData<Vector>(m_Resource, "m_vCPPositions", index);
}

bool CTFObjectiveResource::TeamCanCapPoint(int index, int team)
{
	Assert( m_Resource.IsValid() );
	AssertValidIndex(index);
	return GetEntData<bool>(m_Resource, "m_bTeamCanCap", TEAM_ARRAY(index, team));
}

int CTFObjectiveResource::GetOwningTeam(int index)
{
	Assert( m_Resource.IsValid() );
	AssertValidIndex(index);
	return GetEntData<int>(m_Resource, "m_iOwner", index);
}

bool CTFObjectiveResource::IsCPVisible(int index)
{
	Assert( m_Resource.IsValid() );
	AssertValidIndex(index);
	return GetEntData<bool>(m_Resource, "m_bCPIsVisible", index);
}

bool CTFObjectiveResource::IsCPBlocked(int index)
{
	Assert( m_Resource.IsValid() );
	AssertValidIndex(index);
	return GetEntSend<bool>(m_Resource, "m_bBlocked", index);
}

bool CTFObjectiveResource::IsCPLocked(int index)
{
	Assert( m_Resource.IsValid() );
	AssertValidIndex(index);
	return GetEntSend<bool>(m_Resource, "m_bCPLocked", index);
}

int CTFObjectiveResource::GetCappingTeam(int index)
{
	Assert( m_Resource.IsValid() );
	AssertValidIndex(index);
	return GetEntSend<int>(m_Resource, "m_iCappingTeam", index);
}

int CTFObjectiveResource::GetTeamInZone(int index)
{
	Assert( m_Resource.IsValid() );
	AssertValidIndex(index);
	return GetEntData<int>(m_Resource, "m_iTeamInZone", index);
}

int CTFObjectiveResource::GetNumPlayersInArea(int index, int team)
{
	Assert( m_Resource.IsValid() );
	AssertValidIndex(index);
	return GetEntData<int>(m_Resource, "m_iNumTeamMembers", TEAM_ARRAY(index, team));
}

int CTFObjectiveResource::GetRequiredCappers(int index, int team)
{
	Assert( m_Resource.IsValid() );
	AssertValidIndex(index);
	return GetEntData<int>(m_Resource, "m_iTeamReqCappers", TEAM_ARRAY(index, team));
}

int CTFObjectiveResource::GetBaseControlPointForTeam(int iTeam)
{
	Assert( m_Resource.IsValid() );
	Assert( iTeam >= 0 && iTeam < MAX_CONTROL_POINT_TEAMS );
	return GetEntSend<int>(m_Resource, "m_iBaseControlPoints", iTeam);
}

float CTFObjectiveResource::GetCPUnlockTime(int index)
{
	Assert( m_Resource.IsValid() );
	AssertValidIndex(index);
	return GetEntSend<float>(m_Resource, "m_flUnlockTimes", index);
}

int CTFObjectiveResource::GetPreviousPointForPoint(int index, int team, int iPrevIndex)
{
	Assert( m_Resource.IsValid() );
	AssertValidIndex(index);
	AssertValidIndex(iPrevIndex);
	int iIndex = iPrevIndex + (index * MAX_PREVIOUS_POINTS) + (team * MAX_CONTROL_POINTS * MAX_PREVIOUS_POINTS);
	return GetEntData<int>(m_Resource, "m_iPreviousPoints", iIndex);
}

bool CTFObjectiveResource::PlayingMiniRounds(void)
{
	Assert( m_Resource.IsValid() );
	return GetEntSend<bool>(m_Resource, "m_bPlayingMiniRounds");
}

bool CTFObjectiveResource::IsInMiniRound(int index)
{
	Assert( m_Resource.IsValid() );
	AssertValidIndex(index);
	return GetEntSend<bool>(m_Resource, "m_bInMiniRound", index);
}

// TO DO  - Base on waypoint danger
// base on base point -- if already have attack point and base point -- less focus on base point
int CTFObjectiveResource::GetRandomValidPointForTeam(int team, ePointAttackDefend type)
{
	if ((team < 2) || (team > 3))
		return 0;

	if (m_iNumControlPoints < 2)
		return 0;

	int iOtherTeam = (team == 2) ? 3 : 2;

	if(CTeamFortress2Mod::IsMapType(TF_MAP_5CP))
	{
		for(int i = 0; i < m_iNumControlPoints; i++)
		{
			if(type == TF2_POINT_ATTACK)
			{
				if(GetOwningTeam(i) != team && !IsCPLocked(i))
				#if defined USE_NAVMESH // We use the actual CP index if using navmesh
					return i;
				#else
					return m_IndexToWaypointAreaTranslation[i];
				#endif
			}
			else
			{
				if(GetOwningTeam(i) != iOtherTeam && !IsCPLocked(i))
				#if defined USE_NAVMESH // We use the actual CP index if using navmesh
					return i;
				#else
					return m_IndexToWaypointAreaTranslation[i];
				#endif
			}
		}
	}

	std::vector<int> points;
	float fTotal = 0.0f;
	TF2PointProb_t *arr = m_ValidPoints[team - 2][type];

	for (int i = 0; i < m_iNumControlPoints; i++)
	{
		if (arr[i].bValid == true)
		{
			points.push_back(i);

			if (type == TF2_POINT_ATTACK)
			{
				if (GetCappingTeam(i) == team)
					arr[i].fProbMultiplier = 3.0f;
				else if ((GetLastCaptureTime(i) + 10.0f) > TIME_NOW)
					arr[i].fProbMultiplier = 2.0f;
			}
			else
			{
				if (GetCappingTeam(i) == iOtherTeam)
				{
					int numplayers = GetNumPlayersInArea(i, iOtherTeam);

					// IF this is not base point and a lot of players are here, reduce probability of defending
					if ((i != GetBaseControlPointForTeam(team)) && (numplayers > 1))
					{
						arr[i].fProbMultiplier = 1.0f - ((float)numplayers / (MAX_PLAYERS / 4));

						if (arr[i].fProbMultiplier <= 0.0f)
							arr[i].fProbMultiplier = 0.1f;
					}
					else // Otherwise there aren't any playres on or is base and has been attacked recently
						arr[i].fProbMultiplier = 4.0f;
				}
				else if ((GetLastCaptureTime(i) + 10.0f) > TIME_NOW)
					arr[i].fProbMultiplier = 2.0f;
			}

			fTotal += arr[i].fProb*arr[i].fProbMultiplier;
		}
	}

	float fRand = RandomFloat(0.0f, fTotal);

	fTotal = 0.0f;

	for (unsigned int i = 0; i < points.size(); i++)
	{
		int index = points[i];

		fTotal += arr[index].fProb*arr[index].fProbMultiplier;

		if (fTotal > fRand)
		{
		#if defined USE_NAVMESH // We use the actual CP index if using navmesh
			return index;
		#else
			return m_IndexToWaypointAreaTranslation[index];
		#endif
		}
	}

	// no points
	return 0;
}

void CTFObjectiveResource::Reset()
{
	Q_memset(this, 0, sizeof(CTFObjectiveResource));

	extern IServerTools *servertools;
	m_Resource = servertools->FindEntityByClassname(NULL, "tf_objective_resource");
	
	Assert( m_Resource.IsValid() );
	Setup();
}

void CTFObjectiveResource::Setup()
{
	m_iNumControlPoints = GetEntSend<int>(m_Resource, "m_iNumControlPoints");

	Q_memset(m_iControlPointWpt, 0xFF, sizeof(int)*MAX_CONTROL_POINTS);

	Q_memset(m_pControlPoints, 0, sizeof(edict_t*)*MAX_CONTROL_POINTS);
	Q_memset(m_iControlPointWpt, 0xFF, sizeof(int)*MAX_CONTROL_POINTS);
	Q_memset(m_fLastCaptureTime, 0, sizeof(float)*MAX_CONTROL_POINTS);

	Q_memset(m_IndexToWaypointAreaTranslation, 0, sizeof(int)*MAX_CONTROL_POINTS);
	Q_memset(m_WaypointAreaToIndexTranslation, 0xFF, sizeof(int)*(MAX_CONTROL_POINTS + 1));

	m_iMonitorPoint[0] = -1;
	m_iMonitorPoint[1] = -1;

	edict_t *pEnt;
	Vector vOrigin;
	for(int i = MAX_PLAYERS; i < MAX_ENTITIES; ++i)
	{
		pEnt = INDEXENT(i);
		if(!pEnt || pEnt->IsFree())
			continue;

		if(strcmp(pEnt->GetClassName(), "team_control_point") == 0)
		{
			vOrigin = CBotGlobals::EntityOrigin(pEnt);

			for(int j = 0; j < m_iNumControlPoints; j++)
			{
				if(m_pControlPoints[j].Get() != NULL)
					continue;

				if(VectorsAreEqual(vOrigin, GetCPPosition(j), 1.0f))
					m_pControlPoints[j] = pEnt;
			}
		}
	}
#if defined USE_NAVMESH
	for(int i = 0; i < m_iNumControlPoints; i++)
	{
		vOrigin = GetCPPosition(i);

		if(m_iControlPointWpt[i] == -1)
		{
			INavMeshArea *area = g_pNavMesh->GetArea(vOrigin);
			if(area)
			{
				m_iControlPointWpt[i] = area->GetID();
			}
		}
	}
#else
	CWaypoint *pWaypoint;
	int iWpt;

	for (int i = 0; i < m_iNumControlPoints; i++)
	{
		vOrigin = GetCPPosition(i);

		if (m_iControlPointWpt[i] == -1)
		{
			iWpt = CWaypointLocations::NearestWaypoint(vOrigin, 1024.0f, -1, false, false, false, NULL, false, 0, false, false, Vector(0, 0, 0), CWaypointTypes::W_FL_CAPPOINT);
			pWaypoint = CWaypoints::GetWaypoint(iWpt);
			m_iControlPointWpt[i] = iWpt;

			// For compatibility -- old waypoints are already set with an area, so take the area from the waypoint here
			// in the future waypoints will automatically be set to the waypoint area anyway
			if (pWaypoint)
			{
				int iArea = pWaypoint->GetArea();
				m_IndexToWaypointAreaTranslation[i] = iArea;

				if ((iArea >= 1) && (iArea <= MAX_CONTROL_POINTS))
					m_WaypointAreaToIndexTranslation[iArea] = i;
			}
			else
			{
				m_IndexToWaypointAreaTranslation[i] = 0;
				m_WaypointAreaToIndexTranslation[i + 1] = -1;
			}
		}
	}
#endif

	m_bInitialised = true;
}

void CTFObjectiveResource::Think()
{
	if(m_bInitialised && (m_fNextCheckMonitoredPoint < TIME_NOW))
	{
		bool bUpdate = (m_fUpdatePointTime < TIME_NOW);

		int iTeam = 0;
		while(iTeam < 2 && !bUpdate)
		{
			if(m_iMonitorPoint[iTeam] != -1)
			{
				for(int j = 0; j < MAX_PREVIOUS_POINTS; j++)
				{
					int prev = GetPreviousPointForPoint(m_iMonitorPoint[iTeam], (iTeam + 2), j);

					if((prev != -1) && (GetOwningTeam(prev) != (iTeam + 2)))
					{
						bUpdate = true;
						break;
					}
				}
			}
			++iTeam;
		}

		if(bUpdate)
		{
			UpdatePoints();

			m_fNextCheckMonitoredPoint = TIME_NOW + 5.0f;
			m_fUpdatePointTime = TIME_NOW + bot_tf2_autoupdate_point_time.GetFloat();
		}
		else
			m_fNextCheckMonitoredPoint = TIME_NOW + 1.0f;
	}
}

void CTFObjectiveResource::UpdatePoints()
{
	static CBotFuncResetAttackPoint ResetBlueAttack(TF2_TEAM_BLUE);
	static CBotFuncResetDefendPoint ResetBlueDefend(TF2_TEAM_BLUE);
	static CBotFuncResetAttackPoint ResetRedAttack(TF2_TEAM_RED);
	static CBotFuncResetDefendPoint ResetRedDefend(TF2_TEAM_RED);
	static CBotFuncPointsUpdated PointsUpdated;
	bool bChanged = false;

	m_iMonitorPoint[0] = -1;
	m_iMonitorPoint[1] = -1;

	CTeamFortress2Mod::m_ObjectiveResource.ResetValidWaypointAreas();

	if(CTeamFortress2Mod::m_ObjectiveResource.UpdateAttackPoints(TF2_TEAM_BLUE))
	{
		CBots::BotFunction(ResetBlueAttack);
		bChanged = true;
	}

	if(CTeamFortress2Mod::m_ObjectiveResource.UpdateAttackPoints(TF2_TEAM_RED))
	{
		CBots::BotFunction(ResetRedAttack);
		bChanged = true;
	}

	if(CTeamFortress2Mod::m_ObjectiveResource.UpdateDefendPoints(TF2_TEAM_BLUE))
	{
		CBots::BotFunction(ResetBlueDefend);
		bChanged = true;
	}

	if(CTeamFortress2Mod::m_ObjectiveResource.UpdateDefendPoints(TF2_TEAM_RED))
	{
		CBots::BotFunction(ResetRedDefend);
		bChanged = true;
	}

	if(bChanged)
		CBots::BotFunction(PointsUpdated);

	CTeamFortress2Mod::m_ObjectiveResource.UpdateValidWaypointAreas();

}

const bool CTFObjectiveResource::IsWaypointAreaValid(int wptarea, int waypointflags) const
{
#if defined USE_NAVMESH
	return true;
#else
	if(wptarea == 0)
		return true;

	// Translate Waypoint Area to Index
	if((wptarea < 0) || (wptarea > MAX_CONTROL_POINTS))
		return false;

	int cpindex = m_WaypointAreaToIndexTranslation[wptarea];

	if(cpindex == -1)
		return false;

	if(waypointflags & CWaypointTypes::W_FL_AREAONLY)
	{
		// AND
		return (m_ValidPoints[0][0][cpindex].bValid && m_ValidPoints[1][1][cpindex].bValid) ||
			(m_ValidPoints[0][1][cpindex].bValid && m_ValidPoints[1][0][cpindex].bValid);
	}

	// OR
	return m_ValidAreas[cpindex];
#endif
}

const bool CTFObjectiveResource::IsCPValidWptArea(int iWptArea, int iTeam, ePointAttackDefend type) const
{
	if(iWptArea == 0)
		return true;

	if((iWptArea < 1) || (iWptArea > MAX_CONTROL_POINTS))
		return false;

	return IsCPValid(m_WaypointAreaToIndexTranslation[iWptArea], iTeam, type);
}

// Returns TRUE if waypoint area is worth attacking or defending at this moment
const bool CTFObjectiveResource::TestProbWptArea(int iWptArea, int iTeam) const
{
	int iCpIndex = m_WaypointAreaToIndexTranslation[iWptArea];

	if((iTeam != TF2_TEAM_BLUE) && (iTeam != TF2_TEAM_RED))
		return true;

	if(iWptArea == 0)
		return true;

	if((iWptArea < 1) || (iWptArea > MAX_CONTROL_POINTS))
		return true;

	return IsCPValid(iCpIndex, iTeam, TF2_POINT_ATTACK) ? (RandomFloat(0.0f, 1.0f) > m_ValidPoints[iTeam - 2][TF2_POINT_ATTACK][iCpIndex].fProb) : (IsCPValid(iCpIndex, iTeam, TF2_POINT_DEFEND) ? (RandomFloat(0.0f, 1.0f) > m_ValidPoints[iTeam - 2][TF2_POINT_DEFEND][iCpIndex].fProb) : true);
}

const bool CTFObjectiveResource::IsCPValid(int iCPIndex, int iTeam, ePointAttackDefend type) const
{
	if((iCPIndex < 0) || (iCPIndex >= MAX_CONTROL_POINTS))
		return false;

	return m_ValidPoints[iTeam - 2][type][iCPIndex].bValid;
}

int CTFObjectiveResource::GetControlPointArea(edict_t *pPoint) const
{
	for (int j = 0; j < m_iNumControlPoints; j++)
	{
		if (m_pControlPoints[j].Get() == pPoint)
			return (j + 1); // return waypoint area (+1)
	}

	return 0;
}

int CTFObjectiveResource::NearestArea(Vector vOrigin)
{
	int iNearest = -1;
	float fNearest = 2048.0f;
	float fDist;

	for (int i = 0; i < m_iNumControlPoints; i++)
	{
		if ((fDist = (GetCPPosition(i) - vOrigin).Length()) < fNearest)
		{
			fNearest = fDist;
			iNearest = i;
		}
	}

	if (iNearest == -1)
		return 0;

	// Add one for waypoint area
	return m_IndexToWaypointAreaTranslation[iNearest];
}

bool CTFObjectiveResource::UpdateDefendPoints(int team)
{
	/*int other = (team==2)?3:2;

	return GetCurrentAttackPoint(other);
	*/
	int signature = 0;
	int other;
	int prev;
	bool isPayLoadMap = CTeamFortress2Mod::IsMapType(TF_MAP_CART) || CTeamFortress2Mod::IsMapType(TF_MAP_CARTRACE);
	TF2PointProb_t *arr;

	//CTeamControlPoint *pPoint;
	//CTeamControlPointMaster *pMaster = CTeamFortress2Mod::getPointMaster();
	CTeamControlPointRound *pRound = CTeamFortress2Mod::GetCurrentRound();

	if (m_Resource.Get() == NULL) // not set up yet
		return false;
	if (team == 0) // invalid team
		return false;

	arr = m_ValidPoints[team - 2][TF2_POINT_DEFEND];

	// reset array
	Q_memset(arr, 0, sizeof(TF2PointProb_t)*MAX_CONTROL_POINTS);

	for (int i = 0; i < m_iNumControlPoints; i++)
	{
		arr[i].fProbMultiplier = 1.0f;
		arr[i].fProb = 1.0f;
		Q_memset(arr[i].iPrev, 0xFF, sizeof(int)*MAX_PREVIOUS_POINTS);

		// not visible
		if (!IsCPVisible(i))
			continue;
		// not unlocked
		if (GetCPUnlockTime(i) > TIME_NOW)
			continue;
		// not in round
		if (m_pControlPoints[i] && pRound && !pRound->IsPointInRound(m_pControlPoints[i]))
			continue;
		//int reqcappers = GetRequiredCappers(i,team);

		//if ( m_pControlPoints[i] )
		//	pPoint = CTeamControlPoint::getPoint(m_pControlPoints[i]);

		// We own this point
		if (GetOwningTeam(i) == team)
		{
			// The other team can capture
			other = (team == 2) ? 3 : 2;

			if (TeamCanCapPoint(i, other))
			{
				// if the other team has capture the previous points
				if ((prev = GetPreviousPointForPoint(i, other, 0)) != -1)
				{
					if (prev == i)
					{
						arr[i].bValid = true;
						continue;
					}
					else
					{
						// This point needs previous points to be captured first
						int j;
						bool bEnemyCanCap = true;

						for (j = 0; j < MAX_PREVIOUS_POINTS; j++)
						{
							// need to go through each previous point to update the array
							// DONT BREAK!!!
							prev = GetPreviousPointForPoint(i, other, j);
							arr[i].iPrev[j] = prev;

							if (prev == -1)
								continue;
							else if (GetOwningTeam(prev) != other)
								bEnemyCanCap = false;
						}

						if (!bEnemyCanCap)
						{
							arr[i].bPrev = true;
							arr[i].bValid = false;
							// Check later by checking prev points
						}
						else
						{
							arr[i].bValid = true;
						}
					}
				}
				else
				{
					if (CTeamFortress2Mod::IsAttackDefendMap())
						arr[i].bValid = true;
					else
					{
						int basepoint = GetBaseControlPointForTeam(team);
						arr[i].bValid = true;

						if (i == basepoint)
						{
							// check that all other points are owned
							int iNumOwned = 0;
							int iNumAvailable = 0;

							for (int j = 0; j < m_iNumControlPoints; j++)
							{
								// not visible
								if (!IsCPVisible(j))
									continue;
								// not unlocked
								if (GetCPUnlockTime(j) > TIME_NOW)
									continue;
								// not in round
								if (m_pControlPoints[j] && pRound && !pRound->IsPointInRound(m_pControlPoints[j]))
									continue;

								if (GetOwningTeam(j) == other)
									iNumOwned++;

								iNumAvailable++;
							}

							if (iNumOwned == (iNumAvailable - 1))
							{
								// other team can capture
								arr[i].fProb = 1.0f;
							}
							else if (iNumOwned == (iNumAvailable - 2))
							{
								// other team can capture this as the next point
								arr[i].fProb = bot_defrate.GetFloat();
							}
							else // still valid but very low probability of ever defending here
								arr[i].fProb = 0.001f;
						}
					}

					continue;
				}
			}
			else
			{
				//arr[i].bValid = true;
				//arr[i].fProb = 0.1f; // 10% of the time defend here
			}
		}
	}
	// do another search through the previous points
	for (int i = 0; i < m_iNumControlPoints; i++)
	{
		if (arr[i].bPrev)
		{
			int iNumPrevPointsAvail = 0;
			int j;

			// Check this points prevous points
			for (j = 0; j < MAX_PREVIOUS_POINTS; j++)
			{
				if (arr[i].iPrev[j] != -1)
				{
					// the previous point is not valid
					if (arr[arr[i].iPrev[j]].bValid)
						iNumPrevPointsAvail++;
				}
			}

			// only one more point to go until this point
			if (iNumPrevPointsAvail == 1)
			{
				// this point is next because the current valid points are required
				arr[i].bNextPoint = true;

				// other team can capture this as the next point
				// lower chance of defending the next point before round has started!!! Get everyone up!!
				arr[i].fProb = CTeamFortress2Mod::HasRoundStarted() ? bot_defrate.GetFloat() : (bot_defrate.GetFloat()*0.5f);
			}
		}
	}

	for (int i = 0; i < m_iNumControlPoints; i++)
	{
		if (arr[i].bNextPoint)
			arr[i].bValid = true;
		else if (arr[i].bValid)
		{
			bool bfound = false;

			// find this point in one of the previous points
			for (int j = 0; j < m_iNumControlPoints; j++)
			{
				if (i == j)
					continue;

				if (arr[j].bPrev)
				{
					for (int k = 0; k < MAX_PREVIOUS_POINTS; k++)
					{
						if (arr[j].iPrev[k] == i)
						{
							bfound = true;
							break;
						}
					}
				}

				if (bfound)
					break;

			}

			if (bfound)
			{
				arr[i].fProb = 1.0f;
			}
			else
			{
				arr[i].fProb = 0.1f;
			}
		}
	}

	// In Payload give lower numbers higher priority 
	if (isPayLoadMap)
	{
		float fMaxProb = 1.0f;
		bool bFirst = true;

		other = (team == 2) ? 3 : 2;

		for (int i = 0; i < m_iNumControlPoints; i++)
		{
			if (arr[i].bValid)
			{
				edict_t *pPayloadBomb = CTeamFortress2Mod::GetPayloadBomb(other);

				if (pPayloadBomb != NULL)
				{
					if (bFirst)
					{
						// TO DO update probability depending on distance to payload bomb
						float fDist = (CBotGlobals::EntityOrigin(pPayloadBomb) - GetCPPosition(i)).Length();

						bFirst = false;

						if (fDist > bot_tf2_payload_dist_retreat.GetFloat())
						{
							arr[i].fProb = 1.0f;

							if (!CTeamFortress2Mod::HasRoundStarted())
								fMaxProb = 0.1f;
							else
								fMaxProb = fMaxProb / 4;
						}
						else
						{
							arr[i].fProb = bot_defrate.GetFloat();

							int j = i + 1;

							if (j < m_iNumControlPoints)
							{
								if (arr[j].bValid == false)
								{
									if (!pRound || (m_pControlPoints[j] && pRound->IsPointInRound(m_pControlPoints[j])))
										arr[j].bValid = true; // this is the next point - move back lads
								}
							}
						}
					}
					else
					{
						arr[i].fProb = fMaxProb;
						fMaxProb = fMaxProb / 4;
					}
				}
				else
				{
					arr[i].fProb = fMaxProb;
					fMaxProb = fMaxProb / 4;
				}


				//arr[i].fProb *= arr[i].fProb; // square it
			}
		}
	}

	// update signature
	for (int i = 0; i < m_iNumControlPoints; i++)
	{
		byte j;
		byte *barr = (byte*)&(arr[i]);

		for (j = 0; j < sizeof(TF2PointProb_t); j++)
			signature = signature + ((barr[j] * (i + 1)) + j);
	}

	if (signature != m_PointSignature[team - 2][TF2_POINT_DEFEND])
	{
		m_PointSignature[team - 2][TF2_POINT_DEFEND] = signature;
		return true;
	}

	return false;
}

// return true if bots should change attack point
bool CTFObjectiveResource::UpdateAttackPoints(int team)
{
	int prev;
	int signature = 0;
	CTeamControlPointRound *pRound = CTeamFortress2Mod::GetCurrentRound();
	TF2PointProb_t *arr;

	if (!m_Resource.IsValid()) // not set up yet
		return false;
	if (team == 0)
		return false;

	arr = m_ValidPoints[team - 2][TF2_POINT_ATTACK];

	// reset array
	Q_memset(arr, 0, sizeof(TF2PointProb_t)*MAX_CONTROL_POINTS);
	Q_memset(arr->iPrev, 0xFF, sizeof(int)*MAX_PREVIOUS_POINTS);

	if ((team == TF2_TEAM_RED) && (CTeamFortress2Mod::IsAttackDefendMap()))
	{
		// no attacking for red on this map
		return false;
	}

	for (int i = 0; i < m_iNumControlPoints; i++)
	{
		arr[i].fProb = 1.0f;
		arr[i].fProbMultiplier = 1.0f;
		Q_memset(arr[i].iPrev, 0xFF, sizeof(int)*MAX_PREVIOUS_POINTS);

		// not visible
		if (!IsCPVisible(i))
			continue;
		// not unlocked
		if (GetCPUnlockTime(i) > TIME_NOW)
			continue;
		// not in round
		if (m_pControlPoints[i] && pRound && !pRound->IsPointInRound(m_pControlPoints[i]))
			continue;

		// We don't own this point
		if (GetOwningTeam(i) != team)
		{
			// we can capture
			if (TeamCanCapPoint(i, team))
			{
				// if we have captured the previous points we can capture
				if ((prev = GetPreviousPointForPoint(i, team, 0)) != -1)
				{
					if (prev == i)
					{
						/*int other = (team==2)?3:2;

						// find the base point
						int basepoint = GetBaseControlPointForTeam(other);

						if ( i == basepoint )
						{
						arr[i].fProb = 0.25f;
						}*/

						arr[i].bValid = true;
					}
					else
					{
						int j;

						bool bCanCap = true;

						for (j = 0; j < MAX_PREVIOUS_POINTS; j++)
						{
							// need to go through each previous point to update the array
							// DONT BREAK!!!
							prev = GetPreviousPointForPoint(i, team, j);
							arr[i].iPrev[j] = prev;

							if (prev == -1)
								continue;
							else if (GetOwningTeam(prev) != team)
								bCanCap = false;
						}

						if (!bCanCap)
						{
							arr[i].bPrev = true;
							arr[i].bValid = false;
							// Check later by checking prev points
						}
						else
						{
							arr[i].bValid = true;

							m_iMonitorPoint[team - 2] = i;
						}
					}
				}
				else
				{
					if (!CTeamFortress2Mod::IsAttackDefendMap())
					{
						// if its not an attack defend map check previous points are owned
						int other = (team == 2) ? 3 : 2;

						// find the base point
						int basepoint = GetBaseControlPointForTeam(other);

						/*if ( i == basepoint )
						{
						arr[i].bValid = true;
						arr[i].fProb = 0.25f;
						}
						else */

						if (basepoint == 0)
						{
							bool allowned = true;

							// make sure bot owns all points above this point
							for (int x = i + 1; x < m_iNumControlPoints; x++)
							{
								if (GetOwningTeam(x) != team)
								{
									allowned = false;
									break;
								}
							}

							if (allowned)
								arr[i].bValid = true;

							continue;
						}
						else if (basepoint == ((m_iNumControlPoints) - 1))
						{
							bool allowned = true;
							// make sure team owns all points below this point
							for (int x = 0; x < i; x++)
							{
								if (GetOwningTeam(x) != team)
								{
									allowned = false;
									break;
								}
							}

							if (allowned)
								arr[i].bValid = true;

							continue;
						}

					}

					arr[i].bValid = true;
				}
			}
		}
	}

	// Flush out less important cap points
	for (int i = 0; i < m_iNumControlPoints; i++)
	{
		if (arr[i].bValid)
		{
			bool bfound = false;

			// find this point in one of the previous points
			for (int j = 0; j < m_iNumControlPoints; j++)
			{
				if (i == j)
					continue;

				if (arr[j].bPrev)
				{
					for (int k = 0; k < MAX_PREVIOUS_POINTS; k++)
					{
						if (arr[j].iPrev[k] == i)
						{
							bfound = true;
							break;
						}
					}
				}

				if (bfound)
					break;

			}

			if (bfound)
			{
				arr[i].fProb = 1.0f;
			}
			else
			{
				arr[i].fProb = 0.1f;
			}
		}
	}

	// In Payload give lower numbers higher priority 
	if (CTeamFortress2Mod::IsMapType(TF_MAP_CART) || CTeamFortress2Mod::IsMapType(TF_MAP_CARTRACE))
	{
		for (int i = 0; i < m_iNumControlPoints; i++)
		{
			if (arr[i].bValid)
			{
				arr[i].fProb = (float)(m_iNumControlPoints + 1 - i);
				arr[i].fProb *= arr[i].fProb; // square it
			}
		}
	}

	for (int i = 0; i < m_iNumControlPoints; i++)
	{
		byte j;
		byte *barr = (byte*)&(arr[i]);

		for (j = 0; j < sizeof(TF2PointProb_t); j++)
			signature = signature + ((barr[j] * (i + 1)) + j);
	}

	if (signature != m_PointSignature[team - 2][TF2_POINT_ATTACK])
	{
		m_PointSignature[team - 2][TF2_POINT_ATTACK] = signature;
		return true;
	}

	return false;

}

void CTFObjectiveResource::UpdateCaptureTime(int index)
{
	m_fLastCaptureTime[index] = TIME_NOW;
}

float CTFObjectiveResource::GetLastCaptureTime(int index) const
{
	return m_fLastCaptureTime[index];
}