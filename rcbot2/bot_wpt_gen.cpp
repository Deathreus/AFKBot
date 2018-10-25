
#include "bot_base.h"
#include "bot_globals.h"
#include "bot_waypoint.h"
#include "bot_waypoint_visibility.h"
#include "bot_waypoint_locations.h"

#include "../NavMesh/NavMesh.h"
#include "../NavMesh/NavHintType.h"

CWaypoints::GenData_t CWaypoints::m_GenData ={ false, false, false, false, 0, 0 };


#ifdef GetClassName
 #undef GetClassName
#endif


extern INavMesh *g_pNavMesh;

void CWaypoints::PrepareGeneration()
{
	Q_memset(&m_GenData, 0, sizeof GenData_t);
	m_GenData.iMaxIndex = g_pNavMesh->GetAreas()->Count();
}

void CWaypoints::ProcessGeneration()
{
	static FrameTimer ftMessageTimer;

	if(m_GenData.bDoneGenerating)
	{
		m_bWantToGenerate = false;
		Msg(" *** done ***\n");
		CWaypoints::GetVisiblity()->SetWorkVisiblity(true);
		return;
	}

	if(m_GenData.bNeedToPostProcess)
		return;

	int iTicks = 0;
	std::vector<int> iNearby;

	IGamePlayer *pPlayer = playerhelpers->GetGamePlayer(1);

	// Stage 1, populate waypoints in the center of areas
	while(!m_GenData.bFirstStageDone && ++iTicks <= 16)
	{
		if(m_GenData.iCurrentIndex == m_GenData.iMaxIndex)
		{
			m_GenData.bFirstStageDone = true;
			m_GenData.iCurrentIndex = 0;
			break;
		}

		INavMeshArea *area = (*g_pNavMesh->GetAreas())[m_GenData.iCurrentIndex];
		if(area)
		{
			Vector vCenter = area->GetCenter();
			CWaypointLocations::GetAllInArea(vCenter, &iNearby, -1);
			bool bSomethingNear = false;
			for(int i : iNearby)
			{
				CWaypoint *pWpt = CWaypoints::GetWaypoint(i);
				if(pWpt && pWpt->DistanceFrom(vCenter) < 200.0f)
				{
					bSomethingNear = true;
					break;
				}
			}

			if(!bSomethingNear)
				CWaypoints::AddWaypoint(pPlayer->GetEdict(), vCenter, 0, true);
		}

		m_GenData.iCurrentIndex++;
		iNearby.clear();
	}

	// Stage 2, create additional waypoints in large areas, skipping spots near other points
	if(m_GenData.bFirstStageDone)
	{
		while(!m_GenData.bSecondStageDone && ++iTicks <= 16)
		{
			if(m_GenData.iCurrentIndex == m_GenData.iMaxIndex)
			{
				m_GenData.bSecondStageDone = true;
				m_GenData.bNeedToPostProcess = true;
				m_GenData.iCurrentIndex = 0;
				break;
			}

			INavMeshArea *area = (*g_pNavMesh->GetAreas())[m_GenData.iCurrentIndex];
			if(area)
			{
				Vector comp = area->GetExtentLow() - area->GetExtentHigh();
				if(comp.LengthSqr() > Square(400.0))
				{
					Vector vCenter = area->GetCenter();
					for(int8_t i = NAV_CORNER_NORTH_WEST; i < NAV_CORNER_COUNT; i++)
					{
						switch((eNavCorner)i)
						{
							case NAV_CORNER_NORTH_WEST:
								{
									vCenter.x += comp.Length() / 1.75;
									vCenter.y -= comp.Length() / 1.75;
									vCenter.z = area->GetZ(vCenter);
								}
								break;
							case NAV_CORNER_NORTH_EAST:
								{
									vCenter.x += comp.Length() / 1.75;
									vCenter.y += comp.Length() / 1.75;
									vCenter.z = area->GetZ(vCenter);
								}
								break;
							case NAV_CORNER_SOUTH_EAST:
								{
									vCenter.x -= comp.Length() / 1.75;
									vCenter.y += comp.Length() / 1.75;
									vCenter.z = area->GetZ(vCenter);
								}
								break;
							case NAV_CORNER_SOUTH_WEST:
								{
									vCenter.x -= comp.Length() / 1.75;
									vCenter.y -= comp.Length() / 1.75;
									vCenter.z = area->GetZ(vCenter);
								}
								break;
						}

						CWaypointLocations::GetAllInArea(vCenter, &iNearby, -1);
						bool bSomethingNear = false;
						for(int j : iNearby)
						{
							CWaypoint *pWpt = CWaypoints::GetWaypoint(i);
							if(pWpt && pWpt->DistanceFrom(vCenter) < 135.0f)
							{
								bSomethingNear = true;
								break;
							}
						}

						if(!bSomethingNear)
							CWaypoints::AddWaypoint(pPlayer->GetEdict(), vCenter, 0, true);
					}
				}
			}

			m_GenData.iCurrentIndex++;
			iNearby.clear();
		}
	}

	iNearby.clear();

	// Stage 3, populate health and ammo positions as well as capture points
	if(m_GenData.bNeedToPostProcess)
		PostProcessGeneration();

	if(ftMessageTimer.IsElapsed())
	{
		if(m_GenData.bNeedToPostProcess)
		{
			Msg(" *** fixing and tagging waypoints ***\n");
		}
		else
		{
			float denominator = (float)m_GenData.iMaxIndex;
			if(!m_GenData.bFirstStageDone) denominator *= 2.0;
			if(!m_GenData.bSecondStageDone) denominator *= 1.5;

			float pct = (float)m_GenData.iCurrentIndex / denominator * 100;
			Msg(" *** generating waypoints %.2f percent ***\n", pct);
		}

		ftMessageTimer.Start(1.0f);
	}
}

void CWaypoints::PostProcessGeneration()
{
	if(!m_GenData.bNeedToPostProcess)
		return;
	if(m_GenData.bDoneGenerating)
		return;

	IGamePlayer *pPlayer = playerhelpers->GetGamePlayer(1);

	for(short int i = MAX_PLAYERS; i < MAX_ENTITIES; i++)
	{
		edict_t *pEdict = INDEXENT(i);
		if(!pEdict || pEdict->IsFree())
			continue;

		if(Q_stristr(pEdict->GetClassName(), "health") != nullptr)
		{
			Vector vOrigin = CBotGlobals::EntityOrigin(pEdict);
			CWaypoints::AddWaypoint(pPlayer->GetEdict(), vOrigin, CWaypointTypes::W_FL_HEALTH, true);
		}

		if(Q_stristr(pEdict->GetClassName(), "ammo") != nullptr)
		{
			Vector vOrigin = CBotGlobals::EntityOrigin(pEdict);
			CWaypoints::AddWaypoint(pPlayer->GetEdict(), vOrigin, CWaypointTypes::W_FL_AMMO, true);
		}

		if(!Q_strcasecmp(pEdict->GetClassName(), "team_control_point"))
		{
			Vector vOrigin = CBotGlobals::EntityOrigin(pEdict);
			float flRadius = pEdict->GetCollideable()->OBBMaxs().AsVector2D().Length();
			CWaypoints::AddWaypoint(pPlayer->GetEdict(), vOrigin, CWaypointTypes::W_FL_CAPPOINT, true, 0, 0, flRadius);
		}

		if(!Q_strncasecmp(pEdict->GetClassName(), "func_breakable", 14))
		{
			Vector vOrigin = CBotGlobals::EntityOrigin(pEdict);
			CWaypoints::AddWaypoint(pPlayer->GetEdict(), vOrigin, CWaypointTypes::W_FL_BREAKABLE, true);
		}

		if(!Q_strcasecmp(pEdict->GetClassName(), "func_regenerate"))
		{
			Vector vOrigin = CBotGlobals::EntityOrigin(pEdict);
			CWaypoints::AddWaypoint(pPlayer->GetEdict(), vOrigin, CWaypointTypes::W_FL_RESUPPLY, true);
		}

		// For MvM, there's a capturezone but not a point, but outside of that mode we don't want to place 2 waypoints if a capturezone exists on a team_control_point
		if(!Q_strcasecmp(pEdict->GetClassName(), "func_capturezone"))
		{
			std::vector<int> iNearby;
			Vector vOrigin = CBotGlobals::EntityOrigin(pEdict);
			float flRadius = pEdict->GetCollideable()->OBBMaxs().AsVector2D().Length();
			CWaypointLocations::GetAllInArea(vOrigin, &iNearby, -1);
			bool bSomethingNear = false;
			for(int j : iNearby)
			{
				CWaypoint *pWpt = CWaypoints::GetWaypoint(j);
				if(pWpt && pWpt->DistanceFrom(vOrigin) < 32.0f)
				{
					bSomethingNear = true;
					break;
				}
			}

			if(!bSomethingNear)
				CWaypoints::AddWaypoint(pPlayer->GetEdict(), vOrigin, CWaypointTypes::W_FL_CAPPOINT, true, 0, 0, flRadius);
		}
	}

	for(INavMeshLadder *ladder : *g_pNavMesh->GetLadders())
	{
		Vector vBottom, vTop;

		vBottom.Init(ladder->GetBottomX(), ladder->GetBottomY(), ladder->GetBottomZ());
		vTop.Init(ladder->GetTopX(), ladder->GetTopY(), ladder->GetTopZ());

		CWaypoints::AddWaypoint(pPlayer->GetEdict(), vBottom, CWaypointTypes::W_FL_LADDER, true);
		CWaypoints::AddWaypoint(pPlayer->GetEdict(), vTop, CWaypointTypes::W_FL_LADDER, true);
	}

	for(INavMeshHint *hint : *g_pNavMesh->GetHints())
	{
		Vector vSpot = hint->GetPos();
		float flYaw = hint->GetYaw();
		byte uType = hint->GetFlags();

		if(uType & NAV_HINT_SENTRY)
			CWaypoints::AddWaypoint(pPlayer->GetEdict(), vSpot, CWaypointTypes::W_FL_SENTRY, true, Ceil2Int(flYaw), 0, 32);
		if(uType & NAV_HINT_SNIPE)
			CWaypoints::AddWaypoint(pPlayer->GetEdict(), vSpot, CWaypointTypes::W_FL_SNIPER, true, Ceil2Int(flYaw), 0, 32);
		if(uType & NAV_HINT_TELEPORTER)
			CWaypoints::AddWaypoint(pPlayer->GetEdict(), vSpot, CWaypointTypes::W_FL_TELE_EXIT, true, Ceil2Int(flYaw), 0, 32);
		if(uType & NAV_HINT_RALLY)
			CWaypoints::AddWaypoint(pPlayer->GetEdict(), vSpot, CWaypointTypes::W_FL_DEFEND, true, Ceil2Int(flYaw), 0, 32);
		if(uType & NAV_HINT_BOMB)
			CWaypoints::AddWaypoint(pPlayer->GetEdict(), vSpot, CWaypointTypes::W_FL_BOMBS_HERE, true, Ceil2Int(flYaw), 0, 32);
	}

	m_GenData.bDoneGenerating = true;
}