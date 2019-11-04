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

#include "bot_base.h"

#include "in_buttons.h"

#include "bot_mods.h"
#include "bot_globals.h"
#include "bot_fortress.h"
#include "bot_weapons.h"
#include "bot_configfile.h"
#include "bot_getprop.h"
#include "bot_navigator.h"
#include "bot_tf2_points.h"
#include "bot_gamerules.h"

#if defined USE_NAVMESH
#include "bot_navmesh.h"
#else
#include "bot_waypoint.h"
#include "bot_waypoint_locations.h"
#endif

#include <vector>
#include <stdexcept>

#ifdef GetClassName
 #undef GetClassName
#endif

eTFMapType CTeamFortress2Mod::m_MapType = TF_MAP_DM;
tf_tele_t CTeamFortress2Mod::m_Teleporters[MAX_PLAYERS];
int CTeamFortress2Mod::m_iArea = 0;
float CTeamFortress2Mod::m_fSetupTime = 0.0f;
float CTeamFortress2Mod::m_fRoundTime = 0.0f;
MyEHandle CTeamFortress2Mod::m_pFlagCarrierRed;
MyEHandle CTeamFortress2Mod::m_pFlagCarrierBlue;
float CTeamFortress2Mod::m_fArenaPointOpenTime = 0.0f;
float CTeamFortress2Mod::m_fPointTime = 0.0f;
tf_sentry_t CTeamFortress2Mod::m_SentryGuns[MAX_PLAYERS];	// used to let bots know if sentries have been sapped or not
tf_disp_t  CTeamFortress2Mod::m_Dispensers[MAX_PLAYERS];	// used to let bots know where friendly/enemy dispensers are
MyEHandle CTeamFortress2Mod::m_PlayerResource;
bool CTeamFortress2Mod::m_bAttackDefendMap = false;
int CTeamFortress2Mod::m_Cappers[MAX_CONTROL_POINTS];
int CTeamFortress2Mod::m_iCapDefenders[MAX_CONTROL_POINTS];
bool CTeamFortress2Mod::m_bHasRoundStarted = true;
bool CTeamFortress2Mod::m_bDontClearPoints = false;
int CTeamFortress2Mod::m_iFlagCarrierTeam = 0;
MyEHandle CTeamFortress2Mod::m_pBoss;
bool CTeamFortress2Mod::m_bBossSummoned = false;
MyEHandle CTeamFortress2Mod::m_pMediGuns[MAX_PLAYERS];
CTFObjectiveResource CTeamFortress2Mod::m_ObjectiveResource;
CTeamControlPointMaster *CTeamFortress2Mod::m_pPointMaster = NULL;
CTeamRoundTimer CTeamFortress2Mod::m_Timer;
MyEHandle CTeamFortress2Mod::m_PointMasterResource;
CTeamControlPointRound *CTeamFortress2Mod::m_pCurrentRound = NULL;
bool CTeamFortress2Mod::m_bFlagStateDefault = true;
MyEHandle CTeamFortress2Mod::m_pPayLoadBombBlue;
MyEHandle CTeamFortress2Mod::m_pPayLoadBombRed;
bool CTeamFortress2Mod::m_bRoundOver = false;
int CTeamFortress2Mod::m_iWinningTeam = 0;
Vector CTeamFortress2Mod::m_vFlagLocationBlue = Vector(0.0f);
Vector CTeamFortress2Mod::m_vFlagLocationRed = Vector(0.0f);
bool CTeamFortress2Mod::m_bFlagLocationValidBlue = false;
bool CTeamFortress2Mod::m_bFlagLocationValidRed = false;
bool CTeamFortress2Mod::m_bMVMFlagStartValid = false;
Vector CTeamFortress2Mod::m_vMVMFlagStart = Vector(0.0f);
bool CTeamFortress2Mod::m_bMVMCapturePointValid = false;
Vector CTeamFortress2Mod::m_vMVMCapturePoint = Vector(0.0f);
bool CTeamFortress2Mod::m_bMVMAlarmSounded = false;
float CTeamFortress2Mod::m_fMVMCapturePointRadius = 0.0f;
int CTeamFortress2Mod::m_iCapturePointWptID = -1;
int CTeamFortress2Mod::m_iFlagPointWptID = -1;
CMannVsMachineUpgradeManager *CTeamFortress2Mod::m_UpgradeManager = NULL;

extern ConVar bot_use_disp_dist;
extern ConVar bot_const_point_master_offset;
extern ConVar bot_tf2_autoupdate_point_time;

extern ConVar *mp_stalemate_enable;

extern IServerGameEnts *gameents;
extern IServerTools *servertools;


bool CTeamFortress2Mod::IsSuddenDeath()
{
	if (!mp_stalemate_enable || !mp_stalemate_enable->GetBool() || IsMapType(TF_MAP_ARENA))
		return false;

	if (!CGameRulesObject::GetGameRules())
		return false;

	return CGameRulesObject::GetProperty("m_iRoundState") == RoundState_Stalemate;
}

bool CTeamFortress2Mod::IsMedievalMode()
{
	if (!CGameRulesObject::GetGameRules())
		return false;

	return CGameRulesObject::GetProperty("m_bPlayingMedieval") == 1;
}

#if defined USE_NAVMESH
bool CTeamFortress2Mod::CheckWaypointForTeam(INavMeshArea *pWpt, int iTeam)
{
	unsigned int TFFlags = pWpt->GetTFAttribs();
	if(TFFlags & (RED_SPAWN_ROOM|RED_SETUP_GATE|RED_ONE_WAY_DOOR))
		return m_bRoundOver || iTeam == TF2_TEAM_RED;
	if(TFFlags & (BLUE_SPAWN_ROOM|BLUE_SETUP_GATE|BLUE_ONE_WAY_DOOR))
		return m_bRoundOver || iTeam == TF2_TEAM_BLUE;

	return true;
}
#else
bool CTeamFortress2Mod::CheckWaypointForTeam(CWaypoint *pWpt, int iTeam)
{
	return m_bRoundOver || ((!pWpt->HasFlag(CWaypointTypes::W_FL_NOBLU) || (iTeam != TF2_TEAM_BLUE)) && (!pWpt->HasFlag(CWaypointTypes::W_FL_NORED) || (iTeam != TF2_TEAM_RED)));
}
#endif

bool CTeamFortress2Mod::IsWaypointAreaValid(int iWptArea, int iWptFlags)
{
	return CTeamFortress2Mod::m_ObjectiveResource.IsWaypointAreaValid(iWptArea, iWptFlags);
}

bool CTeamFortress2Mod::WithinEndOfRound(float fTime)
{
	return (TIME_NOW > (m_Timer.GetEndTime() - fTime));
}

#if !defined USE_NAVMESH
void CTeamFortress2Mod::GetTeamOnlyWaypointFlags(int iTeam, int *iOn, int *iOff)
{
	if (iTeam == TF2_TEAM_BLUE)
	{
		*iOn = CWaypointTypes::W_FL_NORED;
		*iOff = CWaypointTypes::W_FL_NOBLU;
	}
	else if (iTeam == TF2_TEAM_RED)
	{
		*iOn = CWaypointTypes::W_FL_NOBLU;
		*iOff = CWaypointTypes::W_FL_NORED;
	}

}
#endif

void CTeamFortress2Mod::ModFrame()
{
	if (!m_ObjectiveResource.m_Resource)
	{
		m_ObjectiveResource.m_Resource = servertools->FindEntityByClassname(NULL, "tf_objective_resource");

		if (m_ObjectiveResource.m_Resource.IsValid())
			m_ObjectiveResource.Setup();
	}
	else CTeamFortress2Mod::m_ObjectiveResource.Think();
}

void CTeamFortress2Mod::InitMod()
{
	CWeapons::LoadWeapons((m_szWeaponListName == NULL) ? "TF2" : m_szWeaponListName, TF2Weaps);
	CRCBotTF2UtilFile::LoadConfig();
}

// https://forums.alliedmods.net/showthread.php?t=226621
eTFMapType CTeamFortress2Mod::GetMapType()
{
	if (CGameRulesObject::GetProperty("m_bIsInTraining") == 1)
		return TF_MAP_TR;

	if (CGameRulesObject::GetProperty("m_bPlayingSpecialDeliveryMode") == 1)
		return TF_MAP_SD;
	
	if (CGameRulesObject::GetProperty("m_bPlayingMannVsMachine") == 1)
		return TF_MAP_MVM;

	if (CGameRulesObject::GetProperty("m_bPlayingKoth") == 1)
		return TF_MAP_KOTH;

	if (CGameRulesObject::GetProperty("m_bPlayingRobotDestructionMode") == 1)
		return TF_MAP_RD;

	// Cases not caught by the above
	int iGameType = CGameRulesObject::GetProperty("m_nGameType");
	switch (iGameType)
	{
		case 1:
			return TF_MAP_CTF;
		case 2:
		{
			int iRoundCount = 0;
			CBaseEntity *pRoundCP = NULL;
			int iPriority = -1;

			int iRestrictWinner = -1;
			int iRestrictCount = 0;

			while ((pRoundCP = servertools->FindEntityByClassname(pRoundCP, "team_control_point_round")) != NULL)
			{
				iRoundCount++;

				edict_t *pEdict = gameents->BaseEntityToEdict(pRoundCP);
				iRestrictWinner = GetEntData<int>(pEdict, "m_iInvalidCapWinner");
				if (iRestrictWinner > 1)
					iRestrictCount++;

				int iNewPriority = GetEntData<int>(pEdict, "m_nPriority");
				if (iNewPriority > iPriority)
					iPriority = iNewPriority;
				else if (iNewPriority == iPriority)
					return TF_MAP_TC;
			}

			if (iRoundCount > 1 && iRoundCount == iRestrictCount)
				return TF_MAP_CP;
			else if (iRoundCount > 1)
				return TF_MAP_TC;

			CBaseEntity *pMaster = servertools->FindEntityByClassname(NULL, "team_control_point_master");
			if(pMaster)
			{
				edict_t *pEdict = gameents->BaseEntityToEdict(pMaster);
				iRestrictWinner = GetEntData<int>(pEdict, "m_iInvalidCapWinner");
				if(iRestrictWinner > 1)
					return TF_MAP_CP;
			}

			return TF_MAP_5CP;
		}
		case 3:
		{
			if (servertools->FindEntityByClassname(NULL, "tf_logic_multiple_escort") != NULL)
				return TF_MAP_CARTRACE;

			return TF_MAP_CART;
		}
		case 4:
			return TF_MAP_ARENA;
		default:
			break;
	}

	return TF_MAP_DM;
}

void CTeamFortress2Mod::MapInit()
{
	CBotMod::MapInit();

	m_PlayerResource = MyEHandle();
	m_ObjectiveResource.Reset();
	m_pPointMaster = NULL;
	m_PointMasterResource = MyEHandle();
	m_pCurrentRound = NULL;
	m_Timer.Reset();
	m_bFlagStateDefault = true;
	m_bFlagLocationValidBlue = false;
	m_bFlagLocationValidRed = false;
	m_bMVMAlarmSounded = false;
	m_bMVMFlagStartValid = false;
	m_bMVMCapturePointValid = false;
	m_fMVMCapturePointRadius = 0.0f;
	m_iCapturePointWptID = -1;
	m_iFlagPointWptID = -1;

	m_MapType = GetMapType();

#if defined _DEBUG
	const char *szMapname = gpGlobals->mapname.ToCStr();
	smutils->LogMessage(myself, "GetMapType() returned %i on map '%s'", m_MapType, szMapname);
#endif

	m_iArea = 0;

	m_fSetupTime = 5.0f; // 5 seconds normal

	m_fRoundTime = 0.0f;

	m_pFlagCarrierRed = MyEHandle();
	m_pFlagCarrierBlue = MyEHandle();
	m_iFlagCarrierTeam = 0;
	m_bDontClearPoints = false;

	for (short int i = 0; i < MAX_PLAYERS; i++)
	{
		m_Teleporters[i].m_iWaypoint = -1;
		m_Teleporters[i].m_fLastTeleported = 0.0f;
		m_Teleporters[i].entrance = MyEHandle();
		m_Teleporters[i].exit = MyEHandle();
		m_Teleporters[i].sapper = MyEHandle();
		m_SentryGuns[i].sapper = MyEHandle();
		m_SentryGuns[i].sentry = MyEHandle();
		m_Dispensers[i].sapper = MyEHandle();
		m_Dispensers[i].disp = MyEHandle();
		m_pMediGuns[i] = MyEHandle();
	}

	m_bAttackDefendMap = false;
	m_pBoss = MyEHandle();
	m_bBossSummoned = false;

	ResetCappers();
	ResetDefenders();

	if(IsMapType(TF_MAP_MVM))
	{
		void *pAddr;
	#if defined _WINDOWS
		if(!g_pGameConf->GetAddress("g_MannVsMachineUpgrades", &pAddr))
		{
			smutils->LogError(myself, "Couldn't get address of g_MannVsMachineUpgrades");
			return;
		}
	#else
		if(!g_pGameConf->GetMemSig("g_MannVsMachineUpgrades", &pAddr))
		{
			smutils->LogError(myself, "Couldn't get address of g_MannVsMachineUpgrades");
			return;
		}
	#endif

		m_UpgradeManager = reinterpret_cast<CMannVsMachineUpgradeManager *>(pAddr);
		ErrorIfNot(m_UpgradeManager, ("m_UpgradeManager(%p)", m_UpgradeManager));

		for(int i = 0; i < MAX_UPGRADES; i++)
		{
			CMannVsMachineUpgrades upgrade = m_UpgradeManager->m_Upgrades[i];

			DebugMsg("MannVsMachineUpgrade %i", i);
			DebugMsg("\t%s", upgrade.m_szAttribute);
			DebugMsg("\t%s", upgrade.m_szIcon);
			DebugMsg("\t%.3f", upgrade.m_flIncrement);
			DebugMsg("\t%.3f", upgrade.m_flCap);
			DebugMsg("\t%i", upgrade.m_nCost);
			DebugMsg("\t%i", upgrade.m_iUIGroup);
			DebugMsg("\t%i", upgrade.m_iQuality);
			DebugMsg("\t%i", upgrade.m_iTier);
		}
	}
}

int CTeamFortress2Mod::GetTeleporterWaypoint(edict_t *pTele)
{
	for (unsigned short int i = 0; i < MAX_PLAYERS; i++)
	{
		if (m_Teleporters[i].exit == pTele)
			return m_Teleporters[i].m_iWaypoint;
	}

	return -1;
}

bool CTeamFortress2Mod::IsPlayerZoomed(edict_t *pPlayer)
{
	return IsPlayerInCondition(pPlayer, TFCond_Zoomed);
}

bool CTeamFortress2Mod::IsPlayerSlowed(edict_t *pPlayer)
{
	return IsPlayerInCondition(pPlayer, TFCond_Slowed);
}

bool CTeamFortress2Mod::IsPlayerDisguised(edict_t *pPlayer)
{
	if (IsPlayerInCondition(pPlayer, TFCond_OnFire) || IsPlayerInCondition(pPlayer, TFCond_Bleeding)
		|| IsPlayerInCondition(pPlayer, TFCond_Milked) || IsPlayerInCondition(pPlayer, TFCond_Jarated))
		return false;

	return (IsPlayerInCondition(pPlayer, TFCond_Disguised) && !IsPlayerInCondition(pPlayer, TFCond_Disguising));
}

bool CTeamFortress2Mod::IsPlayerTaunting(edict_t *pPlayer)
{
	return IsPlayerInCondition(pPlayer, TFCond_Taunting);
}

bool CTeamFortress2Mod::IsPlayerCloaked(edict_t *pPlayer)
{
	if (IsPlayerInCondition(pPlayer, TFCond_OnFire) || IsPlayerInCondition(pPlayer, TFCond_CloakFlicker)
		|| IsPlayerInCondition(pPlayer, TFCond_Bleeding) || IsPlayerInCondition(pPlayer, TFCond_Milked) || IsPlayerInCondition(pPlayer, TFCond_Jarated))
		return false;

	return (IsPlayerInCondition(pPlayer, TFCond_Cloaked) || IsPlayerInCondition(pPlayer, TFCond_DeadRingered)
		|| IsPlayerInCondition(pPlayer, TFCond_Stealthed) || IsPlayerInCondition(pPlayer, TFCond_StealthedUserBuffFade));
}

bool CTeamFortress2Mod::IsPlayerKrits(edict_t *pPlayer)
{
	return (IsPlayerInCondition(pPlayer, TFCond_Kritzkrieged) || IsPlayerInCondition(pPlayer, TFCond_CritCanteen)
		|| IsPlayerInCondition(pPlayer, TFCond_HalloweenCritCandy) || IsPlayerInCondition(pPlayer, TFCond_CritOnKill)
		|| IsPlayerInCondition(pPlayer, TFCond_CritOnFirstBlood) || IsPlayerInCondition(pPlayer, TFCond_CritMmmph));
}

bool CTeamFortress2Mod::IsPlayerInvuln(edict_t *pPlayer)
{
	return (IsPlayerInCondition(pPlayer, TFCond_Ubercharged) || IsPlayerInCondition(pPlayer, TFCond_UberchargedCanteen)
		|| IsPlayerInCondition(pPlayer, TFCond_UberchargedHidden) || IsPlayerInCondition(pPlayer, TFCond_UberchargedOnTakeDamage));
}

bool CTeamFortress2Mod::IsPlayerOnFire(edict_t *pPlayer)
{
	return IsPlayerInCondition(pPlayer, TFCond_OnFire);
}
// SourceMod
bool CTeamFortress2Mod::IsPlayerInCondition(edict_t *pPlayer, TFCond iCond)
{
	if(!pPlayer)
		return false;

	switch(iCond / 32)
	{
		case 0:
			{
				int bit = 1 << iCond;
				if((GetEntSend<int>(pPlayer, "_condition_bits") & bit) == bit)
				{
					return true;
				}

				if((GetEntSend<int>(pPlayer, "m_nPlayerCond") & bit) == bit)
				{
					return true;
				}
			}
			break;
		case 1:
			{
				int bit = (1 << (iCond - 32));
				if((GetEntSend<int>(pPlayer, "m_nPlayerCondEx") & bit) == bit)
				{
					return true;
				}
			}
			break;
		case 2:
			{
				int bit = (1 << (iCond - 64));
				if((GetEntSend<int>(pPlayer, "m_nPlayerCondEx2") & bit) == bit)
				{
					return true;
				}
			}
			break;
		case 3:
			{
				int bit = (1 << (iCond - 96));
				if((GetEntSend<int>(pPlayer, "m_nPlayerCondEx3") & bit) == bit)
				{
					return true;
				}
			}
			break;
		case 4:
			{
				int bit = (1 << (iCond - 128));
				if((GetEntSend<int>(pPlayer, "m_nPlayerCondEx4") & bit) == bit)
				{
					return true;
				}
			}
			break;
		default:
			smutils->LogError(myself, "Warning: TFCond %i outside of possible condition range", iCond);
			break;
	}

	return false;
}

int CTeamFortress2Mod::NumPlayersOnTeam(int iTeam, bool bAliveOnly)
{
	int8_t num = 0;
	IGamePlayer *pPlayer;
	IPlayerInfo *pPI;

	for (short int i = 1; i < MAX_PLAYERS; i++)
	{
		pPlayer = playerhelpers->GetGamePlayer(i);
		if (!pPlayer || !pPlayer->IsInGame() || pPlayer->IsReplay() || pPlayer->IsSourceTV())
			continue;

		pPI = pPlayer->GetPlayerInfo();
		if (!pPI || pPI->IsObserver())
			continue;

		if (GetTeam(pPlayer->GetEdict()) == iTeam && !(bAliveOnly && pPI->IsDead()))
			num++;
	}

	return num;
}

int CTeamFortress2Mod::NumClassOnTeam(int iTeam, TFClass iClass)
{
	int8_t num = 0;
	IGamePlayer *pPlayer;
	IPlayerInfo *pPI;

	for (short int i = 1; i < MAX_PLAYERS; i++)
	{
		pPlayer = playerhelpers->GetGamePlayer(i);
		if (!pPlayer || !pPlayer->IsInGame() || pPlayer->IsReplay() || pPlayer->IsSourceTV())
			continue;

		pPI = pPlayer->GetPlayerInfo();
		if (!pPI || pPI->IsObserver())
			continue;

		if (GetTeam(pPlayer->GetEdict()) == iTeam && GetClass(pPlayer->GetEdict()) == iClass)
			num++;
	}

	return num;
}

edict_t *CTeamFortress2Mod::FindResourceEntity()
{
	if (!m_PlayerResource)
		m_PlayerResource = servertools->FindEntityByClassname(NULL, "tf_player_manager");

	return m_PlayerResource;
}

TFClass CTeamFortress2Mod::GetSpyDisguise(edict_t *pPlayer)
{
	static int iClass;

	CClassInterface::GetTF2SpyDisguised(pPlayer, &iClass, NULL, NULL, NULL);

	return (TFClass)iClass;
}

float CTeamFortress2Mod::GetClassSpeed(TFClass TFClass)
{
	switch (TFClass)
	{
		case TF_CLASS_SCOUT:
			return 400.0f;
		case TF_CLASS_SOLDIER:
			return 240.0f;
		case TF_CLASS_DEMOMAN:
			return 280.0f;
		case TF_CLASS_MEDIC:
			return 320.0f;
		case TF_CLASS_PYRO:
		case TF_CLASS_SPY:
		case TF_CLASS_ENGINEER:
		case TF_CLASS_SNIPER:
			return 300.0f;
		case TF_CLASS_HWGUY:
			return 200.0f;
		default:
			return 0.0f;
	}
}
// SourceMod
float CTeamFortress2Mod::GetPlayerSpeed(edict_t *pPlayer, TFClass iClass)
{
	IGamePlayer *pPL = playerhelpers->GetGamePlayer(pPlayer);
	if (!pPL || !pPL->IsInGame())
		return 0.0f;

	IPlayerInfo *pPI = pPL->GetPlayerInfo();
	if (!pPI || pPI->IsDead())
		return 0.0f;

	float fSpeed;

	switch (iClass)
	{
		case TF_CLASS_SNIPER:
			{
				fSpeed = IsPlayerInCondition(pPlayer, TFCond_Slowed) ? 80.0f : 300.0f;
			}
			break;
		case TF_CLASS_SOLDIER:
			{
				edict_t *pWeapon = GetEntSendEnt(pPlayer, "m_hActiveWeapon");
				if (pWeapon && !pWeapon->IsFree())
				{
					if (GetEntSend<int>(pWeapon, "m_iItemDefinitionIndex") == 128) // Escape Plan
					{
						int iHealth = pPI->GetHealth();
						if (iHealth > 160)
							fSpeed = 240.0f;
						else if (iHealth > 120)
							fSpeed = 264.0f;
						else if (iHealth > 80)
							fSpeed = 288.0f;
						else if (iHealth > 40)
							fSpeed = 336.0f;
						else
							fSpeed = 384.0f;
					}
					else fSpeed = 240.0f;
				}
				else fSpeed = GetClassSpeed(iClass);
			}
			break;
		case TF_CLASS_DEMOMAN:
			{
				edict_t *pWeapon = GetEntSendEnt(pPlayer, "m_hActiveWeapon");
				if (pWeapon && !pWeapon->IsFree())
				{
					if (GetEntSend<int>(pWeapon, "m_iItemDefinitionIndex") == 128)	// Skullcutter
						fSpeed = IsPlayerInCondition(pPlayer, TFCond_Charging) ? 638.0f : 238.0f;
					else
						fSpeed = IsPlayerInCondition(pPlayer, TFCond_Charging) ? 750.0f : 280.0f;
				}
				else fSpeed = GetClassSpeed(iClass);
			}
			break;
		case TF_CLASS_HWGUY:
			{
				if (IsPlayerInCondition(pPlayer, TFCond_Slowed))
					fSpeed = 110.0f;
				else if (IsPlayerInCondition(pPlayer, TFCond_CritCola))
					fSpeed = 310.0f;
				else
				{
					edict_t *pWeapon = GetEntSendEnt(pPlayer, "m_hActiveWeapon");
					if (pWeapon && !pWeapon->IsFree())
					{
						int iIndex = GetEntSend<int>(pWeapon, "m_iItemDefinitionIndex");
						if (iIndex == 239 || iIndex == 1084 || iIndex == 1100)	//GRU's
							fSpeed = 300.0f;
						else
							fSpeed = 230.0f;
					}
					else fSpeed = GetClassSpeed(iClass);
				}
			}
			break;
		case TF_CLASS_ENGINEER:
			{
				fSpeed = (GetEntSendEnt(pPlayer, "m_hCarriedObject") != NULL) ? 225.0f : 300.0f;
			}
			break;
		default:
			fSpeed = GetClassSpeed(iClass);
			break;
	}

	if (IsPlayerInCondition(pPlayer, TFCond_Dazed))
		fSpeed *= 0.5;

	if (IsPlayerInCondition(pPlayer, TFCond_SpeedBuffAlly))
		fSpeed *= 1.4;

	return fSpeed;
}

void CTeamFortress2Mod::UpdateTeleportTime(edict_t *pOwner)
{
	m_Teleporters[SlotOfEdict(pOwner)].m_fLastTeleported = TIME_NOW;
}

float CTeamFortress2Mod::GetTeleportTime(edict_t *pOwner)
{
	return m_Teleporters[SlotOfEdict(pOwner)].m_fLastTeleported;
}

TFTeam CTeamFortress2Mod::GetTeam(edict_t *pEntity)
{
	return (TFTeam)CClassInterface::GetTeam(pEntity);
}

TFClass CTeamFortress2Mod::GetClass(edict_t *pPlayer)
{
	return (TFClass)CClassInterface::GetTF2Class(pPlayer);
}

int CTeamFortress2Mod::GetSentryLevel(edict_t *pSentry)
{
	return CClassInterface::GetObjectUpgradeLevel(pSentry);
}

int CTeamFortress2Mod::GetDispenserLevel(edict_t *pDispenser)
{
	return CClassInterface::GetObjectUpgradeLevel(pDispenser);
}

int CTeamFortress2Mod::GetEnemyTeam(int iTeam)
{
	return (iTeam == TF2_TEAM_BLUE) ? TF2_TEAM_RED : TF2_TEAM_BLUE;
}

bool CTeamFortress2Mod::IsFlag(edict_t *pEntity, int iTeam)
{
	return (!iTeam || (GetEnemyTeam(iTeam) == GetTeam(pEntity))) && (strcmp(pEntity->GetClassName(), "item_teamflag") == 0);
}

bool CTeamFortress2Mod::IsBoss(edict_t *pEntity, float *fFactor)
{
	if (m_bBossSummoned)
	{
		if (m_pBoss.IsValid() && CBotGlobals::EntityIsAlive(m_pBoss))
			return m_pBoss == pEntity;
		else if ((strcmp(pEntity->GetClassName(), "merasmus") == 0) ||
			(strcmp(pEntity->GetClassName(), "headless_hatman") == 0) ||
			(strcmp(pEntity->GetClassName(), "eyeball_boss") == 0) ||
			(strcmp(pEntity->GetClassName(), "tf_zombie") == 0))
		{
			m_pBoss = pEntity;
			return true;
		}
	}
	else if (CTeamFortress2Mod::IsMapType(TF_MAP_CARTRACE))
	{
		if (m_pBoss == pEntity)
			return true;
		else if (strcmp(pEntity->GetClassName(), "tf_zombie") == 0)
		{
			m_pBoss = pEntity;
			return true;
		}
	}
	else if (CTeamFortress2Mod::IsMapType(TF_MAP_MVM))
	{
		if (m_pBoss == pEntity)
			return true;
		else if (strcmp(pEntity->GetClassName(), "tank_boss") == 0)
		{
			if (fFactor != NULL)
				*fFactor = 200.0f;

			m_pBoss = pEntity;
			return true;
		}
	}

	return false;
}

bool CTeamFortress2Mod::IsSentry(edict_t *pEntity, int iTeam, bool checkcarrying)
{
	return (!iTeam || (iTeam == GetTeam(pEntity))) && (strcmp(pEntity->GetClassName(), "obj_sentrygun") == 0) && !(checkcarrying && CClassInterface::IsObjectBeingPlaced(pEntity));
}

bool CTeamFortress2Mod::IsDispenser(edict_t *pEntity, int iTeam, bool checkcarrying)
{
	return (!iTeam || (iTeam == GetTeam(pEntity))) && (strcmp(pEntity->GetClassName(), "obj_dispenser") == 0) && !(checkcarrying && CClassInterface::IsObjectBeingPlaced(pEntity));
}

bool CTeamFortress2Mod::IsTeleporter(edict_t *pEntity, int iTeam, bool checkcarrying)
{
	return (!iTeam || (iTeam == GetTeam(pEntity))) && (strcmp(pEntity->GetClassName(), "obj_teleporter") == 0) && !(checkcarrying && CClassInterface::IsObjectBeingPlaced(pEntity));
}

bool CTeamFortress2Mod::IsTeleporterEntrance(edict_t *pEntity, int iTeam, bool checkcarrying)
{
	return IsTeleporter(pEntity, iTeam) && CClassInterface::IsTeleporterMode(pEntity, TELE_ENTRANCE) && !(checkcarrying && CClassInterface::IsObjectBeingPlaced(pEntity));
}

bool CTeamFortress2Mod::IsTeleporterExit(edict_t *pEntity, int iTeam, bool checkcarrying)
{
	return IsTeleporter(pEntity, iTeam) && CClassInterface::IsTeleporterMode(pEntity, TELE_EXIT) && !(checkcarrying && CClassInterface::IsObjectBeingPlaced(pEntity));
}

bool CTeamFortress2Mod::IsPipeBomb(edict_t *pEntity, int iTeam)
{
	return (!iTeam || (iTeam == GetTeam(pEntity))) && (strcmp(pEntity->GetClassName(), "tf_projectile_pipe_remote") == 0);
}

bool CTeamFortress2Mod::IsHurtfulPipeGrenade(edict_t *pEntity, edict_t *pPlayer, bool bCheckOwner)
{
	if (strcmp(pEntity->GetClassName(), "tf_projectile_pipe") == 0)
	{
		if (bCheckOwner && (CClassInterface::GetPipeBombOwner(pEntity) == pPlayer))
			return true;

		int iPlayerTeam = GetTeam(pPlayer);
		int iGrenTeam = GetTeam(pEntity);

		return iPlayerTeam != iGrenTeam;
	}

	return false;
}

bool CTeamFortress2Mod::IsRocket(edict_t *pEntity, int iTeam)
{
	return (!iTeam || (iTeam == GetTeam(pEntity))) && (strcmp(pEntity->GetClassName(), "tf_projectile_rocket") == 0);
}

edict_t *CTeamFortress2Mod::GetMediGun(edict_t *pPlayer)
{
	if (CClassInterface::GetTF2Class(pPlayer) == TF_CLASS_MEDIC)
		return m_pMediGuns[SlotOfEdict(pPlayer)];

	return NULL;
}

void CTeamFortress2Mod::FindMediGun(edict_t *pPlayer)
{
	Vector vOrigin = CBotGlobals::EntityOrigin(pPlayer);

	for (short int i = (MAX_PLAYERS + 1); i < MAX_ENTITIES; i++)
	{
		edict_t *pEnt = INDEXENT(i);
		if (pEnt && CBotGlobals::EntityIsValid(pEnt) && !strcmp(pEnt->GetClassName(), "tf_weapon_medigun"))
		{
			if ((CBotGlobals::EntityOrigin(pEnt) - vOrigin).AsVector2D().IsLengthLessThan(8.0f))
			{
				m_pMediGuns[SlotOfEdict(pPlayer)] = pEnt;
				break;
			}
		}
	}
}

// Get the teleporter exit of an entrance
edict_t *CTeamFortress2Mod::GetTeleporterExit(edict_t *pTele)
{
	int i;
	edict_t *pExit;

	for (i = 0; i < MAX_PLAYERS; i++)
	{
		if (m_Teleporters[i].entrance.Get() == pTele)
		{
			if ((pExit = m_Teleporters[i].exit.Get()) != NULL)
			{
				return pExit;
			}

			return NULL;
		}
	}

	return NULL;
}

// check if the entity is a health kit
bool CTeamFortress2Mod::IsHealthKit(edict_t *pEntity)
{
	return strncmp(pEntity->GetClassName(), "item_healthkit", 14) == 0;
}

bool CTeamFortress2Mod::IsAreaOwnedByTeam(int iArea, int iTeam)
{
	if (CTeamFortress2Mod::m_ObjectiveResource.IsInitialised())
	{
		return ((iArea == 0) || (CTeamFortress2Mod::m_ObjectiveResource.GetOwningTeam(CTeamFortress2Mod::m_ObjectiveResource.m_WaypointAreaToIndexTranslation[iArea]) == iTeam));
	}

	return false;
}

// cehc kif the team can pick up a flag in SD mode (special delivery)
bool CTeamFortress2Mod::CanTeamPickupFlag_SD(int iTeam, bool bGetUnknown)
{
	if (IsArenaPointOpen())
	{
		if (bGetUnknown)
			return (m_iFlagCarrierTeam == iTeam);
		else
			return (m_iFlagCarrierTeam == 0);
	}

	return false;
}
// For MVM only
bool CTeamFortress2Mod::GetFlagLocation(int iTeam, Vector *vec)
{
	if (GetDroppedFlagLocation(iTeam, vec))
	{
		return true;
	}
	else if (HasRoundStarted() && IsFlagCarried(iTeam))
	{
		*vec = CBotGlobals::EntityOrigin(GetFlagCarrier(iTeam));
		return true;
	}

	return false;
}

// for special delivery mod and MVM
void CTeamFortress2Mod::FlagReturned(int iTeam)
{
	m_iFlagCarrierTeam = 0;
	m_bFlagStateDefault = true;

	if (iTeam == TF2_TEAM_BLUE)
	{
		m_bFlagLocationValidBlue = false;
	}
	else if (iTeam == TF2_TEAM_RED)
	{
		m_bFlagLocationValidRed = false;
	}
}

void CTeamFortress2Mod::FlagPickedUp(int iTeam, edict_t *pPlayer)
{
	m_bFlagStateDefault = false;
	if (iTeam == TF2_TEAM_BLUE)
	{
		m_pFlagCarrierBlue = pPlayer;
		m_bFlagLocationValidBlue = false;
	}
	else if (iTeam == TF2_TEAM_RED)
	{
		m_pFlagCarrierRed = pPlayer;
		m_bFlagLocationValidRed = false;
	}

	m_iFlagCarrierTeam = iTeam;

	CBotTF2FunctionEnemyAtIntel function(iTeam, CBotGlobals::EntityOrigin(pPlayer), FLAGEVENT_PICKUP);
	CBots::BotFunction(function);
}

bool CTeamFortress2Mod::IsArenaPointOpen()
{
	return m_fArenaPointOpenTime < TIME_NOW;
}

void CTeamFortress2Mod::ResetSetupTime()
{
	m_fRoundTime = TIME_NOW + m_Timer.GetSetupTime();
	m_fArenaPointOpenTime = TIME_NOW + m_fPointTime;
}

bool CTeamFortress2Mod::HasRoundStarted()
{
	return m_bHasRoundStarted || (!IsMapType(TF_MAP_MVM) && (TIME_NOW > m_fRoundTime));
}

void CTeamFortress2Mod::SetPointOpenTime(int time)
{
	m_fArenaPointOpenTime = 0.0f;
	m_fPointTime = (float)time;
}

void CTeamFortress2Mod::SetSetupTime(int time)
{
	m_fRoundTime = 0.0f;
	m_fSetupTime = (float)time;
}

bool CTeamFortress2Mod::IsAmmo(edict_t *pEntity)
{
	static const char *szClassname;

	szClassname = pEntity->GetClassName();

	return (strcmp(szClassname, "tf_ammo_pack") == 0) || (strncmp(szClassname, "item_ammopack", 13) == 0);
}

bool CTeamFortress2Mod::IsPayloadBomb(edict_t *pEntity, int iTeam)
{
	return ((strncmp(pEntity->GetClassName(), "mapobj_cart_dispenser", 21) == 0) && (CClassInterface::GetTeam(pEntity) == iTeam));
}

#if defined USE_NAVMESH
INavMeshArea *CTeamFortress2Mod::GetBestWaypointMVM(CBot *pBot, int iFlags)
{
	Vector vFlagLocation;
	bool bFlagLocationValid = CTeamFortress2Mod::GetFlagLocation(TF2_TEAM_BLUE, &vFlagLocation);

	CNavMeshNavigator *pNav = static_cast<CNavMeshNavigator *>(pBot->GetNavigator());

	if(HasRoundStarted() && m_bMVMFlagStartValid && m_bMVMCapturePointValid && bFlagLocationValid)
	{
		if(m_bMVMAlarmSounded)
			return pNav->RandomGoalNearestArea(m_vMVMCapturePoint, true, true, iFlags, TF2_TEAM_RED);
		else if(((m_vMVMFlagStart - vFlagLocation).Length() < 1024.0f))
			return pNav->RandomGoalNearestArea(vFlagLocation, true, true, iFlags, TF2_TEAM_RED);
		else
			return pNav->RandomGoalBetweenAreas(vFlagLocation, m_vMVMCapturePoint, true, true, iFlags, TF2_TEAM_RED);
	}
	else if(bFlagLocationValid)
	{
		return pNav->RandomGoalNearestArea(vFlagLocation, true, true, iFlags, TF2_TEAM_RED);
	}

	return NULL;
}
#else
CWaypoint *CTeamFortress2Mod::GetBestWaypointMVM(CBot *pBot, int iFlags)
{
	Vector vFlagLocation;

	bool bFlagLocationValid = CTeamFortress2Mod::GetFlagLocation(TF2_TEAM_BLUE, &vFlagLocation);

	if (HasRoundStarted() && m_bMVMFlagStartValid && m_bMVMCapturePointValid && bFlagLocationValid)
	{
		if (m_bMVMAlarmSounded)
			return CWaypoints::RandomWaypointGoalNearestArea(iFlags, TF2_TEAM_RED, 0, false, pBot, true, &m_vMVMCapturePoint, -1, true, m_iCapturePointWptID);
		else if (((m_vMVMFlagStart - vFlagLocation).Length() < 1024.0f))
			return CWaypoints::RandomWaypointGoalNearestArea(iFlags, TF2_TEAM_RED, 0, false, pBot, true, &vFlagLocation, -1, true);
		else
			return CWaypoints::RandomWaypointGoalBetweenArea(iFlags, TF2_TEAM_RED, 0, false, pBot, true, &vFlagLocation, &m_vMVMCapturePoint, true, -1, m_iCapturePointWptID);
	}
	else if (bFlagLocationValid)
	{
		return CWaypoints::RandomWaypointGoalNearestArea(iFlags, TF2_TEAM_RED, 0, false, pBot, true, &vFlagLocation, -1, true, m_iFlagPointWptID);
	}

	return NULL;
}
#endif

// check voice commands
void CTeamFortress2Mod::ClientCommand(edict_t *pEntity, int argc, const char *cmd, const char *arg1, const char *arg2)
{
	if (argc > 2 && !strcmp(cmd, "voicemenu"))
	{
		// somebody said a voice command
		byte vcmd = 0;

		switch (atoi(arg1))
		{
			case 0:
				switch (atoi(arg2))
				{
					case 0:
						vcmd = TF_VC_MEDIC;
						break;
					case 1:
						vcmd = TF_VC_THANKS;
						break;
					case 2:
						vcmd = TF_VC_GOGOGO;
						break;
					case 3:
						vcmd = TF_VC_MOVEUP;
						break;
					case 4:
						vcmd = TF_VC_GOLEFT;
						break;
					case 5:
						vcmd = TF_VC_GORIGHT;
						break;
					case 6:
						vcmd = TF_VC_YES;
						break;
					case 7:
						vcmd = TF_VC_NO;
						break;
				}
				break;
			case 1:
				switch (atoi(arg2))
				{
					case 0:
						vcmd = TF_VC_INCOMING;
						break;
					case 1:
						vcmd = TF_VC_SPY;
						break;
					case 2:
						vcmd = TF_VC_SENTRYAHEAD;
						break;
					case 3:
						vcmd = TF_VC_TELEPORTERHERE;
						break;
					case 4:
						vcmd = TF_VC_DISPENSERHERE;
						break;
					case 5:
						vcmd = TF_VC_SENTRYHERE;
						break;
					case 6:
						vcmd = TF_VC_ACTIVATEUBER;
						break;
					case 7:
						vcmd = TF_VC_UBERREADY;
						break;
				}
				break;
			case 2:
				switch (atoi(arg2))
				{
					case 0:
						vcmd = TF_VC_HELP;
						break;
					case 1:
						vcmd = TF_VC_BATTLECRY;
						break;
					case 2:
						vcmd = TF_VC_CHEERS;
						break;
					case 3:
						vcmd = TF_VC_JEERS;
						break;
					case 4:
						vcmd = TF_VC_POSITIVE;
						break;
					case 5:
						vcmd = TF_VC_NEGATIVE;
						break;
					case 6:
						vcmd = TF_VC_NICESHOT;
						break;
					case 7:
						vcmd = TF_VC_GOODJOB;
						break;
				}
				break;
			default:
				return;
				break;
		}

		CBroadcastVoiceCommand voicecmd(pEntity, vcmd);
		CBots::BotFunction(voicecmd);
	}
}

// to fixed
void CTeamFortress2Mod::TeleporterBuilt(edict_t *pOwner, eObjectType type, edict_t *pBuilding)
{
	if (type != OBJ_TELE) //(type != ENGI_ENTRANCE) && (type != ENGI_EXIT))
		return;

	short int iIndex = ENTINDEX(pOwner) - 1;

	if ((iIndex < 0) || (iIndex >= MAX_PLAYERS))
		return;

	TFTeam team = GetTeam(pOwner);

	if (CTeamFortress2Mod::IsTeleporterEntrance(pBuilding, team))
		m_Teleporters[iIndex].entrance = pBuilding;
	else if (CTeamFortress2Mod::IsTeleporterExit(pBuilding, team))
		m_Teleporters[iIndex].exit = pBuilding;

	m_Teleporters[iIndex].sapper = MyEHandle();
	m_Teleporters[iIndex].m_fLastTeleported = 0.0f;

#if defined USE_NAVMESH
	m_Teleporters[iIndex].m_iWaypoint = g_pNavMesh->GetArea(CBotGlobals::EntityOrigin(pBuilding))->GetID();
#else
	m_Teleporters[iIndex].m_iWaypoint = CWaypointLocations::NearestWaypoint(CBotGlobals::EntityOrigin(pBuilding), 400.0f, -1, true);
#endif
}

// used for changing class if I'm doing badly in my team
int CTeamFortress2Mod::GetHighestScore()
{
	short int highest = 0;
	short int score;
	edict_t *edict;

	for (short int i = 1; i < MAX_PLAYERS; i++)
	{
		edict = INDEXENT(i);

		if (edict && CBotGlobals::EntityIsValid(edict))
		{
			score = (short int)CClassInterface::GetTF2Score(edict);

			if (score > highest)
			{
				highest = score;
			}
		}
	}

	return highest;
}

// check if there is another building near where I want to build
// check quickly by using the storage of sentryguns etc in the mod class
bool CTeamFortress2Mod::BuildingNearby(TFTeam iTeam, Vector vOrigin)
{
	edict_t *pPlayer;

	for (short int i = 1; i < MAX_PLAYERS; i++)
	{
		pPlayer = INDEXENT(i);

		// crash bug fix 
		if (!pPlayer || pPlayer->IsFree())
			continue;

		if (CClassInterface::GetTF2Class(pPlayer) != TF_CLASS_ENGINEER)
			continue;

		if (CClassInterface::GetTeam(pPlayer) != iTeam)
			continue;

		if (m_SentryGuns[i-1].sentry.IsValid())
		{
			if ((vOrigin - CBotGlobals::EntityOrigin(m_SentryGuns[i-1].sentry.Get())).Length() < 100)
				return true;
		}

		if (m_Dispensers[i-1].disp.IsValid())
		{
			if ((vOrigin - CBotGlobals::EntityOrigin(m_Dispensers[i-1].disp.Get())).Length() < 100)
				return true;
		}

		if (m_Teleporters[i-1].entrance.IsValid())
		{
			if ((vOrigin - CBotGlobals::EntityOrigin(m_Teleporters[i-1].entrance.Get())).Length() < 100)
				return true;
		}

		if (m_Teleporters[i-1].exit.IsValid())
		{
			if ((vOrigin - CBotGlobals::EntityOrigin(m_Teleporters[i-1].exit.Get())).Length() < 100)
				return true;
		}

	}

	return false;
}

//Get the building
edict_t *CTeamFortress2Mod::GetBuilding(eObjectType object, edict_t *pOwner)
{
	static short int i = ENTINDEX(pOwner) + 1;

	switch (object)
	{
		case OBJ_DISP:
			return m_Dispensers[i].disp.Get();
			break;
		case OBJ_SENTRY:
			return m_SentryGuns[i].sentry.Get();
			break;
		case OBJ_TELE:
			if (m_Teleporters[i].entrance.IsValid())
				return m_Teleporters[i].entrance.Get();
			return m_Teleporters[i].exit.Get();
			break;
	}

	return NULL;
}

// Get the owner of 
edict_t *CTeamFortress2Mod::GetBuildingOwner(eObjectType object, int index)
{
	switch (object)
	{
		case OBJ_DISP:
			for (short int i = 0; i < MAX_PLAYERS; i++)
			{
				if (m_Dispensers[i].disp.IsValid() && (ENTINDEX(m_Dispensers[i].disp.Get()) == index))
					return INDEXENT(i + 1);
			}
			break;
		case OBJ_SENTRY:
			for (short int i = 0; i < MAX_PLAYERS; i++)
			{
				if (m_SentryGuns[i].sentry.IsValid() && (ENTINDEX(m_SentryGuns[i].sentry.Get()) == index))
					return INDEXENT(i + 1);
			}
			break;
		case OBJ_TELE:
			for (short int i = 0; i < MAX_PLAYERS; i++)
			{
				if (m_Teleporters[i].entrance.IsValid() && (ENTINDEX(m_Teleporters[i].entrance.Get()) == index))
					return INDEXENT(i + 1);
				if (m_Teleporters[i].exit.IsValid() && (ENTINDEX(m_Teleporters[i].exit.Get()) == index))
					return INDEXENT(i + 1);
			}
			break;
	}

	return NULL;
}

edict_t *CTeamFortress2Mod::NearestDispenser(Vector vOrigin, TFTeam team)
{
	edict_t *pNearest = NULL;
	edict_t *pDisp;
	float fDist;
	float fNearest = bot_use_disp_dist.GetFloat();

	for (short int i = 0; i < MAX_PLAYERS; i++)
	{
		//m_Dispensers[i]
		pDisp = m_Dispensers[i].disp.Get();

		if (pDisp)
		{
			if (CTeamFortress2Mod::GetTeam(pDisp) == team)
			{
				fDist = (CBotGlobals::EntityOrigin(pDisp) - vOrigin).Length();

				if (fDist < fNearest)
				{
					pNearest = pDisp;
					fNearest = fDist;
				}
			}
		}
	}

	return pNearest;
}

void CTeamFortress2Mod::SapperPlaced(edict_t *pOwner, eObjectType type, edict_t *pSapper)
{
	short int index = ENTINDEX(pOwner) - 1;
	if ((index >= 0) && (index < MAX_PLAYERS))
	{
		if (type == OBJ_TELE)
			m_Teleporters[index].sapper = pSapper;
		else if (type == OBJ_DISP)
			m_Dispensers[index].sapper = pSapper;
		else if (type == OBJ_SENTRY)
			m_SentryGuns[index].sapper = pSapper;
	}
}

#if !defined USE_NAVMESH
void CTeamFortress2Mod::AddWaypointFlags(edict_t *pPlayer, edict_t *pEdict, int *iFlags, int *iArea, float *fMaxDistance)
{
	string_t model = pEdict->GetIServerEntity()->GetModelName();

	if (strncmp(pEdict->GetClassName(), "item_health", 11) == 0)
		*iFlags |= CWaypointTypes::W_FL_HEALTH;
	else if (strncmp(pEdict->GetClassName(), "item_ammo", 9) == 0)
		*iFlags |= CWaypointTypes::W_FL_AMMO;
	else if (strcmp(pEdict->GetClassName(), "prop_dynamic") == 0)
	{
		if (strcmp(model.ToCStr(), "models/props_gameplay/resupply_locker.mdl") == 0)
		{
			*iFlags |= CWaypointTypes::W_FL_RESUPPLY;

			if (CTeamFortress2Mod::GetTeam(pPlayer) == TF2_TEAM_BLUE)
				*iFlags |= CWaypointTypes::W_FL_NORED;
			else if (CTeamFortress2Mod::GetTeam(pPlayer) == TF2_TEAM_RED)
				*iFlags |= CWaypointTypes::W_FL_NOBLU;
		}
	}
	else if (strcmp(pEdict->GetClassName(), "team_control_point") == 0)
	{
		*iFlags |= CWaypointTypes::W_FL_CAPPOINT;
		*iArea = m_ObjectiveResource.GetControlPointArea(pEdict);
		*fMaxDistance = 100;
	}
}
#endif

void CTeamFortress2Mod::SapperDestroyed(edict_t *pOwner, eObjectType type, edict_t *pSapper)
{
	for (short int index = 0; index < MAX_PLAYERS; index++)
	{
		if (type == OBJ_TELE)
		{
			if (m_Teleporters[index].sapper.Get_Old() == pSapper)
				m_Teleporters[index].sapper = MyEHandle();

		}
		else if (type == OBJ_DISP)
		{
			if (m_Dispensers[index].sapper.Get_Old() == pSapper)
				m_Dispensers[index].sapper = MyEHandle();
		}
		else if (type == OBJ_SENTRY)
		{
			if (m_SentryGuns[index].sapper.Get_Old() == pSapper)
				m_SentryGuns[index].sapper = MyEHandle();
		}
	}
}

void CTeamFortress2Mod::UpdatePointMaster()
{
	if (!m_PointMasterResource.IsValid())
	{
		CBaseEntity *pMaster = servertools->FindEntityByClassname(NULL, "team_control_point_master");

		if (pMaster)
		{
			unsigned long mempoint = ((unsigned long)pMaster + bot_const_point_master_offset.GetInt());

			m_pPointMaster = (CTeamControlPointMaster*)mempoint;
			m_PointMasterResource = pMaster;
		}
	}

	if (m_pPointMaster != NULL)
	{
		m_pCurrentRound = m_pPointMaster->GetCurrentRound();
	}
}

edict_t *CTeamFortress2Mod::GetPayloadBomb(int team)
{
	if (team == TF2_TEAM_BLUE)
		return m_pPayLoadBombBlue;
	else if (team == TF2_TEAM_RED)
		return m_pPayLoadBombRed;

	return NULL;
}

void CTeamFortress2Mod::RoundReset()
{
	if (!m_ObjectiveResource.m_Resource)
	{
		m_ObjectiveResource.m_Resource = servertools->FindEntityByClassname(NULL, "tf_objective_resource");
	}

	//always Reset on new round
	if (m_ObjectiveResource.m_Resource.IsValid())
		m_ObjectiveResource.Setup();

	m_Timer.Reset();

	UpdatePointMaster();

	if (m_ObjectiveResource.IsInitialised())
	{
		int numpoints = m_ObjectiveResource.GetNumControlPoints();
		int i;

		for (i = 0; i < numpoints; i++)
		{
			if (m_ObjectiveResource.GetOwningTeam(i) != TF2_TEAM_RED)
				break;
		}

		m_ObjectiveResource.m_fUpdatePointTime = TIME_NOW + bot_tf2_autoupdate_point_time.GetFloat();
		m_ObjectiveResource.m_fNextCheckMonitoredPoint = TIME_NOW + 0.2f;

		m_ObjectiveResource.UpdatePoints();
	}

	m_iWinningTeam = 0;
	m_bRoundOver = false;
	m_bHasRoundStarted = false;
	m_iFlagCarrierTeam = 0;
	m_pPayLoadBombBlue = MyEHandle();
	m_pPayLoadBombRed = MyEHandle();


	if (IsMapType(TF_MAP_MVM))
	{
		if (!m_bMVMFlagStartValid)
		{
		#if defined USE_NAVMESH
			edict_t *pFlag = gameents->BaseEntityToEdict(servertools->FindEntityByClassname(NULL, "item_teamflag"));
			if(CBotGlobals::EntityIsValid(pFlag))
			{
				Vector vOrigin = pFlag->GetCollideable()->GetCollisionOrigin();
				INavMeshArea *pClosest = g_pNavMesh->GetNearestArea(vOrigin);
				if(pClosest)
				{
					m_vMVMFlagStart = m_vFlagLocationBlue = pClosest->GetCenter();
					m_bMVMFlagStartValid = m_bFlagLocationValidBlue = true;
					m_iFlagPointWptID = pClosest->GetID();
				}
			}
		#else
			CWaypoint *pGoal = CWaypoints::RandomWaypointGoal(CWaypointTypes::W_FL_FLAG);

			if (pGoal)
			{
				m_vMVMFlagStart = m_vFlagLocationBlue = pGoal->GetOrigin();
				m_bMVMFlagStartValid = m_bFlagLocationValidBlue = true;
				m_iFlagPointWptID = CWaypoints::GetWaypointIndex(pGoal);
			}
		#endif
		}
		else
		{
			// Reset Flag Location
			m_vFlagLocationBlue = m_vMVMFlagStart;
			m_bFlagLocationValidBlue = true;
		}

		if (!m_bMVMCapturePointValid)
		{
		#if defined USE_NAVMESH
			edict_t *pPoint = gameents->BaseEntityToEdict(servertools->FindEntityByClassname(NULL, "func_capturezone"));
			if(CBotGlobals::EntityIsValid(pPoint))
			{
				Vector vOrigin = pPoint->GetCollideable()->GetCollisionOrigin();
				INavMeshArea *pClosest = g_pNavMesh->GetNearestArea(vOrigin);
				if(pClosest)
				{
					m_vMVMCapturePoint = g_pNavMesh->GetClosestPointOnArea(pClosest, vOrigin);
					m_bMVMCapturePointValid = true;
					m_fMVMCapturePointRadius = (pClosest->GetSWCornerZ() - pClosest->GetNECornerZ()) / 2;
					m_iCapturePointWptID = pClosest->GetID();
				}
			}
		#else
			CWaypoint *pGoal = CWaypoints::RandomWaypointGoal(CWaypointTypes::W_FL_CAPPOINT);

			if (pGoal)
			{
				m_vMVMCapturePoint = pGoal->GetOrigin();
				m_bMVMCapturePointValid = true;
				m_fMVMCapturePointRadius = pGoal->GetRadius();
				m_iCapturePointWptID = CWaypoints::GetWaypointIndex(pGoal);
			}
		#endif
		}
	}
}

void CTeamFortress2Mod::SentryBuilt(edict_t *pOwner, eObjectType type, edict_t *pBuilding)
{
	static short int index;
	static tf_sentry_t *temp;

	index = ENTINDEX(pOwner) - 1;

	if ((index >= 0) && (index < MAX_PLAYERS))
	{
		if (type == OBJ_SENTRY)
		{
			temp = &(m_SentryGuns[index]);
			temp->sentry = pBuilding;
			temp->sapper = MyEHandle();
		}
	}
}

bool CTeamFortress2Mod::IsSentryGun(edict_t *pEdict)
{
	static short int i;
	static tf_sentry_t *temp;

	temp = m_SentryGuns;

	for (i = 0; i < MAX_PLAYERS; i++)
	{
		if (temp->sentry == pEdict)
			return true;

		temp++;
	}

	// Fallback
	if (GetEntSend<byte>(pEdict, "m_iObjectType") == OBJ_SENTRY)
		return true;

	return false;
}

void CTeamFortress2Mod::DispenserBuilt(edict_t *pOwner, eObjectType type, edict_t *pBuilding)
{
	static short int index;
	static tf_disp_t *temp;

	index = ENTINDEX(pOwner) - 1;

	if ((index >= 0) && (index < MAX_PLAYERS))
	{
		if (type == OBJ_DISP)
		{
			temp = &(m_Dispensers[index]);
			temp->disp = pBuilding;
			temp->sapper = MyEHandle();
		}
	}
}

void CTeamFortress2Mod::UpdateRedPayloadBomb(edict_t *pent)
{
	edict_t *cur = m_pPayLoadBombRed;

	if (cur != pent)
		m_pPayLoadBombRed = pent;
}

void CTeamFortress2Mod::UpdateBluePayloadBomb(edict_t *pent)
{
	edict_t *cur = m_pPayLoadBombBlue;

	if (cur != pent)
		m_pPayLoadBombBlue = pent;
}

bool CTeamFortress2Mod::IsDefending(edict_t *pPlayer)//, int iCapIndex = -1 )
{
	int iIndex = (1 << (ENTINDEX(pPlayer) - 1));

	if (m_ObjectiveResource.GetNumControlPoints() > 0)
	{
		int iTeam = CClassInterface::GetTeam(pPlayer);

		for (short int i = 0; i < MAX_CONTROL_POINTS; i++)
		{
			if (m_ObjectiveResource.IsCPValid(i, iTeam, TF2_POINT_DEFEND))
			{
				if ((m_iCapDefenders[i] & iIndex) == iIndex)
					return true;
			}
		}
	}

	return false;
}

bool CTeamFortress2Mod::IsCapping(edict_t *pPlayer)
{
	int index = (1 << (ENTINDEX(pPlayer) - 1));

	if (m_ObjectiveResource.GetNumControlPoints() > 0)
	{
		int iTeam = CClassInterface::GetTeam(pPlayer);

		int i = 0;

		for (i = 0; i < MAX_CAP_POINTS; i++)
		{
			if (m_ObjectiveResource.IsCPValid(i, iTeam, TF2_POINT_ATTACK))
			{
				if ((m_Cappers[i] & index) == index)
					return true;
			}
		}
	}

	return false;
}

CMannVsMachineUpgrades *CTeamFortress2Mod::GetUpgradeByIndex(int index)
{
	return &m_UpgradeManager->m_Upgrades[index];
}

int CTeamFortress2Mod::GetAttributeIndexByName(const char *pszName)
{
	short index = m_UpgradeManager->m_UpgradeMap.Find(pszName);
	if(m_UpgradeManager->m_UpgradeMap.IsValidIndex(index))
		return m_UpgradeManager->m_UpgradeMap[index];

	return 0;
}

bool CTeamFortress2Mod::CanUpgradeWithAttribute(edict_t *pPlayer, int iWeapSlot, int iAttrIndex, CMannVsMachineUpgrades *upgrade)
{
	static ICallWrapper *pWrapper = NULL;
	static bool bWarned = false;
	if(!pWrapper)
	{
		void *pAddr;
		if(!g_pGameConf->GetMemSig("CTFGameRules::CanUpgradeWithAttrib", &pAddr))
		{
			if(!bWarned)
			{
				smutils->LogError(myself, "Couldn't get signature of CTFGameRules::CanUpgradeWithAttrib, MvM logic will be broken");
				bWarned = true;
			}
			return false;
		}

		PassInfo pass[4];
		pass[0].flags = PASSFLAG_BYVAL;
		pass[0].type = PassType_Basic;
		pass[0].size = sizeof(void *);
		pass[1].flags = PASSFLAG_BYVAL;
		pass[1].type = PassType_Basic;
		pass[1].size = sizeof(int);
		pass[2].flags = PASSFLAG_BYVAL;
		pass[2].type = PassType_Basic;
		pass[2].size = sizeof(uint16_t);
		pass[3].flags = PASSFLAG_BYVAL;
		pass[3].type = PassType_Basic;
		pass[3].size = sizeof(void *);

		PassInfo ret;
		ret.flags = PASSFLAG_BYVAL;
		ret.type = PassType_Basic;
		ret.size = sizeof(bool);

		if(!(pWrapper = g_pBinTools->CreateCall(pAddr, CallConv_ThisCall, &ret, pass, 4)))
		{
			if(!bWarned)
			{
				smutils->LogError(myself, "Couldn't create call to CTFGameRules::CanUpgradeWithAttrib, MvM logic will be broken");
				bWarned = true;
			}
			return false;
		}
	}

	CBaseEntity *pEntity = gameents->EdictToBaseEntity(pPlayer);

	unsigned char vstk[sizeof(void *)*3 + sizeof(int) + sizeof(uint16_t)];
	unsigned char *vptr = vstk;

	*(void **)vptr = CGameRulesObject::GetGameRules();
	vptr += sizeof(void *);
	*(CBaseEntity **)vptr = pEntity;
	vptr += sizeof(void *);
	*(int *)vptr = iWeapSlot;
	vptr += sizeof(int);
	*(uint16_t *)vptr = iAttrIndex;
	vptr += sizeof(uint16_t);
	*(CMannVsMachineUpgrades **)vptr = upgrade;

	bool ret;
	pWrapper->Execute(vstk, &ret);

	return ret;
}

bool CTeamFortress2Mod::IsUpgradeTierEnabled(edict_t *pPlayer, int iWeapSlot, int iUpgTier)
{
	static ICallWrapper *pWrapper = NULL;
	static bool bWarned = false;
	if(!pWrapper)
	{
		void *pAddr;
		if(!g_pGameConf->GetMemSig("CTFGameRules::IsUpgradeTierEnabled", &pAddr))
		{
			if(!bWarned)
			{
				smutils->LogError(myself, "Couldn't get signature of CTFGameRules::IsUpgradeTierEnabled, MvM logic will be broken");
				bWarned = true;
			}
			return false;
		}

		PassInfo pass[3];
		pass[0].flags = PASSFLAG_BYVAL;
		pass[0].type = PassType_Basic;
		pass[0].size = sizeof(void *);
		pass[1].flags = PASSFLAG_BYVAL;
		pass[1].type = PassType_Basic;
		pass[1].size = sizeof(int);
		pass[2].flags = PASSFLAG_BYVAL;
		pass[2].type = PassType_Basic;
		pass[2].size = sizeof(int);

		PassInfo ret;
		ret.flags = PASSFLAG_BYVAL;
		ret.type = PassType_Basic;
		ret.size = sizeof(bool);

		if(!(pWrapper = g_pBinTools->CreateCall(pAddr, CallConv_ThisCall, &ret, pass, 3)))
		{
			if(!bWarned)
			{
				smutils->LogError(myself, "Couldn't create call to CTFGameRules::IsUpgradeTierEnabled, MvM logic will be broken");
				bWarned = true;
			}
			return false;
		}
	}

	CBaseEntity *pEntity = gameents->EdictToBaseEntity(pPlayer);

	unsigned char vstk[sizeof(void *)*2 + sizeof(int)*2];
	unsigned char *vptr = vstk;

	*(void **)vptr = CGameRulesObject::GetGameRules();
	vptr += sizeof(void *);
	*(CBaseEntity **)vptr = pEntity;
	vptr += sizeof(void *);
	*(int *)vptr = iWeapSlot;
	vptr += sizeof(int);
	*(int *)vptr = iUpgTier;

	bool ret;
	pWrapper->Execute(vstk, &ret);

	return ret;
}

void CTeamFortress2Mod::FreeMemory()
{
	m_UpgradeManager = nullptr;
}
