
#include "engine_wrappers.h"
#include "server_class.h"
#include "bot_const.h"
#include "bot.h"
#include "bot_globals.h"
#include "bot_getprop.h"

CClassInterfaceValue CClassInterface :: g_GetProps[GET_PROPDATA_MAX];
bool CClassInterfaceValue :: m_berror = false;

extern IServerGameDLL *servergamedll;

void UTIL_FindServerClassnamePrint(const char *name_cmd)
{
	edict_t *current;

	for (int i = 0; i < gpGlobals->maxEntities; i++)
	{
		current = engine->PEntityOfEntIndex(i);
		if (current == NULL)
		{
			continue;
		}

		IServerNetworkable *network = current->GetNetworkable();

		if (network == NULL)
		{
			continue;
		}

		ServerClass *sClass = network->GetServerClass();
		const char *name = sClass->GetName();
		

		if (strcmp(name, name_cmd) == 0)
		{
			CBotGlobals::botMessage(NULL,0,"%s",current->GetClassName());
			return;
		}
	}

	CBotGlobals::botMessage(NULL,0,"Not found");
}



void UTIL_FindServerClassPrint(const char *name_cmd)
{
	char temp[128];
	char name[128];

	strncpy(name,name_cmd,127);
	name[127] = 0;
	__strlow(name);

	ServerClass *pClass = servergamedll->GetAllServerClasses();

	while (pClass)
	{
		strncpy(temp,pClass->m_pNetworkName,127);
		temp[127] = 0;

		__strlow(temp);

		if (strstr(temp,name) != NULL )
		{
			CBotGlobals::botMessage(NULL,0,"%s",pClass->m_pNetworkName);
			//break;
		}
		pClass = pClass->m_pNext;
	}
}
/**
 * Searches for a named Server Class.
 *
 * @param name		Name of the top-level server class.
 * @return 		Server class matching the name, or NULL if none found.
 */
ServerClass *UTIL_FindServerClass(const char *name)
{
	ServerClass *pClass = servergamedll->GetAllServerClasses();

	while (pClass)
	{
		if (strcmpi(pClass->m_pNetworkName, name) == 0)
		{
			return pClass;
		}
		pClass = pClass->m_pNext;
	}

	return NULL;
	
}

/**
 * Recursively looks through a send table for a given named property.
 *
 * @param pTable	Send table to browse.
 * @param name		Property to search for.
 * @return 		SendProp pointer on success, NULL on failure.
 */
bool g_PrintProps = false;

SendProp *UTIL_FindSendProp(SendTable *pTable, const char *name)
{
	int count = pTable->GetNumProps();
	//SendTable *pTable;
	SendProp *pProp;
	for (int i=0; i<count; i++)
	{
		pProp = pTable->GetProp(i);

		if ( g_PrintProps )
			Msg("%s\n",pProp->GetName());

		if (strcmp(pProp->GetName(), name) == 0)
		{
			return pProp;
		}
		if (pProp->GetDataTable())
		{
			if ((pProp=UTIL_FindSendProp(pProp->GetDataTable(), name)) != NULL)
			{
				return pProp;
			}
		}
	}
 
	return NULL;
}
/**
 * vim: set ts=4 :
 * =============================================================================
 * SourceMod
 * Copyright (C) 2004-2008 AlliedModders LLC.  All rights reserved.
 * =============================================================================
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License, version 3.0, as published by the
 * Free Software Foundation.
 * 
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * As a special exception, AlliedModders LLC gives you permission to link the
 * code of this program (as well as its derivative works) to "Half-Life 2," the
 * "Source Engine," the "SourcePawn JIT," and any Game MODs that run on software
 * by the Valve Corporation.  You must obey the GNU General Public License in
 * all respects for all other code used.  Additionally, AlliedModders LLC grants
 * this exception to all derivative works.  AlliedModders LLC defines further
 * exceptions, found in LICENSE.txt (as of this writing, version JULY-31-2007),
 * or <http://www.sourcemod.net/license.php>.
 *
 * Version: $Id$
 */

struct sm_sendprop_info_t
{
	SendProp *prop;					/**< Property instance. */
	unsigned int actual_offset;		/**< Actual computed offset. */
};

bool UTIL_FindInSendTable(SendTable *pTable, 
						  const char *name,
						  sm_sendprop_info_t *info,
						  unsigned int offset)
{
	const char *pname;
	int props = pTable->GetNumProps();
	SendProp *prop;

	for (int i=0; i<props; i++)
	{
		prop = pTable->GetProp(i);
		pname = prop->GetName();

		if ( g_PrintProps )
			Msg("%d : %s\n",offset + prop->GetOffset(),pname);

		if (pname && strcmp(name, pname) == 0)
		{
			info->prop = prop;
			// for some reason offset is sometimes negative when it shouldn't be
			// so take the absolute value
			info->actual_offset = offset + abs(info->prop->GetOffset());
			return true;
		}
		if (prop->GetDataTable())
		{
			if (UTIL_FindInSendTable(prop->GetDataTable(), 
				name,
				info,
				offset + prop->GetOffset())
				)
			{
				return true;
			}
		}
	}

	return false;
}

bool UTIL_FindSendPropInfo(ServerClass *pInfo, const char *szType, unsigned int *offset)
{
	if ( !pInfo )
	{
		return false;
	}

	sm_sendprop_info_t temp_info;

	if (!UTIL_FindInSendTable(pInfo->m_pTable, szType, &temp_info, 0))
	{
		return false;
	}

	*offset = temp_info.actual_offset;

	return true;
}

CBaseHandle *CClassInterfaceValue :: getEntityHandle ( edict_t *edict ) 
{ 
	getData(edict); 

	return (CBaseHandle *)m_data;
}

edict_t *CClassInterfaceValue :: getEntity ( edict_t *edict ) 
{ 
	static CBaseHandle *hndl;

	m_berror = false;

	getData(edict); 


	if (m_berror)
		return NULL;

	hndl = (CBaseHandle *)m_data; 

	if ( hndl )
		return INDEXENT(hndl->GetEntryIndex());

	return NULL;
}

void CClassInterfaceValue :: init ( char *key, char *value, unsigned int preoffset )
{
	m_class = CStrings::getString(key);
	m_value = CStrings::getString(value);
	m_data = NULL;
	m_preoffset = preoffset;
	m_offset = 0;
}

void UTIL_FindPropPrint(const char *prop_name)
{
	bool bInterfaceErr = false;

	unsigned int offset;

	try
	{
		ServerClass *pClass = servergamedll->GetAllServerClasses();

		while (pClass)
		{
			offset = 0;

			UTIL_FindSendPropInfo(pClass,prop_name,&offset);

			if ( offset != 0 )
			{
				CBotGlobals::botMessage(NULL,0,"found in %s : offset %d",pClass->m_pNetworkName, offset);
				//break;
			}
			pClass = pClass->m_pNext;
		}
	}
	catch (...)
	{
		bInterfaceErr = true;
	}
}

void CClassInterfaceValue :: findOffset ( )
{
	//if (!m_offset)
	//{
	ServerClass *sc = UTIL_FindServerClass(m_class);

	if ( sc )
	{
		UTIL_FindSendPropInfo(sc,m_value,&m_offset);
	}
#ifdef _DEBUG	
	else
	{
		CBotGlobals::botMessage(NULL,1,"Warning: Couldn't find CLASS %s",m_class);
		return;
	}
#endif

	if ( m_offset > 0 )
		m_offset += m_preoffset;
#ifdef _DEBUG	
	else
	{
		CBotGlobals::botMessage(NULL,1,"Warning: Couldn't find getprop %s for class %s",m_value,m_class);
	}
#endif
}
/* Find and save all offsets at load to save CPU */
void CClassInterface:: init ()
{
	//	DEFINE_GETPROP			ID						Class			Variable	Offset
		DEFINE_GETPROP(GETPROP_TF2MINIBUILDING,"CObjectSentryGun","m_bMiniBuilding",0);

		DEFINE_GETPROP(GETPROP_TF2SCORE,"CTFPlayerResource","m_iTotalScore",0);
		DEFINE_GETPROP(GETPROP_ENTITY_FLAGS,"CBaseEntity","m_iEffectFlags",0);
		DEFINE_GETPROP(GETPROP_TEAM,"CBaseEntity","m_iTeamNum",0);
		DEFINE_GETPROP(GETPROP_PLAYERHEALTH,"CBasePlayer","m_iHealth",0);
		DEFINE_GETPROP(GETPROP_EFFECTS,"CBaseEntity","m_fEffects",0);
		DEFINE_GETPROP(GETPROP_AMMO,"CBasePlayer","m_iAmmo",0);
		DEFINE_GETPROP(GETPROP_TF2_NUMHEALERS,"CTFPlayer","m_nNumHealers",0);
		DEFINE_GETPROP(GETPROP_TF2_CONDITIONS,"CTFPlayer","m_nPlayerCond",0);
		DEFINE_GETPROP(GETPROP_VELOCITY,"CBasePlayer","m_vecVelocity[0]",0);
		DEFINE_GETPROP(GETPROP_TF2CLASS,"CTFPlayer","m_PlayerClass",4);
		DEFINE_GETPROP(GETPROP_TF2SPYMETER,"CTFPlayer","m_flCloakMeter",0);
		DEFINE_GETPROP(GETPROP_TF2SPYDISGUISED_TEAM,"CTFPlayer","m_nDisguiseTeam",0);
		DEFINE_GETPROP(GETPROP_TF2SPYDISGUISED_CLASS,"CTFPlayer","m_nDisguiseClass",0);
		DEFINE_GETPROP(GETPROP_TF2SPYDISGUISED_TARGET_INDEX,"CTFPlayer","m_iDisguiseTargetIndex",0);
		DEFINE_GETPROP(GETPROP_TF2SPYDISGUISED_DIS_HEALTH,"CTFPlayer","m_iDisguiseHealth",0);
		DEFINE_GETPROP(GETPROP_TF2MEDIGUN_HEALING,"CWeaponMedigun","m_bHealing",0);
		DEFINE_GETPROP(GETPROP_TF2MEDIGUN_TARGETTING,"CWeaponMedigun","m_hHealingTarget",0);
		DEFINE_GETPROP(GETPROP_TF2TELEPORTERMODE,"CObjectTeleporter","m_iObjectMode",0);
		DEFINE_GETPROP(GETPROP_CURRENTWEAPON,"CBaseCombatCharacter","m_hActiveWeapon",0);
		DEFINE_GETPROP(GETPROP_TF2UBERCHARGE_LEVEL,"CWeaponMedigun","m_flChargeLevel",0);
		DEFINE_GETPROP(GETPROP_TF2SENTRYHEALTH,"CObjectSentrygun","m_iHealth",0);
		DEFINE_GETPROP(GETPROP_TF2DISPENSERHEALTH,"CObjectDispenser","m_iHealth",0);
		DEFINE_GETPROP(GETPROP_TF2TELEPORTERHEALTH,"CObjectTeleporter","m_iHealth",0);
		DEFINE_GETPROP(GETPROP_TF2OBJECTCARRIED,"CObjectSentrygun","m_bCarried",0);
		DEFINE_GETPROP(GETPROP_TF2OBJECTUPGRADELEVEL,"CObjectSentrygun","m_iUpgradeLevel",0);
		DEFINE_GETPROP(GETPROP_TF2OBJECTUPGRADEMETAL,"CObjectSentrygun","m_iUpgradeMetal",0);
		DEFINE_GETPROP(GETPROP_TF2OBJECTMAXHEALTH,"CObjectSentrygun","m_iMaxHealth",0);
		DEFINE_GETPROP(GETPROP_TF2OBJECTSHELLS,"CObjectSentrygun","m_iAmmoShells",0);
		DEFINE_GETPROP(GETPROP_TF2OBJECTROCKETS,"CObjectSentrygun","m_iAmmoRockets",0);
		DEFINE_GETPROP(GETPROP_TF2DISPMETAL,"CObjectDispenser","m_iAmmoMetal",0);
		DEFINE_GETPROP(GETPROP_MAXSPEED,"CBasePlayer","m_flMaxspeed",0);
		DEFINE_GETPROP(GETPROP_CONSTRAINT_SPEED,"CTFPlayer","m_flConstraintSpeedFactor",0);
		DEFINE_GETPROP(GETPROP_TF2OBJECTBUILDING,"CObjectDispenser","m_bBuilding",0);
		DEFINE_GETPROP(GETPROP_ENTITYFLAGS,"CBasePlayer","m_fFlags",0);

		// hl2dm
		DEFINE_GETPROP(GETPROP_HL2DM_PHYSCANNON_ATTACHED,"CWeaponPhysCannon","m_hAttachedObject",0);
		DEFINE_GETPROP(GETPROP_HL2DM_PHYSCANNON_OPEN,"CWeaponPhysCannon","m_bOpen",0);
		DEFINE_GETPROP(GETPROP_HL2DM_PLAYER_AUXPOWER,"CHL2MP_Player","m_flSuitPower",0);
		DEFINE_GETPROP(GETPROP_HL2DM_LADDER_ENT,"CHL2MP_Player","m_hLadder",0);
		
		DEFINE_GETPROP(GETPROP_WEAPONLIST,"CBaseCombatCharacter","m_hMyWeapons",0);
		DEFINE_GETPROP(GETPROP_WEAPONSTATE,"CBaseCombatWeapon","m_iState",0);

		DEFINE_GETPROP(GETPROP_WEAPONCLIP1,"CBaseCombatWeapon","m_iClip1",0);
		DEFINE_GETPROP(GETPROP_WEAPONCLIP2,"CBaseCombatWeapon","m_iClip2",0);

		DEFINE_GETPROP(GETPROP_WEAPON_AMMOTYPE1,"CBaseCombatWeapon","m_iPrimaryAmmoType",0);
		DEFINE_GETPROP(GETPROP_WEAPON_AMMOTYPE2,"CBaseCombatWeapon","m_iSecondaryAmmoType",0);

		DEFINE_GETPROP(GETPROP_DOD_PLAYERCLASS,"CDODPlayer","m_iPlayerClass",0);
		DEFINE_GETPROP(GETPROP_DOD_DES_PLAYERCLASS,"CDODPlayer","m_iDesiredPlayerClass",0);

		DEFINE_GETPROP(GETPROP_DOD_STAMINA,"CDODPlayer","m_flStamina",0);
		DEFINE_GETPROP(GETPROP_DOD_PRONE,"CDODPlayer","m_bProne",0);
		DEFINE_GETPROP(GETPROP_SEQUENCE,"CBaseAnimating","m_nSequence",0);
		DEFINE_GETPROP(GETPROP_CYCLE,"CBaseAnimating","m_flCycle",0);

		DEFINE_GETPROP(GETPROP_DOD_CP_NUMCAPS,"CDODObjectiveResource","m_iNumControlPoints",0);
		DEFINE_GETPROP(GETPROP_DOD_CP_POSITIONS,"CDODObjectiveResource","m_vCPPositions",0);
		DEFINE_GETPROP(GETPROP_DOD_CP_ALLIES_REQ_CAP,"CDODObjectiveResource","m_iAlliesReqCappers",0);
		DEFINE_GETPROP(GETPROP_DOD_CP_AXIS_REQ_CAP,"CDODObjectiveResource","m_iAxisReqCappers",0);
		DEFINE_GETPROP(GETPROP_DOD_CP_NUM_AXIS,"CDODObjectiveResource","m_iNumAxis",0);
		DEFINE_GETPROP(GETPROP_DOD_CP_NUM_ALLIES,"CDODObjectiveResource","m_iNumAllies",0);
		DEFINE_GETPROP(GETPROP_DOD_CP_OWNER,"CDODObjectiveResource","m_iOwner",0);
		DEFINE_GETPROP(GETPROP_DOD_SNIPER_ZOOMED,"CDODSniperWeapon","m_bZoomed",0);
		DEFINE_GETPROP(GETPROP_DOD_MACHINEGUN_DEPLOYED,"CDODBipodWeapon","m_bDeployed",0);
		DEFINE_GETPROP(GETPROP_DOD_ROCKET_DEPLOYED,"CDODBaseRocketWeapon","m_bDeployed",0);
		DEFINE_GETPROP(GETPROP_DOD_SEMI_AUTO,"CDODFireSelectWeapon","m_bSemiAuto",0);
		DEFINE_GETPROP(GETPROP_MOVETYPE,"CBaseEntity","movetype",0);
		DEFINE_GETPROP(GETPROP_DOD_GREN_THROWER,"CDODBaseGrenade","m_hThrower",0);
		DEFINE_GETPROP(GETPROP_DOD_SCORE,"CDODPlayerResource","m_iScore",0);
		DEFINE_GETPROP(GETPROP_DOD_OBJSCORE,"CDODPlayerResource","m_iObjScore",0);
		DEFINE_GETPROP(GETPROP_DOD_DEATHS,"CDODPlayerResource","m_iDeaths",0);
		DEFINE_GETPROP(GETPROP_DOD_SMOKESPAWN_TIME,"CDODSmokeGrenade","m_flSmokeSpawnTime",0);
		DEFINE_GETPROP(GETPROP_DOD_ROUNDTIME,"CDODGameRulesProxy","m_flRestartRoundTime",0);
		DEFINE_GETPROP(GETPROP_DOD_K98ZOOM,"CWeaponK98","m_bZoomed",0);
		DEFINE_GETPROP(GETPROP_DOD_GARANDZOOM,"CWeaponGarand","m_bZoomed",0);
		DEFINE_GETPROP(GETPROP_DOD_ALLIESBOMBING,"CDODGameRulesProxy","m_bAlliesAreBombing",0);
		DEFINE_GETPROP(GETPROP_DOD_AXISBOMBING,"CDODGameRulesProxy","m_bAxisAreBombing",0);
		DEFINE_GETPROP(GETPROP_DOD_BOMBSPLANTED,"CDODObjectiveResource","m_bBombPlanted",0);
		DEFINE_GETPROP(GETPROP_DOD_BOMBSREQ,"CDODObjectiveResource","m_iBombsRequired",0);
		DEFINE_GETPROP(GETPROP_DOD_BOMBSDEFUSED,"CDODObjectiveResource","m_bBombBeingDefused",0);
		DEFINE_GETPROP(GETPROP_DOD_BOMBSREMAINING,"CDODObjectiveResource","m_iBombsRemaining",0);
		DEFINE_GETPROP(GETPROP_DOD_PLANTINGBOMB,"CDODPlayer","m_bPlanting",0);
		DEFINE_GETPROP(GETPROP_DOD_DEFUSINGBOMB,"CDODPlayer","m_bDefusing",0);
		DEFINE_GETPROP(GETPROP_DOD_BOMB_STATE,"CDODBombTarget","m_iState",0);
		DEFINE_GETPROP(GETPROP_DOD_BOMB_TEAM,"CDODBombTarget","m_iBombingTeam",0);
		DEFINE_GETPROP(GETPROP_DOD_CP_VISIBLE,"CDODObjectiveResource","m_bCPIsVisible",0);

		DEFINE_GETPROP(GETPROP_ALL_ENTOWNER,"CBaseEntity","m_hOwnerEntity",0);
		DEFINE_GETPROP(GETPROP_GROUND_ENTITY,"CBasePlayer","m_hGroundEntity",0);
		DEFINE_GETPROP(GETPROP_ORIGIN,"CBasePlayer","m_vecOrigin",0);
		DEFINE_GETPROP(GETPROP_TAKEDAMAGE,"CBaseEntity","m_takedamage",0);

		DEFINE_GETPROP(GETPROP_SENTRY_ENEMY,"CObjectSentrygun","m_hEnemy",0);
		DEFINE_GETPROP(GETPROP_WATERLEVEL,"CBasePlayer","m_nWaterLevel",0);

		DEFINE_GETPROP(GETPROP_TF2_TELEPORT_RECHARGETIME,"CObjectTeleporter","m_flRechargeTime",0);
		DEFINE_GETPROP(GETPROP_TF2_TELEPORT_RECHARGEDURATION,"CObjectTeleporter","m_flCurrentRechargeDuration",0);

		/* All the nutty TF2 Objective Resource Stuff */
		DEFINE_GETPROP(GETPROP_TF2_OBJTR_m_vCPPositions,"CTFObjectiveResource","m_vCPPositions",0);
		DEFINE_GETPROP(GETPROP_TF2_OBJTR_m_bCPIsVisible,"CTFObjectiveResource","m_bCPIsVisible",0);
		DEFINE_GETPROP(GETPROP_TF2_OBJTR_m_iTeamIcons,"CTFObjectiveResource","m_iTeamIcons",0);
		DEFINE_GETPROP(GETPROP_TF2_OBJTR_m_iTeamOverlays,"CTFObjectiveResource","m_iTeamOverlays",0);
		DEFINE_GETPROP(GETPROP_TF2_OBJTR_m_iTeamReqCappers,"CTFObjectiveResource","m_iTeamReqCappers",0);
		DEFINE_GETPROP(GETPROP_TF2_OBJTR_m_flTeamCapTime,"CTFObjectiveResource","m_flTeamCapTime",0);
		DEFINE_GETPROP(GETPROP_TF2_OBJTR_m_iPreviousPoints,"CTFObjectiveResource","m_iPreviousPoints",0);
		DEFINE_GETPROP(GETPROP_TF2_OBJTR_m_bTeamCanCap,"CTFObjectiveResource","m_bTeamCanCap",0);
		DEFINE_GETPROP(GETPROP_TF2_OBJTR_m_iTeamBaseIcons,"CTFObjectiveResource","m_iTeamBaseIcons",0);
		DEFINE_GETPROP(GETPROP_TF2_OBJTR_m_iBaseControlPoints,"CTFObjectiveResource","m_iBaseControlPoints",0);
		DEFINE_GETPROP(GETPROP_TF2_OBJTR_m_bInMiniRound,"CTFObjectiveResource","m_bInMiniRound",0);
		DEFINE_GETPROP(GETPROP_TF2_OBJTR_m_iWarnOnCap,"CTFObjectiveResource","m_iWarnOnCap",0);
		DEFINE_GETPROP(GETPROP_TF2_OBJTR_m_iCPGroup,"CTFObjectiveResource","m_iCPGroup",0);
		DEFINE_GETPROP(GETPROP_TF2_OBJTR_m_bCPLocked,"CTFObjectiveResource","m_bCPLocked",0);
		DEFINE_GETPROP(GETPROP_TF2_OBJTR_m_bTrackAlarm,"CTFObjectiveResource","m_bTrackAlarm",0);
		DEFINE_GETPROP(GETPROP_TF2_OBJTR_m_flUnlockTimes,"CTFObjectiveResource","m_flUnlockTimes",0);
		DEFINE_GETPROP(GETPROP_TF2_OBJTR_m_flCPTimerTimes,"CTFObjectiveResource","m_flCPTimerTimes",0);
		DEFINE_GETPROP(GETPROP_TF2_OBJTR_m_iNumTeamMembers,"CTFObjectiveResource","m_iNumTeamMembers",0);
		DEFINE_GETPROP(GETPROP_TF2_OBJTR_m_iCappingTeam,"CTFObjectiveResource","m_iCappingTeam",0);
		DEFINE_GETPROP(GETPROP_TF2_OBJTR_m_iTeamInZone,"CTFObjectiveResource","m_iTeamInZone",0);
		DEFINE_GETPROP(GETPROP_TF2_OBJTR_m_bBlocked,"CTFObjectiveResource","m_bBlocked",0);
		DEFINE_GETPROP(GETPROP_TF2_OBJTR_m_iOwner,"CTFObjectiveResource","m_iOwner",0);
		DEFINE_GETPROP(GETPROP_TF2_OBJTR_m_bCPCapRateScalesWithPlayers,"CTFObjectiveResource","m_bCPCapRateScalesWithPlayers",0);
		DEFINE_GETPROP(GETPROP_TF2_OBJTR_m_iNumControlPoints,"CTFObjectiveResource","m_iNumControlPoints",0);
		DEFINE_GETPROP(GETPROP_TF2_OBJTR_m_bPlayingMiniRounds,"CTFObjectiveResource","m_bPlayingMiniRounds",0);
		DEFINE_GETPROP(GETPROP_TF2_RNDTM_m_flTimerEndTime,"CTeamRoundTimer","m_flTimerEndTime",0);
		DEFINE_GETPROP(GETPROP_TF2_RNDTM_m_nSetupTimeLength,"CTeamRoundTimer","m_nSetupTimeLength",0);
		DEFINE_GETPROP(GETPROP_TF2_RNDTM_m_bInSetup,"CTeamRoundTimer","m_bInSetup",0);
		DEFINE_GETPROP(GETPROP_PIPEBOMB_OWNER,"CTFGrenadePipebombProjectile","m_hThrower",0);
		DEFINE_GETPROP(GETPROP_SENTRYGUN_PLACING,"CObjectSentrygun","m_bPlacing",0);

		DEFINE_GETPROP(GETPROP_TF2_TAUNTYAW,"CTFPlayer","m_flTauntYaw",0);
		DEFINE_GETPROP(GETPROP_TF2_HIGHFIVE,"CTFPlayer","m_bIsReadyToHighFive",0);
		DEFINE_GETPROP(GETPROP_TF2_HIGHFIVE_PARTNER,"CTFPlayer","m_hHighFivePartner",0);
		//8480 : m_hCarriedObject
		//8484 : m_bCarryingObject
		DEFINE_GETPROP(GETPROP_TF2_ISCARRYINGOBJ,"CTFPlayer","m_bCarryingObject",0);
		DEFINE_GETPROP(GETPROP_TF2_GETCARRIEDOBJ,"CTFPlayer","m_hCarriedObject",0);
		DEFINE_GETPROP(GETPROP_TF2_ATTRIBUTELIST,"CTFPlayer","m_AttributeList",0);

		// Addon stuff for TF2
		DEFINE_GETPROP(GETPROP_TF2_ITEMDEFINITIONINDEX,"CTFWeaponBase","m_iItemDefinitionIndex",0);
		DEFINE_GETPROP(GETPROP_TF2_DISGUISEWEARABLE,"CTFWearable","m_bDisguiseWearable",0);
		DEFINE_GETPROP(GETPROP_TF2_ENTITYLEVEL,"CBaseAttributableItem","m_iEntityLevel",0);
		DEFINE_GETPROP(GETPROP_TF2_RAGEMETER,"CTFPlayer","m_flRageMeter",0);
		DEFINE_GETPROP(GETPROP_TF2_RAGEDRAINING,"CTFPlayer","m_bRageDraining",0);
		DEFINE_GETPROP(GETPROP_TF2_ENTITYQUALITY,"CBaseAttributableItem","m_iEntityQuality",0);
		DEFINE_GETPROP(GETPROP_TF2_WEAPON_INITIALIZED,"CBaseAttributableItem","m_bInitialized",0);
		DEFINE_GETPROP(GETPROP_SIMULATIONTIME,"CBaseEntity","m_flSimulationTime",0);
		DEFINE_GETPROP(GETPROP_TF2_INUPGRADEZONE,"CTFPlayer","m_bInUpgradeZone",0);
		DEFINE_GETPROP(GETPROP_TF2_EXTRAWEARABLE, "CTFWeaponBase", "m_hExtraWearable", 0);
		DEFINE_GETPROP(GETPROP_TF2_EXTRAWEARABLEVIEWMODEL, "CTFWeaponBase", "m_hExtraWearableViewModel", 0); 
		DEFINE_GETPROP(GETPROP_TF2_ENERGYDRINKMETER, "CTFPlayer", "m_flEnergyDrinkMeter", 0);
		DEFINE_GETPROP(GETPROP_TF2_MEDIEVALMODE, "CTFGameRulesProxy", "m_bPlayingMedieval", 0);
		DEFINE_GETPROP(GETPROP_TF2_ACTIVEWEAPON, "CTFPlayer", "m_hActiveWeapon", 0);
		DEFINE_GETPROP(GETPROP_TF2_BUILDER_TYPE, "CTFWeaponBuilder", "m_iObjectType", 0);
		DEFINE_GETPROP(GETPROP_TF2_BUILDER_MODE, "CTFWeaponBuilder", "m_iObjectMode", 0);
		DEFINE_GETPROP(GETPROP_TF2_CHARGE_RESIST_TYPE, "CWeaponMedigun", "m_nChargeResistType", 0);
		DEFINE_GETPROP(GETPROP_TF2_ROUNDSTATE, "CTFGameRulesProxy", "m_iRoundState", 0);
		DEFINE_GETPROP(GETPROP_TF2DESIREDCLASS, "CTFPlayer", "m_iDesiredClass", 0);

		for ( unsigned int i = 0; i < GET_PROPDATA_MAX; i ++ )
		{
			//if ( g_GetProps[i]
			g_GetProps[i].findOffset();
		}
}

void CClassInterface :: setupCTeamRoundTimer ( CTeamRoundTimer *pTimer )
{
	/*
		GETPROP_TF2_RNDTM_m_flTimerEndTime,
	GETPROP_TF2_RNDTM_m_nSetupTimeLength,
	GETPROP_TF2_RNDTM_m_bInSetup,
	*/
	pTimer->m_flTimerEndTime = g_GetProps[GETPROP_TF2_RNDTM_m_flTimerEndTime].getFloatPointer(pTimer->m_Resource);
	pTimer->m_nSetupTimeLength = g_GetProps[GETPROP_TF2_RNDTM_m_nSetupTimeLength].getIntPointer(pTimer->m_Resource);
	pTimer->m_bInSetup = g_GetProps[GETPROP_TF2_RNDTM_m_bInSetup].getBoolPointer(pTimer->m_Resource);
}

//#define GETTF2OBJ_INT(x) pResource->x = g_GetProps[GETPROP_TF2_OBJTR_#x].getIntPointer(edict);

bool CClassInterface :: getTF2ObjectiveResource ( CTFObjectiveResource *pResource )
{
	edict_t *edict = pResource->m_ObjectiveResource.get();

	pResource->m_iNumControlPoints = g_GetProps[GETPROP_TF2_OBJTR_m_iNumControlPoints].getIntPointer(edict);
	pResource->m_bBlocked = g_GetProps[GETPROP_TF2_OBJTR_m_bBlocked].getBoolPointer(edict);
	pResource->m_bCPCapRateScalesWithPlayers = g_GetProps[GETPROP_TF2_OBJTR_m_bCPCapRateScalesWithPlayers].getBoolPointer(edict);
	pResource->m_bCPIsVisible = g_GetProps[GETPROP_TF2_OBJTR_m_bCPIsVisible].getIntPointer(edict);
	pResource->m_bCPLocked = g_GetProps[GETPROP_TF2_OBJTR_m_bCPLocked].getBoolPointer(edict);
	pResource->m_flCPTimerTimes = g_GetProps[GETPROP_TF2_OBJTR_m_flCPTimerTimes].getFloatPointer(edict);
	pResource->m_bTeamCanCap = g_GetProps[GETPROP_TF2_OBJTR_m_bTeamCanCap].getBoolPointer(edict);
	pResource->m_flTeamCapTime = g_GetProps[GETPROP_TF2_OBJTR_m_bTeamCanCap].getFloatPointer(edict);
	pResource->m_vCPPositions = g_GetProps[GETPROP_TF2_OBJTR_m_vCPPositions].getVectorPointer(edict);
	pResource->m_iOwner = g_GetProps[GETPROP_TF2_OBJTR_m_iOwner].getIntPointer(edict);
	pResource->m_flUnlockTimes = g_GetProps[GETPROP_TF2_OBJTR_m_flUnlockTimes].getFloatPointer(edict);
	pResource->m_iCappingTeam = g_GetProps[GETPROP_TF2_OBJTR_m_iCappingTeam].getIntPointer(edict);
	pResource->m_iCPGroup = g_GetProps[GETPROP_TF2_OBJTR_m_iCPGroup].getIntPointer(edict);
	pResource->m_bPlayingMiniRounds = g_GetProps[GETPROP_TF2_OBJTR_m_bPlayingMiniRounds].getBoolPointer(edict);
	pResource->m_iTeamIcons = g_GetProps[GETPROP_TF2_OBJTR_m_iTeamIcons].getIntPointer(edict);
	pResource->m_iTeamOverlays = g_GetProps[GETPROP_TF2_OBJTR_m_iTeamOverlays].getIntPointer(edict);
	pResource->m_iTeamReqCappers = g_GetProps[GETPROP_TF2_OBJTR_m_iTeamReqCappers].getIntPointer(edict);
	pResource->m_iPreviousPoints = g_GetProps[GETPROP_TF2_OBJTR_m_iPreviousPoints].getIntPointer(edict);
	pResource->m_iTeamBaseIcons = g_GetProps[GETPROP_TF2_OBJTR_m_iTeamBaseIcons].getIntPointer(edict);
	pResource->m_bInMiniRound = g_GetProps[GETPROP_TF2_OBJTR_m_bInMiniRound].getBoolPointer(edict);
	pResource->m_iWarnOnCap = g_GetProps[GETPROP_TF2_OBJTR_m_iWarnOnCap].getIntPointer(edict);
	pResource->m_iNumTeamMembers = g_GetProps[GETPROP_TF2_OBJTR_m_iNumTeamMembers].getIntPointer(edict);
	pResource->m_bTrackAlarm = g_GetProps[GETPROP_TF2_OBJTR_m_bTrackAlarm].getBoolPointer(edict);
	pResource->m_iTeamInZone = g_GetProps[GETPROP_TF2_OBJTR_m_iTeamInZone].getIntPointer(edict);
	pResource->m_iBaseControlPoints = g_GetProps[GETPROP_TF2_OBJTR_m_iBaseControlPoints].getIntPointer(edict);
	return true;
}


void CClassInterfaceValue :: getData ( void *edict, bool bIsEdict )
{
	static IServerUnknown *pUnknown;
	static CBaseEntity *pEntity;

	if (!m_offset || (edict==NULL))
	{
		m_data = NULL;
		m_berror = true;
		return;
	}

	if (bIsEdict)
	{
		edict_t *pEdict = reinterpret_cast<edict_t*>(edict);

		pUnknown = (IServerUnknown *)pEdict->GetUnknown();

		if (!pUnknown)
		{
			m_data = NULL;
			m_berror = true;
			return;
		}

		pEntity = pUnknown->GetBaseEntity();

		m_data = (void *)((char *)pEntity + m_offset);
	}
	else
	{
		// raw
		m_data = (void *)((char *)edict + m_offset);
	}

}

edict_t *CClassInterface::FindEntityByClassnameNearest(Vector vstart, const char *classname, float fMindist, edict_t *pOwner)
{
	edict_t *current;
	edict_t *pfound = NULL;
	float fDist;
	const char *pszClassname;
	// speed up loop by by using smaller ints in register
	register short int max = (short int)gpGlobals->maxEntities;

	for (register short int i = 0; i < max; i++)
	{
		current = engine->PEntityOfEntIndex(i);

		if (current == NULL)
			continue;

		if ( current->IsFree() )
			continue;

		if ( pOwner != NULL )
		{
			if ( getOwner(current) != pOwner )
				continue;
		}

		pszClassname = current->GetClassName(); // For Debugging purposes

		if (strcmp(classname, pszClassname) == 0)
		{
			fDist = (vstart - CBotGlobals::entityOrigin(current)).Length();

			if ( !pfound  || (fDist < fMindist))
			{
				fMindist = fDist;
				pfound = current;
			}
		}
	}

	return pfound;
}

edict_t *CClassInterface::FindEntityByNetClassNearest(Vector vstart, const char *classname)
{
	edict_t *current;
	edict_t *pfound = NULL;
	float fMindist = 8192.0f;
	float fDist;

	for (short int i = 0; i < gpGlobals->maxEntities; i++)
	{
		current = engine->PEntityOfEntIndex(i);
		if (current == NULL)
		{
			continue;
		}
		if ( current->IsFree() )
			continue;
		if ( current->GetUnknown() == NULL )
			continue;
		
		IServerNetworkable *network = current->GetNetworkable();

		if (network == NULL)
		{
			continue;
		}

		ServerClass *sClass = network->GetServerClass();
		const char *name = sClass->GetName();
		
		if (strcmp(name, classname) == 0)
		{
			fDist = (vstart - CBotGlobals::entityOrigin(current)).Length();

			if ( !pfound  || (fDist < fMindist))
			{
				fMindist = fDist;
				pfound = current;
			}
		}
	}

	return pfound;
}

const char *CClassInterface::FindEntityNetClass(int start, const char *classname)
{
	edict_t *current;

	for (int i = ((start != -1) ? start : 0); i < gpGlobals->maxEntities; i++)
	{
		current = engine->PEntityOfEntIndex(i);
		if (current == NULL)
		{
			continue;
		}

		IServerNetworkable *network = current->GetNetworkable();

		if (network == NULL)
		{
			continue;
		}

		if (strcmp(current->GetClassName(), classname) == 0)
		{
			ServerClass *sClass = network->GetServerClass();
			
			return sClass->GetName();
		
		}
	}

	return NULL;
}
// http://svn.alliedmods.net/viewvc.cgi/trunk/extensions/tf2/extension.cpp?revision=2183&root=sourcemod&pathrev=2183
edict_t *CClassInterface::FindEntityByNetClass(int start, const char *classname)
{
	edict_t *current;

	for (int i = ((start != -1) ? start : 0); i < gpGlobals->maxEntities; i++)
	{
		current = engine->PEntityOfEntIndex(i);
		if (current == NULL)
		{
			continue;
		}

		IServerNetworkable *network = current->GetNetworkable();

		if (network == NULL)
		{
			continue;
		}

		ServerClass *sClass = network->GetServerClass();
		const char *name = sClass->GetName();
		

		if (strcmp(name, classname) == 0)
		{
			return current;
		}
	}

	return NULL;
}


 int CClassInterface::getTF2Score ( edict_t *edict ) 
	{ 
		edict_t *res = CTeamFortress2Mod::findResourceEntity();
		int *score_array = NULL;

		if ( res )
		{
			score_array = g_GetProps[GETPROP_TF2SCORE].getIntPointer(res);

			if ( score_array )
				return score_array[ENTINDEX(edict)-1];
		}

		return 0;
	}
