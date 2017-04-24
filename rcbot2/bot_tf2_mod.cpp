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
#include "engine_wrappers.h"

//#include "server_class.h"

#include "bot_base.h"

#include "in_buttons.h"

#include "bot_mods.h"
#include "bot_globals.h"
#include "bot_fortress.h"
#include "bot_weapons.h"
#include "bot_configfile.h"
#include "bot_getprop.h"
#include "bot_navigator.h"
#include "bot_waypoint.h"
#include "bot_waypoint_locations.h"
#include "bot_tf2_points.h"
#include "bot_gamerules.h"

#ifdef GetClassName
#undef GetClassName
#endif

eTFMapType CTeamFortress2Mod::m_MapType = TF_MAP_CTF;
tf_tele_t CTeamFortress2Mod::m_Teleporters[MAX_PLAYERS];
int CTeamFortress2Mod::m_iArea = 0;
float CTeamFortress2Mod::m_fSetupTime = 0.0f;
float CTeamFortress2Mod::m_fRoundTime = 0.0f;
MyEHandle CTeamFortress2Mod::m_pFlagCarrierRed = MyEHandle(NULL);
MyEHandle CTeamFortress2Mod::m_pFlagCarrierBlue = MyEHandle(NULL);
float CTeamFortress2Mod::m_fArenaPointOpenTime = 0.0f;
float CTeamFortress2Mod::m_fPointTime = 0.0f;
tf_sentry_t CTeamFortress2Mod::m_SentryGuns[MAX_PLAYERS];	// used to let bots know if sentries have been sapped or not
tf_disp_t  CTeamFortress2Mod::m_Dispensers[MAX_PLAYERS];	// used to let bots know where friendly/enemy dispensers are
MyEHandle CTeamFortress2Mod::m_pResourceEntity = MyEHandle(NULL);
MyEHandle CTeamFortress2Mod::m_pGameRules = MyEHandle(NULL);
bool CTeamFortress2Mod::m_bAttackDefendMap = false;
int CTeamFortress2Mod::m_Cappers[MAX_CONTROL_POINTS];
int CTeamFortress2Mod::m_iCapDefenders[MAX_CONTROL_POINTS];
bool CTeamFortress2Mod::m_bHasRoundStarted = true;
bool CTeamFortress2Mod::m_bDontClearPoints = false;
int CTeamFortress2Mod::m_iFlagCarrierTeam = 0;
MyEHandle CTeamFortress2Mod::m_pBoss = MyEHandle(NULL);
bool CTeamFortress2Mod::m_bBossSummoned = false;
MyEHandle CTeamFortress2Mod::pMediGuns[MAX_PLAYERS];
CTFObjectiveResource CTeamFortress2Mod::m_ObjectiveResource = CTFObjectiveResource();
CTeamControlPointMaster *CTeamFortress2Mod::m_PointMaster = NULL;
CTeamRoundTimer CTeamFortress2Mod::m_Timer;
MyEHandle CTeamFortress2Mod::m_PointMasterResource = MyEHandle(NULL);
CTeamControlPointRound *CTeamFortress2Mod::m_pCurrentRound = NULL;
bool CTeamFortress2Mod::bFlagStateDefault = true;
MyEHandle CTeamFortress2Mod::m_pPayLoadBombBlue = MyEHandle(NULL);
MyEHandle CTeamFortress2Mod::m_pPayLoadBombRed = MyEHandle(NULL);
bool CTeamFortress2Mod::m_bRoundOver = false;
int CTeamFortress2Mod::m_iWinningTeam = 0;
Vector CTeamFortress2Mod::m_vFlagLocationBlue = Vector(0, 0, 0);
Vector CTeamFortress2Mod::m_vFlagLocationRed = Vector(0, 0, 0);
bool CTeamFortress2Mod::m_bFlagLocationValidBlue = false;
bool CTeamFortress2Mod::m_bFlagLocationValidRed = false;
bool CTeamFortress2Mod::m_bMVMFlagStartValid = false;
Vector CTeamFortress2Mod::m_vMVMFlagStart = Vector(0, 0, 0);
bool CTeamFortress2Mod::m_bMVMCapturePointValid = false;
Vector CTeamFortress2Mod::m_vMVMCapturePoint = Vector(0, 0, 0);
bool CTeamFortress2Mod::m_bMVMAlarmSounded = false;
float CTeamFortress2Mod::m_fMVMCapturePointRadius = 0.0f;
int CTeamFortress2Mod::m_iCapturePointWptID = -1;
int CTeamFortress2Mod::m_iFlagPointWptID = -1;
vector<CTF2Loadout*> CTeamFortress2Mod::m_pLoadoutWeapons[TF2_SLOT_MAX][9];

extern ConVar bot_use_disp_dist;

extern IServerGameEnts *gameents;
extern IServerTools *servertools;

CTF2Loadout::CTF2Loadout(const char *pszClassname, int iIndex, int iQuality, int iMinLevel, int iMaxLevel)
{
	m_pszClassname = CStrings::GetString(pszClassname);
	m_iIndex = iIndex;
	m_iQuality = iQuality;
	m_iMinLevel = iMinLevel;
	m_iMaxLevel = iMaxLevel;
	m_bCanBeUsedInMedieval = false;

}

bool CTeamFortress2Mod::IsSuddenDeath()
{
	extern ConVar *mp_stalemate_enable;

	// Bot weapon Randomizer -- leonardo
	if (!mp_stalemate_enable || !mp_stalemate_enable->GetBool() || IsMapType(TF_MAP_ARENA))
		return false;

	if (!CGameRulesObject::GetGameRules())
		return false;

	return CGameRulesObject::GameRules_GetProp("m_iRoundState") == RoundState_Stalemate;
}

bool CTeamFortress2Mod::IsMedievalMode()
{
	if (!CGameRulesObject::GetGameRules())
		return false;

	return CGameRulesObject::GameRules_GetProp("m_bPlayingMedieval") == 1;
}

bool CTeamFortress2Mod::CheckWaypointForTeam(CWaypoint *pWpt, int iTeam)
{
	// Returns true if team can go to waypoint
	return m_bRoundOver || ((!pWpt->HasFlag(CWaypointTypes::W_FL_NOBLU) || (iTeam != TF2_TEAM_BLUE)) && (!pWpt->HasFlag(CWaypointTypes::W_FL_NORED) || (iTeam != TF2_TEAM_RED)));
}


bool CTeamFortress2Mod::IsWaypointAreaValid(int iWptArea, int iWptFlags)
{
	return CTeamFortress2Mod::m_ObjectiveResource.IsWaypointAreaValid(iWptArea, iWptFlags);
}
///////////////////////////
bool CTeamFortress2Mod::WithinEndOfRound(float fTime)
{
	if (m_Timer.m_flTimerEndTime)
		return (gpGlobals->curtime > (*m_Timer.m_flTimerEndTime - fTime));

	return false;
}


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

void CTeamFortress2Mod::ModFrame()
{
	if (m_bPlayerHasSpawned)
	{
		if (m_ObjectiveResource.m_ObjectiveResource == NULL)
		{
			m_ObjectiveResource.m_ObjectiveResource = CClassInterface::FindEntityByNetClass(MAX_PLAYERS + 1, "CTFObjectiveResource");

			if (m_ObjectiveResource.m_ObjectiveResource.Get() != NULL)
				m_ObjectiveResource.Setup();
		}
		else
			CTeamFortress2Mod::m_ObjectiveResource.Think();
	}
}

void CTeamFortress2Mod::InitMod()
{
	CWeapons::LoadWeapons((m_szWeaponListName == NULL) ? "TF2" : m_szWeaponListName, TF2Weaps);
	CRCBotTF2UtilFile::LoadConfig();
}

eTFMapType CTeamFortress2Mod::GetMapType()
{
	if (CGameRulesObject::GameRules_GetProp("m_bIsInTraining") == 1)
		return TF_MAP_TR;

	if (CGameRulesObject::GameRules_GetProp("m_bPlayingSpecialDeliveryMode") == 1)
		return TF_MAP_SD;
	
	if (CGameRulesObject::GameRules_GetProp("m_bPlayingMannVsMachine") == 1)
		return TF_MAP_MVM;

	if (CGameRulesObject::GameRules_GetProp("m_bPlayingKoth") == 1)
		return TF_MAP_KOTH;

	if (CGameRulesObject::GameRules_GetProp("m_bPlayingRobotDestructionMode") == 1)
		return TF_MAP_RD;

	// Cases not caught by the above
	int iGameType = CGameRulesObject::GameRules_GetProp("m_nGameType");
	switch (iGameType)
	{
		case 1:
		{
			return TF_MAP_CTF;
		}
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
				iRestrictWinner = CEntData::GetEntData(pEdict, "m_iInvalidCapWinner");
				if (iRestrictWinner > 1)
					iRestrictCount++;

				int iNewPriority = CEntData::GetEntData(pEdict, "m_nPriority");
				if (iNewPriority > iPriority)
					iPriority = iNewPriority;
				else if (iNewPriority == iPriority)
					return TF_MAP_TC;
			}

			if (iRoundCount > 1 && iRoundCount == iRestrictCount)
				return TF_MAP_CP;
			else if (iRoundCount > 1)
				return TF_MAP_TC;

			return TF_MAP_CP;
		}
		case 3:
		{
			if (servertools->FindEntityByClassname(NULL, "tf_logic_multiple_escort") != NULL)
				return TF_MAP_CARTRACE;

			return TF_MAP_CART;
		}
		case 4:
		{
			return TF_MAP_ARENA;
		}
	}

	return TF_MAP_DM;
}

void CTeamFortress2Mod::MapInit()
{
	CBotMod::MapInit();

	unsigned int i = 0;

	const char *szMapname = gpGlobals->mapname.ToCStr();

	m_pResourceEntity = NULL;
	m_ObjectiveResource.m_ObjectiveResource = NULL;
	m_ObjectiveResource.Reset();
	m_PointMaster = NULL;
	m_PointMasterResource = NULL;
	m_pCurrentRound = NULL;
	m_Timer = CTeamRoundTimer();
	bFlagStateDefault = true;
	m_bFlagLocationValidBlue = false;
	m_bFlagLocationValidRed = false;
	m_bMVMAlarmSounded = false;
	m_bMVMFlagStartValid = false;
	m_bMVMCapturePointValid = false;
	m_fMVMCapturePointRadius = 0.0f;
	m_iCapturePointWptID = -1;
	m_iFlagPointWptID = -1;

	m_MapType = GetMapType();
	//smutils->LogMessage(myself, "m_MapType = %i", m_MapType);

	m_iArea = 0;

	m_fSetupTime = 5.0f; // 5 seconds normal

	m_fRoundTime = 0.0f;

	m_pFlagCarrierRed = NULL;
	m_pFlagCarrierBlue = NULL;
	m_iFlagCarrierTeam = 0;
	m_bDontClearPoints = false;

	for (i = 0; i < MAX_PLAYERS; i++)
	{
		m_Teleporters[i].m_iWaypoint = -1;
		m_Teleporters[i].m_fLastTeleported = 0.0f;
		m_Teleporters[i].entrance = MyEHandle(NULL);
		m_Teleporters[i].exit = MyEHandle(NULL);
		m_Teleporters[i].sapper = MyEHandle(NULL);
		m_SentryGuns[i].sapper = MyEHandle(NULL);
		m_SentryGuns[i].sentry = MyEHandle(NULL);
		m_Dispensers[i].sapper = MyEHandle(NULL);
		m_Dispensers[i].disp = MyEHandle(NULL);
		pMediGuns[i] = NULL;
	}

	m_bAttackDefendMap = false;
	m_pBoss = NULL;
	m_bBossSummoned = false;

	ResetCappers();
	ResetDefenders();
}

int CTeamFortress2Mod::GetTeleporterWaypoint(edict_t *pTele)
{
	int i;

	for (i = 0; i < MAX_PLAYERS; i++)
	{
		if (m_Teleporters[i].exit.Get() == pTele)
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
	return (IsPlayerInCondition(pPlayer, TFCond_Disguised) && !IsPlayerInCondition(pPlayer, TFCond_Disguising));
}

bool CTeamFortress2Mod::IsPlayerTaunting(edict_t *pPlayer)
{
	return IsPlayerInCondition(pPlayer, TFCond_Taunting);
}

bool CTeamFortress2Mod::IsPlayerCloaked(edict_t *pPlayer)
{
	if (IsPlayerInCondition(pPlayer, TFCond_OnFire) || IsPlayerInCondition(pPlayer, TFCond_CloakFlicker))
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
	switch (iCond / 32)
	{
	case 0:
	{
		int bit = 1 << iCond;
		if ((CEntData::GetEntSend(pPlayer, "m_nPlayerCond") & bit) == bit)
		{
			return true;
		}

		if ((CEntData::GetEntSend(pPlayer, "_condition_bits") & bit) == bit)
		{
			return true;
		}
	}
	case 1:
	{
		int bit = (1 << (iCond - 32));
		if ((CEntData::GetEntSend(pPlayer, "m_nPlayerCondEx") & bit) == bit)
		{
			return true;
		}
	}
	case 2:
	{
		int bit = (1 << (iCond - 64));
		if ((CEntData::GetEntSend(pPlayer, "m_nPlayerCondEx2") & bit) == bit)
		{
			return true;
		}
	}
	case 3:
	{
		int bit = (1 << (iCond - 96));
		if ((CEntData::GetEntSend(pPlayer, "m_nPlayerCondEx3") & bit) == bit)
		{
			return true;
		}
	}
	default:
		return false;
	}
}


int CTeamFortress2Mod::NumClassOnTeam(int iTeam, int iClass)
{
	int i = 0;
	int num = 0;
	edict_t *pEdict;

	for (i = 1; i <= CBotGlobals::NumClients(); i++)
	{
		pEdict = INDEXENT(i);

		if (CBotGlobals::EntityIsValid(pEdict))
		{
			if (GetTeam(pEdict) == iTeam)
			{
				if (CClassInterface::GetTF2Class(pEdict) == iClass)
					num++;
			}
		}
	}

	return num;
}


edict_t *CTeamFortress2Mod::FindResourceEntity()
{
	if (!m_pResourceEntity) // crash fix
		m_pResourceEntity = CClassInterface::FindEntityByNetClass(MAX_PLAYERS + 1, "CTFPlayerResource");

	return m_pResourceEntity;
}

TFClass CTeamFortress2Mod::GetSpyDisguise(edict_t *pPlayer)
{
	static int iClass;

	CClassInterface::GetTF2SpyDisguised(pPlayer, &iClass, NULL, NULL, NULL);

	return (TFClass)iClass;
}


float CTeamFortress2Mod::TF2_GetClassSpeed(int iClass)
{
	switch (iClass)
	{
	case TF_CLASS_SCOUT: return 400.0f;
	case TF_CLASS_SOLDIER: return 240.0f;
	case TF_CLASS_DEMOMAN: return 280.0f;
	case TF_CLASS_MEDIC: return 320.0f;
	case TF_CLASS_PYRO: return 300.0f;
	case TF_CLASS_SPY: return 300.0f;
	case TF_CLASS_ENGINEER: return 300.0f;
	case TF_CLASS_SNIPER: return 300.0f;
	case TF_CLASS_HWGUY: return 200.0f;
	}
	return 0.0f;
}
// SourceMod
float CTeamFortress2Mod::TF2_GetPlayerSpeed(edict_t *pPlayer, TFClass iClass)
{
	IGamePlayer *pPL = playerhelpers->GetGamePlayer(pPlayer);
	if (!pPL || !pPL->IsInGame())
		return 0.0f;

	IPlayerInfo *pPI = pPL->GetPlayerInfo();
	if (!pPI || pPI->IsDead())
		return 0.0f;

	static float fSpeed;

	switch (iClass)
	{
	case TF_CLASS_SNIPER:
	{
		fSpeed = IsPlayerInCondition(pPlayer, TFCond_Slowed) ? 80.0f : 300.0f;
	}
	case TF_CLASS_SOLDIER:
	{
		edict_t *pWeapon = CEntData::GetEntSendEnt(pPlayer, "m_hActiveWeapon");
		if (pWeapon && !pWeapon->IsFree())
		{
			if (CEntData::GetEntSend(pWeapon, "m_iItemDefinitionIndex") == 128)
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
			else
				fSpeed = 240.0f;
		}
		else fSpeed = TF2_GetClassSpeed(iClass);
	}
	case TF_CLASS_DEMOMAN:
	{
		edict_t *pWeapon = CEntData::GetEntSendEnt(pPlayer, "m_hActiveWeapon");
		if (pWeapon && !pWeapon->IsFree())
		{
			if (CEntData::GetEntSend(pWeapon, "m_iItemDefinitionIndex") == 128)
				fSpeed = IsPlayerInCondition(pPlayer, TFCond_Charging) ? 638.0f : 238.0f;
			else
				fSpeed = IsPlayerInCondition(pPlayer, TFCond_Charging) ? 750.0f : 280.0f;
		}
		else fSpeed = TF2_GetClassSpeed(iClass);
	}
	case TF_CLASS_HWGUY:
	{
		if (IsPlayerInCondition(pPlayer, TFCond_Slowed))
			fSpeed = 110.0f;
		else if (IsPlayerInCondition(pPlayer, TFCond_CritCola))
			fSpeed = 310.0f;
		else
		{
			edict_t *pWeapon = CEntData::GetEntSendEnt(pPlayer, "m_hActiveWeapon");
			if (pWeapon && !pWeapon->IsFree())
			{
				int iIndex = CEntData::GetEntSend(pWeapon, "m_iItemDefinitionIndex");
				if (iIndex == 239 || iIndex == 1084 || iIndex == 1100)
					fSpeed = 300.0f;
				else
					fSpeed = 230.0f;
			}
			else fSpeed = TF2_GetClassSpeed(iClass);
		}
	}
	case TF_CLASS_ENGINEER:
		fSpeed = (CEntData::GetEntSendEnt(pPlayer, "m_hCarriedObject") != NULL) ? 225.0f : 300.0f;
	default:
		fSpeed = TF2_GetClassSpeed(iClass);
	}

	if (IsPlayerInCondition(pPlayer, TFCond_Dazed))
		fSpeed *= 0.5;

	if (IsPlayerInCondition(pPlayer, TFCond_SpeedBuffAlly))
		fSpeed *= 1.4;

	return fSpeed;
}

int CTeamFortress2Mod::GetTeam(edict_t *pEntity)
{
	return CClassInterface::GetTeam(pEntity);
}

int CTeamFortress2Mod::GetSentryLevel(edict_t *pSentry)
{
	return CClassInterface::GetTF2UpgradeLevel(pSentry);
}

int CTeamFortress2Mod::GetDispenserLevel(edict_t *pDispenser)
{
	return CClassInterface::GetTF2UpgradeLevel(pDispenser);
}

int CTeamFortress2Mod::GetEnemyTeam(int iTeam)
{
	return (iTeam == TF2_TEAM_BLUE) ? TF2_TEAM_RED : TF2_TEAM_BLUE;
}

/*
------------------int : m_nDisguiseTeam
------------------int : m_nDisguiseClass
------------------int : m_iDisguiseTarGetIndex
------------------int : m_iDisguiseHealth

*/

bool CTeamFortress2Mod::IsDispenser(edict_t *pEntity, int iTeam, bool checkcarrying)
{
	return (!iTeam || (iTeam == GetTeam(pEntity))) && (strcmp(pEntity->GetClassName(), "obj_dispenser") == 0) && (checkcarrying || !CClassInterface::IsSentryGunBeingPlaced(pEntity));
}

bool CTeamFortress2Mod::IsFlag(edict_t *pEntity, int iTeam)
{
	return (!iTeam || (GetEnemyTeam(iTeam) == GetTeam(pEntity))) && (strcmp(pEntity->GetClassName(), "item_teamflag") == 0);
}

bool CTeamFortress2Mod::IsBoss(edict_t *pEntity, float *fFactor)
{
	if (m_bBossSummoned)
	{
		if (m_pBoss.Get() && CBotGlobals::EntityIsAlive(m_pBoss.Get()))
			return m_pBoss.Get() == pEntity;
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
		if (m_pBoss.Get() == pEntity)
			return true;
		// for plr_hightower_event summon event is not called! Boo tf2!!!
		else if (strcmp(pEntity->GetClassName(), "tf_zombie") == 0)
		{
			m_pBoss = pEntity;
			return true;
		}
	}
	else if (CTeamFortress2Mod::IsMapType(TF_MAP_MVM))
	{
		if (m_pBoss.Get() == pEntity)
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


void CTeamFortress2Mod::UpdateTeleportTime(edict_t *pOwner)
{
	m_Teleporters[ENTINDEX(pOwner) - 1].m_fLastTeleported = engine->Time();
}

float CTeamFortress2Mod::GetTeleportTime(edict_t *pOwner)
{
	return m_Teleporters[ENTINDEX(pOwner) - 1].m_fLastTeleported;
}

bool CTeamFortress2Mod::IsSentry(edict_t *pEntity, int iTeam, bool checkcarrying)
{
	return (!iTeam || (iTeam == GetTeam(pEntity))) && (strcmp(pEntity->GetClassName(), "obj_sentrygun") == 0) && (checkcarrying || !CClassInterface::IsSentryGunBeingPlaced(pEntity));
}

bool CTeamFortress2Mod::IsTeleporter(edict_t *pEntity, int iTeam, bool checkcarrying)
{
	return (!iTeam || (iTeam == GetTeam(pEntity))) && (strcmp(pEntity->GetClassName(), "obj_teleporter") == 0) && (checkcarrying || !CClassInterface::IsSentryGunBeingPlaced(pEntity));
}

bool CTeamFortress2Mod::IsTeleporterEntrance(edict_t *pEntity, int iTeam, bool checkcarrying)
{
	return IsTeleporter(pEntity, iTeam) && CClassInterface::IsTeleporterMode(pEntity, TELE_ENTRANCE) && (checkcarrying || !CClassInterface::IsSentryGunBeingPlaced(pEntity));
}

bool CTeamFortress2Mod::IsTeleporterExit(edict_t *pEntity, int iTeam, bool checkcarrying)
{
	return IsTeleporter(pEntity, iTeam) && CClassInterface::IsTeleporterMode(pEntity, TELE_EXIT) && (checkcarrying || !CClassInterface::IsSentryGunBeingPlaced(pEntity));
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
		return pMediGuns[ENTINDEX(pPlayer) - 1];
	return NULL;
}

void CTeamFortress2Mod::FindMediGun(edict_t *pPlayer)
{
	static int i;
	static edict_t *pEnt;
	static Vector vOrigin;

	vOrigin = CBotGlobals::EntityOrigin(pPlayer);

	for (i = (MAX_PLAYERS + 1); i < MAX_ENTITIES; i++)
	{
		pEnt = INDEXENT(i);

		if (pEnt && CBotGlobals::EntityIsValid(pEnt))
		{
			if (strcmp(pEnt->GetClassName(), "tf_weapon_medigun") == 0)
			{
				if (CBotGlobals::EntityOrigin(pEnt) == vOrigin)
				{
					pMediGuns[ENTINDEX(pPlayer) - 1] = pEnt;
					break;
				}
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

			return false;
		}
	}

	return false;
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
	bFlagStateDefault = true;

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
	bFlagStateDefault = false;
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

	CBotTF2FunctionEnemyAtIntel *function = new CBotTF2FunctionEnemyAtIntel(iTeam, CBotGlobals::EntityOrigin(pPlayer), FLAGEVENT_PICKUP);

	CBots::BotFunction(function);

	delete function;
}

bool CTeamFortress2Mod::IsArenaPointOpen()
{
	return m_fArenaPointOpenTime < engine->Time();
}

void CTeamFortress2Mod::ResetSetupTime()
{
	m_fRoundTime = engine->Time() + m_Timer.GetSetupTime();
	m_fArenaPointOpenTime = engine->Time() + m_fPointTime;
}

bool CTeamFortress2Mod::HasRoundStarted()
{
	return m_bHasRoundStarted || (!IsMapType(TF_MAP_MVM) && (engine->Time() > m_fRoundTime));

	//return (engine->Time() > m_fRoundTime);
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

// check voice commands
void CTeamFortress2Mod::ClientCommand(edict_t *pEntity, int argc, const char *cmd, const char *arg1, const char *arg2)
{
	if (argc > 2 && !strcmp(cmd, "voicemenu"))
	{
		// somebody said a voice command
		u_VOICECMD vcmd;

		vcmd.voicecmd = 0;
		vcmd.b1.v1 = atoi(arg1);
		vcmd.b1.v2 = atoi(arg2);

		CBroadcastVoiceCommand voicecmd = CBroadcastVoiceCommand(pEntity, vcmd.voicecmd);

		CBots::BotFunction(&voicecmd);
	}
}

// to fixed
void CTeamFortress2Mod::TeleporterBuilt(edict_t *pOwner, eObjectType type, edict_t *pBuilding)
{
	int team;

	if ((type != OBJ_TELE)) //(type != ENGI_ENTRANCE) && (type != ENGI_EXIT) )
		return;

	short int iIndex = ENTINDEX(pOwner) - 1;

	if ((iIndex < 0) || (iIndex > MAX_PLAYERS))
		return;

	team = GetTeam(pOwner);

	if (CTeamFortress2Mod::IsTeleporterEntrance(pBuilding, team))
		m_Teleporters[iIndex].entrance = MyEHandle(pBuilding);
	else if (CTeamFortress2Mod::IsTeleporterExit(pBuilding, team))
		m_Teleporters[iIndex].exit = MyEHandle(pBuilding);

	m_Teleporters[iIndex].sapper = MyEHandle();
	m_Teleporters[iIndex].m_fLastTeleported = 0.0f;
	m_Teleporters[iIndex].m_iWaypoint = CWaypointLocations::NearestWaypoint(CBotGlobals::EntityOrigin(pBuilding), 400.0f, -1, true);
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
bool CTeamFortress2Mod::BuildingNearby(int iTeam, Vector vOrigin)
{
	edict_t *pPlayer;
	short int sentryIndex;

	for (short int i = 1; i < MAX_PLAYERS; i++)
	{
		pPlayer = INDEXENT(i);

		// crash bug fix 
		if (!pPlayer || pPlayer->IsFree())
			continue;

		sentryIndex = i - 1;

		if (CClassInterface::GetTF2Class(pPlayer) != TF_CLASS_ENGINEER)
			continue;

		if (CClassInterface::GetTeam(pPlayer) != iTeam)
			continue;

		if (m_SentryGuns[sentryIndex].sentry.Get())
		{
			if ((vOrigin - CBotGlobals::EntityOrigin(m_SentryGuns[sentryIndex].sentry.Get())).Length() < 100)
				return true;
		}

		if (m_Dispensers[sentryIndex].disp.Get())
		{
			if ((vOrigin - CBotGlobals::EntityOrigin(m_Dispensers[sentryIndex].disp.Get())).Length() < 100)
				return true;
		}

		if (m_Teleporters[sentryIndex].entrance.Get())
		{
			if ((vOrigin - CBotGlobals::EntityOrigin(m_Teleporters[sentryIndex].entrance.Get())).Length() < 100)
				return true;
		}

		if (m_Teleporters[sentryIndex].exit.Get())
		{
			if ((vOrigin - CBotGlobals::EntityOrigin(m_Teleporters[sentryIndex].exit.Get())).Length() < 100)
				return true;
		}

	}

	return false;
}

//Get the building
edict_t *CTeamFortress2Mod::GetBuilding(eObjectType object, edict_t *pOwner)
{
	static tf_tele_t *tele;
	short int i = ENTINDEX(pOwner) + 1;

	switch (object)
	{
	case OBJ_DISP:
		return m_Dispensers[i].disp.Get();
	case OBJ_SENTRY:
		return m_SentryGuns[i].sentry.Get();
	case OBJ_TELE:
		if (m_Teleporters[i].entrance.Get() != NULL)
			return m_Teleporters[i].entrance.Get();
		return m_Teleporters[i].exit.Get();
	}

	return NULL;
}

// Get the owner of 
edict_t *CTeamFortress2Mod::GetBuildingOwner(eObjectType object, short index)
{
	short int i;
	static tf_tele_t *tele;

	switch (object)
	{
	case OBJ_DISP:
		for (i = 0; i < MAX_PLAYERS; i++)
		{
			if (m_Dispensers[i].disp.Get() && (ENTINDEX(m_Dispensers[i].disp.Get()) == index))
				return INDEXENT(i + 1);
		}
		//m_SentryGuns[i].
		break;
	case OBJ_SENTRY:
		for (i = 0; i < MAX_PLAYERS; i++)
		{
			if (m_SentryGuns[i].sentry.Get() && (ENTINDEX(m_SentryGuns[i].sentry.Get()) == index))
				return INDEXENT(i + 1);
		}
		break;
	case OBJ_TELE:
		tele = m_Teleporters;

		for (i = 0; i < MAX_PLAYERS; i++)
		{
			if (tele->entrance.Get() && (ENTINDEX(tele->entrance.Get()) == index))
				return INDEXENT(i + 1);
			if (tele->exit.Get() && (ENTINDEX(tele->exit.Get()) == index))
				return INDEXENT(i + 1);

			tele++;
		}
		break;
	}

	return NULL;
}

edict_t *CTeamFortress2Mod::NearestDispenser(Vector vOrigin, int team)
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
			m_Teleporters[index].sapper = MyEHandle(pSapper);
		else if (type == OBJ_DISP)
			m_Dispensers[index].sapper = MyEHandle(pSapper);
		else if (type == OBJ_SENTRY)
			m_SentryGuns[index].sapper = MyEHandle(pSapper);
	}
}

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
	if (m_PointMasterResource.Get() == NULL)
	{
		edict_t *pMaster = CClassInterface::FindEntityByClassname(MAX_PLAYERS, "team_control_point_master");

		if (pMaster)
		{
			extern ConVar bot_const_point_master_offset;
			extern IServerGameEnts *gameents;

			CBaseEntity *pMasterEntity = gameents->EdictToBaseEntity(pMaster);

			unsigned long full_size = sizeof(pMasterEntity);
			unsigned long mempoint = ((unsigned long)pMasterEntity) + bot_const_point_master_offset.GetInt();

			m_PointMaster = (CTeamControlPointMaster*)mempoint;
			m_PointMasterResource = pMaster;
		}
	}

	if (m_PointMaster != NULL)
	{
		m_pCurrentRound = m_PointMaster->GetCurrentRound();
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
	if (m_ObjectiveResource.m_ObjectiveResource.Get() == NULL)
	{
		m_ObjectiveResource.m_ObjectiveResource = CClassInterface::FindEntityByNetClass(MAX_PLAYERS + 1, "CTFObjectiveResource");
	}

	//always Reset on new round
	if (m_ObjectiveResource.m_ObjectiveResource.Get() != NULL)
		m_ObjectiveResource.Setup();

	m_Timer.Reset();

	UpdatePointMaster();

	if (m_ObjectiveResource.IsInitialised())
	{
		extern ConVar bot_tf2_autoupdate_point_time;
		int numpoints = m_ObjectiveResource.GetNumControlPoints();
		int i;

		for (i = 0; i < numpoints; i++)
		{
			if (m_ObjectiveResource.GetOwningTeam(i) != TF2_TEAM_RED)
				break;
		}

		// if all points are owned by RED at start up then its an attack defend map
		SetAttackDefendMap(i == numpoints);

		m_ObjectiveResource.m_fUpdatePointTime = engine->Time() + bot_tf2_autoupdate_point_time.GetFloat();
		m_ObjectiveResource.m_fNextCheckMonitoredPoint = engine->Time() + 0.2f;

		m_ObjectiveResource.UpdatePoints();

	}

	m_iWinningTeam = 0;
	m_bRoundOver = false;
	m_bHasRoundStarted = false;
	m_iFlagCarrierTeam = 0;
	m_pPayLoadBombBlue = NULL;
	m_pPayLoadBombRed = NULL;


	if (IsMapType(TF_MAP_MVM))
	{
		if (!m_bMVMFlagStartValid)
		{
			CWaypoint *pGoal = CWaypoints::RandomWaypointGoal(CWaypointTypes::W_FL_FLAG);

			if (pGoal)
			{
				m_vMVMFlagStart = m_vFlagLocationBlue = pGoal->GetOrigin();
				m_bMVMFlagStartValid = m_bFlagLocationValidBlue = true;
				m_iFlagPointWptID = CWaypoints::GetWaypointIndex(pGoal);
			}
		}
		else
		{
			// Reset Flag Location
			m_vFlagLocationBlue = m_vMVMFlagStart;
			m_bFlagLocationValidBlue = true;
		}

		if (!m_bMVMCapturePointValid)
		{
			CWaypoint *pGoal = CWaypoints::RandomWaypointGoal(CWaypointTypes::W_FL_CAPPOINT);

			if (pGoal)
			{
				m_vMVMCapturePoint = pGoal->GetOrigin();
				m_bMVMCapturePointValid = true;
				m_fMVMCapturePointRadius = pGoal->GetRadius();
				m_iCapturePointWptID = CWaypoints::GetWaypointIndex(pGoal);
			}
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
			temp->sentry = MyEHandle(pBuilding);
			temp->sapper = MyEHandle();
			//m_SentryGuns[index].builder
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
			temp->disp = MyEHandle(pBuilding);
			temp->sapper = MyEHandle();
			//m_Dispensers[index].builder = userid;
		}
	}
}

void CTeamFortress2Mod::UpdateRedPayloadBomb(edict_t *pent)
{
	edict_t *cur = m_pPayLoadBombRed.Get();

	if (cur != pent)
		m_pPayLoadBombRed = pent;
}

void CTeamFortress2Mod::UpdateBluePayloadBomb(edict_t *pent)
{
	edict_t *cur = m_pPayLoadBombBlue.Get();

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

void CTeamFortress2Mod::FreeMemory()
{
	vector<CTF2Loadout*> loadouts_to_free;

	for (unsigned int i = 0; i < TF2_SLOT_MAX; i++)
	{
		for (unsigned int j = 0; j < 9; j++)
		{
			for (unsigned int k = 0; k < m_pLoadoutWeapons[i][j].size(); k++)
			{
				CTF2Loadout *wep = (m_pLoadoutWeapons[i][j]).at(k);

				bool bfound = false;

				for (unsigned int l = 0; l < loadouts_to_free.size(); l++)
				{
					if (loadouts_to_free.at(l) == wep)
					{
						bfound = true;
						break;
					}
				}

				if (bfound == false)
					loadouts_to_free.push_back(wep);

				(m_pLoadoutWeapons[i][j])[k] = NULL;
			}

			(m_pLoadoutWeapons[i][j]).clear();
		}
	}

	for (unsigned int i = 0; i < loadouts_to_free.size(); i++)
	{
		CTF2Loadout *wep = loadouts_to_free.at(i);

		delete wep;
		loadouts_to_free[i] = NULL;
	}

	loadouts_to_free.clear();
}
