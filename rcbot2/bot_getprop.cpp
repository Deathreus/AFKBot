
#include "engine_wrappers.h"
#include "server_class.h"
#include "bot_const.h"
#include "bot_globals.h"
#include "bot_getprop.h"

#ifdef GetClassName
#undef GetClassName
#endif

#ifdef GetProp
#undef GetProp
#endif

CClassInterfaceValue CClassInterface::g_GetProps[GET_PROPDATA_MAX];
bool CClassInterfaceValue::m_berror = false;

extern IServerGameEnts *gameents;

#define FIND_PROP_DATA(td) \
	datamap_t *pMap; \
	if ((pMap = gamehelpers->GetDataMap(pEntity)) == NULL) \
	{ \
		return 0; \
	} \
	sm_datatable_info_t info; \
	if (!gamehelpers->FindDataMapInfo(pMap, prop, &info)) \
	{ \
		return 0; \
	} \
	td = info.prop;

#define CHECK_SET_PROP_DATA_OFFSET() \
	if (element < 0 || element >= td->fieldSize) \
	{ \
		return 0; \
	} \
	\
	offset = info.actual_offset + (element * (td->fieldSizeInBytes / td->fieldSize));

#define FIND_PROP_SEND(type, type_name) \
	sm_sendprop_info_t info;\
	SendProp *pProp; \
	IServerUnknown *pUnk = (IServerUnknown *)pEntity; \
	IServerNetworkable *pNet = pUnk->GetNetworkable(); \
	if (!pNet) \
	{ \
		smutils->LogError(myself, "Edict %d is not networkable", gamehelpers->EntityToReference(pEntity)); \
		return 0; \
	} \
	if (!gamehelpers->FindSendPropInfo(pNet->GetServerClass()->GetName(), prop, &info)) \
	{ \
		return 0; \
	} \
	\
	offset = info.actual_offset; \
	pProp = info.prop; \
	bit_count = pProp->m_nBits; \
	\
	switch (pProp->GetType()) \
	{ \
		case type: \
		{ \
			if (element != 0) \
			{ \
				return 0; \
			} \
			break; \
		} \
		case DPT_DataTable: \
		{ \
			FIND_PROP_IN_SENDTABLE(info, pProp, element, type, type_name); \
			\
			offset += pProp->GetOffset(); \
			bit_count = pProp->m_nBits; \
			break; \
		} \
		default: \
		{ \
			return 0; \
		} \
	}

#define FIND_PROP_IN_SENDTABLE(info, pProp, element, type, type_name) \
	SendTable *pTable = pProp->GetDataTable(); \
	if (!pTable) \
	{ \
		return 0; \
	} \
	\
	int elementCount = pTable->GetNumProps(); \
	if (element < 0 || element >= elementCount) \
	{ \
		return 0; \
	} \
	\
	pProp = pTable->GetProp(element); \
	if (pProp->GetType() != type) \
	{ \
		return 0; \
	}

CBaseHandle *CClassInterfaceValue::GetEntityHandle(edict_t *edict)
{
	GetData(edict);

	return (CBaseHandle *)m_data;
}

edict_t *CClassInterfaceValue::GetEntity(edict_t *edict)
{
	static CBaseHandle *hndl;

	m_berror = false;

	GetData(edict);

	if (m_berror)
		return NULL;

	hndl = (CBaseHandle *)m_data;

	if (hndl)
		return INDEXENT(hndl->GetEntryIndex());

	return NULL;
}

void CClassInterfaceValue::Init(char *key, char *value, unsigned int preoffset)
{
	m_class = CStrings::GetString(key);
	m_value = CStrings::GetString(value);
	m_data = NULL;
	m_preoffset = preoffset;
	m_offset = 0;
}

void CClassInterfaceValue::FindOffset()
{
	ServerClass *sc = gamehelpers->FindServerClass(m_class);
	sm_sendprop_info_t info;

	if (sc)
	{
		if (gamehelpers->FindSendPropInfo(m_class, m_value, &info))
			m_offset = info.actual_offset;
	}
#ifdef _DEBUG	
	else
	{
		CBotGlobals::BotMessage(NULL, 1, "Warning: Couldn't find class %s", m_class);
		return;
	}
#endif

	if (m_offset > 0)
		m_offset += m_preoffset;
#ifdef _DEBUG	
	else
		CBotGlobals::BotMessage(NULL, 1, "Warning: Couldn't find getprop %s for class %s", m_value, m_class);
#endif
}
/* Find and save all offsets at load to save CPU */
void CClassInterface::Init()
{
	//	DEFINE_GETPROP			ID						Class			Variable	Offset
	DEFINE_GETPROP(GETPROP_TEAM,	"CBaseEntity", "m_iTeamNum", 0);
	DEFINE_GETPROP(GETPROP_PLAYERHEALTH, "CBasePlayer", "m_iHealth", 0);
	DEFINE_GETPROP(GETPROP_EFFECTS, "CBaseEntity", "m_fEffects", 0);
	DEFINE_GETPROP(GETPROP_AMMO, "CBasePlayer", "m_iAmmo", 0);
	DEFINE_GETPROP(GETPROP_VELOCITY, "CBasePlayer", "m_vecVelocity[0]", 0);
	DEFINE_GETPROP(GETPROP_CURRENTWEAPON, "CBaseCombatCharacter", "m_hActiveWeapon", 0);
	DEFINE_GETPROP(GETPROP_MAXSPEED, "CBasePlayer", "m_flMaxspeed", 0);
	DEFINE_GETPROP(GETPROP_ENTITYFLAGS, "CBasePlayer", "m_fFlags", 0);
	DEFINE_GETPROP(GETPROP_MOVETYPE, "CBaseEntity", "movetype", 0);
	DEFINE_GETPROP(GETPROP_ENTOWNER, "CBaseEntity", "m_hOwnerEntity", 0);
	DEFINE_GETPROP(GETPROP_GROUND_ENTITY, "CBasePlayer", "m_hGroundEntity", 0);
	DEFINE_GETPROP(GETPROP_ORIGIN, "CBasePlayer", "m_vecOrigin", 0);
	DEFINE_GETPROP(GETPROP_WATERLEVEL, "CBasePlayer", "m_nWaterLevel", 0);
	DEFINE_GETPROP(GETPROP_SEQUENCE, "CBaseAnimating", "m_nSequence", 0);
	DEFINE_GETPROP(GETPROP_CYCLE, "CBaseAnimating", "m_flCycle", 0);
	DEFINE_GETPROP(GETPROP_WEAPONLIST, "CBaseCombatCharacter", "m_hMyWeapons", 0);
	DEFINE_GETPROP(GETPROP_WEAPONSTATE, "CBaseCombatWeapon", "m_iState", 0);
	DEFINE_GETPROP(GETPROP_WEAPONCLIP1, "CBaseCombatWeapon", "m_iClip1", 0);
	DEFINE_GETPROP(GETPROP_WEAPONCLIP2, "CBaseCombatWeapon", "m_iClip2", 0);
	DEFINE_GETPROP(GETPROP_WEAPON_AMMOTYPE1, "CBaseCombatWeapon", "m_iPrimaryAmmoType", 0);
	DEFINE_GETPROP(GETPROP_WEAPON_AMMOTYPE2, "CBaseCombatWeapon", "m_iSecondaryAmmoType", 0);

	/* All the nutty TF2 Objective Resource Stuff */
	DEFINE_GETPROP(GETPROP_TF2_OBJTR_m_vCPPositions, "CTFObjectiveResource", "m_vCPPositions", 0);
	DEFINE_GETPROP(GETPROP_TF2_OBJTR_m_bCPIsVisible, "CTFObjectiveResource", "m_bCPIsVisible", 0);
	DEFINE_GETPROP(GETPROP_TF2_OBJTR_m_iTeamIcons, "CTFObjectiveResource", "m_iTeamIcons", 0);
	DEFINE_GETPROP(GETPROP_TF2_OBJTR_m_iTeamOverlays, "CTFObjectiveResource", "m_iTeamOverlays", 0);
	DEFINE_GETPROP(GETPROP_TF2_OBJTR_m_iTeamReqCappers, "CTFObjectiveResource", "m_iTeamReqCappers", 0);
	DEFINE_GETPROP(GETPROP_TF2_OBJTR_m_flTeamCapTime, "CTFObjectiveResource", "m_flTeamCapTime", 0);
	DEFINE_GETPROP(GETPROP_TF2_OBJTR_m_iPreviousPoints, "CTFObjectiveResource", "m_iPreviousPoints", 0);
	DEFINE_GETPROP(GETPROP_TF2_OBJTR_m_bTeamCanCap, "CTFObjectiveResource", "m_bTeamCanCap", 0);
	DEFINE_GETPROP(GETPROP_TF2_OBJTR_m_iTeamBaseIcons, "CTFObjectiveResource", "m_iTeamBaseIcons", 0);
	DEFINE_GETPROP(GETPROP_TF2_OBJTR_m_iBaseControlPoints, "CTFObjectiveResource", "m_iBaseControlPoints", 0);
	DEFINE_GETPROP(GETPROP_TF2_OBJTR_m_bInMiniRound, "CTFObjectiveResource", "m_bInMiniRound", 0);
	DEFINE_GETPROP(GETPROP_TF2_OBJTR_m_iWarnOnCap, "CTFObjectiveResource", "m_iWarnOnCap", 0);
	DEFINE_GETPROP(GETPROP_TF2_OBJTR_m_iCPGroup, "CTFObjectiveResource", "m_iCPGroup", 0);
	DEFINE_GETPROP(GETPROP_TF2_OBJTR_m_bCPLocked, "CTFObjectiveResource", "m_bCPLocked", 0);
	DEFINE_GETPROP(GETPROP_TF2_OBJTR_m_bTrackAlarm, "CTFObjectiveResource", "m_bTrackAlarm", 0);
	DEFINE_GETPROP(GETPROP_TF2_OBJTR_m_flUnlockTimes, "CTFObjectiveResource", "m_flUnlockTimes", 0);
	DEFINE_GETPROP(GETPROP_TF2_OBJTR_m_flCPTimerTimes, "CTFObjectiveResource", "m_flCPTimerTimes", 0);
	DEFINE_GETPROP(GETPROP_TF2_OBJTR_m_iNumTeamMembers, "CTFObjectiveResource", "m_iNumTeamMembers", 0);
	DEFINE_GETPROP(GETPROP_TF2_OBJTR_m_iCappingTeam, "CTFObjectiveResource", "m_iCappingTeam", 0);
	DEFINE_GETPROP(GETPROP_TF2_OBJTR_m_iTeamInZone, "CTFObjectiveResource", "m_iTeamInZone", 0);
	DEFINE_GETPROP(GETPROP_TF2_OBJTR_m_bBlocked, "CTFObjectiveResource", "m_bBlocked", 0);
	DEFINE_GETPROP(GETPROP_TF2_OBJTR_m_iOwner, "CTFObjectiveResource", "m_iOwner", 0);
	DEFINE_GETPROP(GETPROP_TF2_OBJTR_m_bCPCapRateScalesWithPlayers, "CTFObjectiveResource", "m_bCPCapRateScalesWithPlayers", 0);
	DEFINE_GETPROP(GETPROP_TF2_OBJTR_m_iNumControlPoints, "CTFObjectiveResource", "m_iNumControlPoints", 0);
	DEFINE_GETPROP(GETPROP_TF2_OBJTR_m_bPlayingMiniRounds, "CTFObjectiveResource", "m_bPlayingMiniRounds", 0);
	DEFINE_GETPROP(GETPROP_TF2_RNDTM_m_flTimerEndTime, "CTeamRoundTimer", "m_flTimerEndTime", 0);
	DEFINE_GETPROP(GETPROP_TF2_RNDTM_m_nSetupTimeLength, "CTeamRoundTimer", "m_nSetupTimeLength", 0);

	DEFINE_GETPROP(GETPROP_TF2_ITEMDEFINITIONINDEX, "CTFWeaponBase", "m_iItemDefinitionIndex", 0);
	DEFINE_GETPROP(GETPROP_TF2_DISGUISEWEARABLE, "CTFWearable", "m_bDisguiseWearable", 0);
	DEFINE_GETPROP(GETPROP_TF2_ENTITYLEVEL, "CBaseAttributableItem", "m_iEntityLevel", 0);
	DEFINE_GETPROP(GETPROP_TF2_ENTITYQUALITY, "CBaseAttributableItem", "m_iEntityQuality", 0);
	DEFINE_GETPROP(GETPROP_TF2_WEAPON_INITIALIZED, "CBaseAttributableItem", "m_bInitialized", 0);
	DEFINE_GETPROP(GETPROP_SIMULATIONTIME, "CBaseEntity", "m_flSimulationTime", 0);
	DEFINE_GETPROP(GETPROP_TF2_EXTRAWEARABLE, "CTFWeaponBase", "m_hExtraWearable", 0);
	DEFINE_GETPROP(GETPROP_TF2_EXTRAWEARABLEVIEWMODEL, "CTFWeaponBase", "m_hExtraWearableViewModel", 0);
	DEFINE_GETPROP(GETPROP_TF2_MEDIEVALMODE, "CTFGameRulesProxy", "m_bPlayingMedieval", 0);
	DEFINE_GETPROP(GETPROP_TF2SCORE, "CTFPlayerResource", "m_iTotalScore", 0);
	DEFINE_GETPROP(GETPROP_SENTRY_ENEMY, "CObjectSentrygun", "m_hEnemy", 0);
	DEFINE_GETPROP(GETPROP_TF2_TELEPORT_RECHARGETIME, "CObjectTeleporter", "m_flRechargeTime", 0);
	DEFINE_GETPROP(GETPROP_TF2_TELEPORT_RECHARGEDURATION, "CObjectTeleporter", "m_flCurrentRechargeDuration", 0);
	DEFINE_GETPROP(GETPROP_TF2MINIBUILDING, "CObjectSentryGun", "m_bMiniBuilding", 0);
	DEFINE_GETPROP(GETPROP_TF2_RAGEMETER, "CTFPlayer", "m_flRageMeter", 0);
	DEFINE_GETPROP(GETPROP_TF2_RAGEDRAINING, "CTFPlayer", "m_bRageDraining", 0);
	DEFINE_GETPROP(GETPROP_TF2_INUPGRADEZONE, "CTFPlayer", "m_bInUpgradeZone", 0);
	DEFINE_GETPROP(GETPROP_TF2_ENERGYDRINKMETER, "CTFPlayer", "m_flEnergyDrinkMeter", 0);
	DEFINE_GETPROP(GETPROP_TF2_BUILDER_TYPE, "CTFWeaponBuilder", "m_iObjectType", 0);
	DEFINE_GETPROP(GETPROP_TF2_BUILDER_MODE, "CTFWeaponBuilder", "m_iObjectMode", 0);
	DEFINE_GETPROP(GETPROP_TF2_ACTIVEWEAPON, "CTFPlayer", "m_hActiveWeapon", 0);
	DEFINE_GETPROP(GETPROP_TF2_CHARGE_RESIST_TYPE, "CWeaponMedigun", "m_nChargeResistType", 0);
	DEFINE_GETPROP(GETPROP_TF2_ROUNDSTATE, "CTFGameRulesProxy", "m_iRoundState", 0);
	DEFINE_GETPROP(GETPROP_TF2DESIREDCLASS, "CTFPlayer", "m_iDesiredPlayerClass", 0);
	DEFINE_GETPROP(GETPROP_TF2_NUMHEALERS, "CTFPlayer", "m_nNumHealers", 0);
	DEFINE_GETPROP(GETPROP_TF2_CONDITIONS, "CTFPlayer", "m_nPlayerCond", 0);
	DEFINE_GETPROP(GETPROP_TF2CLASS, "CTFPlayer", "m_iClass", 0);
	DEFINE_GETPROP(GETPROP_TF2SPYMETER, "CTFPlayer", "m_flCloakMeter", 0);
	DEFINE_GETPROP(GETPROP_TF2SPYDISGUISED_TEAM, "CTFPlayer", "m_nDisguiseTeam", 0);
	DEFINE_GETPROP(GETPROP_TF2SPYDISGUISED_CLASS, "CTFPlayer", "m_nDisguiseClass", 0);
	DEFINE_GETPROP(GETPROP_TF2SPYDISGUISED_TARGET_INDEX, "CTFPlayer", "m_iDisguiseTargetIndex", 0);
	DEFINE_GETPROP(GETPROP_TF2SPYDISGUISED_DIS_HEALTH, "CTFPlayer", "m_iDisguiseHealth", 0);
	DEFINE_GETPROP(GETPROP_TF2MEDIGUN_HEALING, "CWeaponMedigun", "m_bHealing", 0);
	DEFINE_GETPROP(GETPROP_TF2MEDIGUN_TARGETTING, "CWeaponMedigun", "m_hHealingTarget", 0);
	DEFINE_GETPROP(GETPROP_TF2TELEPORTERMODE, "CObjectTeleporter", "m_iObjectMode", 0);
	DEFINE_GETPROP(GETPROP_TF2UBERCHARGE_LEVEL, "CWeaponMedigun", "m_flChargeLevel", 0);
	DEFINE_GETPROP(GETPROP_TF2SENTRYHEALTH, "CObjectSentrygun", "m_iHealth", 0);
	DEFINE_GETPROP(GETPROP_TF2DISPENSERHEALTH, "CObjectDispenser", "m_iHealth", 0);
	DEFINE_GETPROP(GETPROP_TF2TELEPORTERHEALTH, "CObjectTeleporter", "m_iHealth", 0);
	DEFINE_GETPROP(GETPROP_TF2OBJECTCARRIED, "CObjectSentrygun", "m_bCarried", 0);
	DEFINE_GETPROP(GETPROP_TF2OBJECTUPGRADELEVEL, "CObjectSentrygun", "m_iUpgradeLevel", 0);
	DEFINE_GETPROP(GETPROP_TF2OBJECTUPGRADEMETAL, "CObjectSentrygun", "m_iUpgradeMetal", 0);
	DEFINE_GETPROP(GETPROP_TF2OBJECTMAXHEALTH, "CObjectSentrygun", "m_iMaxHealth", 0);
	DEFINE_GETPROP(GETPROP_TF2OBJECTSHELLS, "CObjectSentrygun", "m_iAmmoShells", 0);
	DEFINE_GETPROP(GETPROP_TF2OBJECTROCKETS, "CObjectSentrygun", "m_iAmmoRockets", 0);
	DEFINE_GETPROP(GETPROP_TF2DISPMETAL, "CObjectDispenser", "m_iAmmoMetal", 0);
	DEFINE_GETPROP(GETPROP_CONSTRAINT_SPEED, "CTFPlayer", "m_flConstraintSpeedFactor", 0);
	DEFINE_GETPROP(GETPROP_TF2OBJECTBUILDING, "CObjectDispenser", "m_bBuilding", 0);
	DEFINE_GETPROP(GETPROP_PIPEBOMB_OWNER, "CTFGrenadePipebombProjectile", "m_hThrower", 0);
	DEFINE_GETPROP(GETPROP_SENTRYGUN_PLACING, "CObjectSentrygun", "m_bPlacing", 0);
	DEFINE_GETPROP(GETPROP_TF2_TAUNTYAW, "CTFPlayer", "m_flTauntYaw", 0);
	DEFINE_GETPROP(GETPROP_TF2_HIGHFIVE, "CTFPlayer", "m_bIsReadyToHighFive", 0);
	DEFINE_GETPROP(GETPROP_TF2_HIGHFIVE_PARTNER, "CTFPlayer", "m_hHighFivePartner", 0);
	DEFINE_GETPROP(GETPROP_TF2_ISCARRYINGOBJ, "CTFPlayer", "m_bCarryingObject", 0);
	DEFINE_GETPROP(GETPROP_TF2_GETCARRIEDOBJ, "CTFPlayer", "m_hCarriedObject", 0);
	DEFINE_GETPROP(GETPROP_TF2_ATTRIBUTELIST, "CTFPlayer", "m_AttributeList", 0);
	DEFINE_GETPROP(GETPROP_MVMHASMINPLAYERSREADY, "CTFGameRulesProxy", "m_bHaveMinPlayersToEnableReady", 0);

	for (unsigned int i = 0; i < GET_PROPDATA_MAX; i++)
	{
		g_GetProps[i].FindOffset();
	}
}

void CClassInterface::SetupCTeamRoundTimer(CTeamRoundTimer *pTimer)
{
	pTimer->m_flTimerEndTime = g_GetProps[GETPROP_TF2_RNDTM_m_flTimerEndTime].GetFloatPointer(pTimer->m_Resource);
	pTimer->m_nSetupTimeLength = g_GetProps[GETPROP_TF2_RNDTM_m_nSetupTimeLength].GetIntPointer(pTimer->m_Resource);
}

//#define GETTF2OBJ_INT(x) pResource->x = g_GetProps[GETPROP_TF2_OBJTR_#x].getIntPointer(edict);

bool CClassInterface::GetTF2ObjectiveResource(CTFObjectiveResource *pResource)
{
	edict_t *edict = pResource->m_ObjectiveResource.Get();

	pResource->m_iNumControlPoints = g_GetProps[GETPROP_TF2_OBJTR_m_iNumControlPoints].GetIntPointer(edict);
	pResource->m_bBlocked = g_GetProps[GETPROP_TF2_OBJTR_m_bBlocked].GetBoolPointer(edict);
	pResource->m_bCPCapRateScalesWithPlayers = g_GetProps[GETPROP_TF2_OBJTR_m_bCPCapRateScalesWithPlayers].GetBoolPointer(edict);
	pResource->m_bCPIsVisible = g_GetProps[GETPROP_TF2_OBJTR_m_bCPIsVisible].GetIntPointer(edict);
	pResource->m_bCPLocked = g_GetProps[GETPROP_TF2_OBJTR_m_bCPLocked].GetBoolPointer(edict);
	pResource->m_flCPTimerTimes = g_GetProps[GETPROP_TF2_OBJTR_m_flCPTimerTimes].GetFloatPointer(edict);
	pResource->m_bTeamCanCap = g_GetProps[GETPROP_TF2_OBJTR_m_bTeamCanCap].GetBoolPointer(edict);
	pResource->m_flTeamCapTime = g_GetProps[GETPROP_TF2_OBJTR_m_bTeamCanCap].GetFloatPointer(edict);
	pResource->m_vCPPositions = g_GetProps[GETPROP_TF2_OBJTR_m_vCPPositions].GetVectorPointer(edict);
	pResource->m_iOwner = g_GetProps[GETPROP_TF2_OBJTR_m_iOwner].GetIntPointer(edict);
	pResource->m_flUnlockTimes = g_GetProps[GETPROP_TF2_OBJTR_m_flUnlockTimes].GetFloatPointer(edict);
	pResource->m_iCappingTeam = g_GetProps[GETPROP_TF2_OBJTR_m_iCappingTeam].GetIntPointer(edict);
	pResource->m_iCPGroup = g_GetProps[GETPROP_TF2_OBJTR_m_iCPGroup].GetIntPointer(edict);
	pResource->m_bPlayingMiniRounds = g_GetProps[GETPROP_TF2_OBJTR_m_bPlayingMiniRounds].GetBoolPointer(edict);
	pResource->m_iTeamIcons = g_GetProps[GETPROP_TF2_OBJTR_m_iTeamIcons].GetIntPointer(edict);
	pResource->m_iTeamOverlays = g_GetProps[GETPROP_TF2_OBJTR_m_iTeamOverlays].GetIntPointer(edict);
	pResource->m_iTeamReqCappers = g_GetProps[GETPROP_TF2_OBJTR_m_iTeamReqCappers].GetIntPointer(edict);
	pResource->m_iPreviousPoints = g_GetProps[GETPROP_TF2_OBJTR_m_iPreviousPoints].GetIntPointer(edict);
	pResource->m_iTeamBaseIcons = g_GetProps[GETPROP_TF2_OBJTR_m_iTeamBaseIcons].GetIntPointer(edict);
	pResource->m_bInMiniRound = g_GetProps[GETPROP_TF2_OBJTR_m_bInMiniRound].GetBoolPointer(edict);
	pResource->m_iWarnOnCap = g_GetProps[GETPROP_TF2_OBJTR_m_iWarnOnCap].GetIntPointer(edict);
	pResource->m_iNumTeamMembers = g_GetProps[GETPROP_TF2_OBJTR_m_iNumTeamMembers].GetIntPointer(edict);
	pResource->m_bTrackAlarm = g_GetProps[GETPROP_TF2_OBJTR_m_bTrackAlarm].GetBoolPointer(edict);
	pResource->m_iTeamInZone = g_GetProps[GETPROP_TF2_OBJTR_m_iTeamInZone].GetIntPointer(edict);
	pResource->m_iBaseControlPoints = g_GetProps[GETPROP_TF2_OBJTR_m_iBaseControlPoints].GetIntPointer(edict);
	return true;
}


void CClassInterfaceValue::GetData(void *edict, bool bIsEdict)
{
	static IServerUnknown *pUnknown;
	static CBaseEntity *pEntity;

	if (!m_offset || (edict == NULL))
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

edict_t *CClassInterface::FindEntityByNetClass(int start, const char *classname)
{
	edict_t *pCurrent;
	for (register short int i = start; i < MAX_ENTITIES; i++)
	{
		pCurrent = INDEXENT(i);
		if (pCurrent == NULL)
			continue;

		IServerNetworkable *pNetwork = pCurrent->GetNetworkable();

		if (pNetwork == NULL)
			continue;

		ServerClass *pClass = pNetwork->GetServerClass();
		const char *szName = pClass->GetName();


		if (!strcmp(szName, classname))
		{
			return pCurrent;
		}
	}

	return NULL;
}

edict_t *CClassInterface::FindEntityByNetClassNearest(Vector vstart, const char *classname, float fMindist)
{
	edict_t *pCurrent;
	edict_t *pFound = NULL;
	float fDist;

	for (register short int i = 0; i < MAX_ENTITIES; i++)
	{
		pCurrent = INDEXENT(i);
		if (pCurrent == NULL)
			continue;
		if (pCurrent->IsFree())
			continue;
		if (pCurrent->GetUnknown() == NULL)
			continue;

		IServerNetworkable *pNetwork = pCurrent->GetNetworkable();

		if (pNetwork == NULL)
		{
			continue;
		}

		ServerClass *pClass = pNetwork->GetServerClass();
		const char *szName = pClass->GetName();

		if (!strcmp(szName, classname))
		{
			fDist = (vstart - CBotGlobals::EntityOrigin(pCurrent)).Length();

			if (!pFound || (fDist < fMindist))
			{
				fMindist = fDist;
				pFound = pCurrent;
			}
		}
	}

	return pFound;
}

edict_t *CClassInterface::FindEntityByClassname(int start, const char *classname)
{
	edict_t *pCurrent;
	for (register short int i = start; i < MAX_ENTITIES; i++)
	{
		pCurrent = INDEXENT(i);
		if (pCurrent == NULL)
			continue;
		if (pCurrent->IsFree())
			continue;

		const char *szName = pCurrent->GetClassName();

		if (!strcmp(szName, classname))
		{
			return pCurrent;
		}
	}

	return NULL;
}

edict_t *CClassInterface::FindEntityByClassnameNearest(Vector vstart, const char *classname, float fMindist, edict_t *pOwner)
{
	edict_t *pCurrent;
	edict_t *pFound = NULL;
	float fDist;
	const char *szName;

	for (register short int i = 0; i < MAX_ENTITIES; i++)
	{
		pCurrent = INDEXENT(i);
		if (pCurrent == NULL)
			continue;
		if (pCurrent->IsFree())
			continue;

		if (pOwner != NULL)
		{
			if (GetOwner(pCurrent) != pOwner)
				continue;
		}

		szName = pCurrent->GetClassName(); // For Debugging purposes

		if (!strcmp(classname, szName))
		{
			fDist = (vstart - CBotGlobals::EntityOrigin(pCurrent)).Length();

			if (!pFound || (fDist < fMindist))
			{
				fMindist = fDist;
				pFound = pCurrent;
			}
		}
	}

	return pFound;
}

int CClassInterface::GetTF2Score(edict_t *edict)
{
	edict_t *res = CTeamFortress2Mod::FindResourceEntity();
	int *score_array = NULL;

	if (res)
	{
		score_array = g_GetProps[GETPROP_TF2SCORE].GetIntPointer(res);

		if (score_array)
			return score_array[ENTINDEX(edict) - 1];
	}

	return 0;
}

int32_t CEntData::GetEntProp(edict_t *pEdict, const char *prop, int element)
{
	int offset;
	int bit_count;

	CBaseEntity *pEntity = gameents->EdictToBaseEntity(pEdict);

	FIND_PROP_SEND(DPT_Int, "integer");
	bool is_unsigned = ((pProp->GetFlags() & SPROP_UNSIGNED) == SPROP_UNSIGNED);
	if (pProp->GetFlags() & SPROP_VARINT)
	{
		bit_count = sizeof(int) * 8;
	}

	if (bit_count < 1)
	{
		bit_count = element * 8;
	}

	if (bit_count >= 17)
	{
		return *(int32_t *)((uint8_t *)pEntity + offset);
	}
	else if (bit_count >= 9)
	{
		if (is_unsigned)
		{
			return *(uint16_t *)((uint8_t *)pEntity + offset);
		}
		else
		{
			return *(int16_t *)((uint8_t *)pEntity + offset);
		}
	}
	else if (bit_count >= 2)
	{
		if (is_unsigned)
		{
			return *(uint8_t *)((uint8_t *)pEntity + offset);
		}
		else
		{
			return *(int8_t *)((uint8_t *)pEntity + offset);
		}
	}
	else
	{
		return *(bool *)((uint8_t *)pEntity + offset) ? 1 : 0;
	}

	return 0;
}

float CEntData::GetEntPropFloat(edict_t *pEdict, const char *prop, int element)
{
	int offset;
	int bit_count;

	CBaseEntity *pEntity = gameents->EdictToBaseEntity(pEdict);

	FIND_PROP_SEND(DPT_Float, "float");

	return *(float *)((uint8_t *)pEntity + offset);
}

const char *CEntData::GetEntPropString(edict_t *pEdict, const char *prop, int element)
{
	int offset;
	const char *src;

	CBaseEntity *pEntity = gameents->EdictToBaseEntity(pEdict);

	sm_sendprop_info_t info;

	IServerUnknown *pUnk = (IServerUnknown *)pEntity;
	IServerNetworkable *pNet = pUnk->GetNetworkable();
	if (!pNet)
	{
		return NULL;
	}
	if (!gamehelpers->FindSendPropInfo(pNet->GetServerClass()->GetName(), prop, &info))
	{
		return NULL;
	}

	offset = info.actual_offset;

	if (info.prop->GetType() != DPT_String)
	{
		return NULL;
	}
	else if (element != 0)
	{
		return NULL;
	}

	if (info.prop->GetProxyFn())
	{
		DVariant var;
		info.prop->GetProxyFn()(info.prop, pEntity, (const void *)((intptr_t)pEntity + offset), &var, element, gamehelpers->EntityToReference(pEntity));
		src = var.m_pString;
	}
	else
	{
		src = (char *)((uint8_t *)pEntity + offset);
	}

	return src;
}

Vector CEntData::GetEntPropVector(edict_t *pEdict, const char *prop, int element)
{
	int offset;
	int bit_count;

	CBaseEntity *pEntity = gameents->EdictToBaseEntity(pEdict);

	FIND_PROP_SEND(DPT_Vector, "vector");

	return *(Vector *)((uint8_t *)pEntity + offset);
}

CBaseEntity *CEntData::GetEntPropEnt(edict_t *pEdict, const char *prop, int element)
{
	int offset;
	int bit_count;

	CBaseEntity *pEntity = gameents->EdictToBaseEntity(pEdict);

	FIND_PROP_SEND(DPT_Int, "integer");

	CBaseHandle &hndl = *(CBaseHandle *)((uint8_t *)pEntity + offset);
	CBaseEntity *pHandleEntity = gamehelpers->ReferenceToEntity(hndl.GetEntryIndex());

	if (!pHandleEntity || hndl != reinterpret_cast<IHandleEntity *>(pHandleEntity)->GetRefEHandle())
		return NULL;

	return pHandleEntity;
}

int32_t CEntData::GetEntData(edict_t *pEdict, const char *prop, int element)
{
	typedescription_t *td;
	int offset;

	CBaseEntity *pEntity = gameents->EdictToBaseEntity(pEdict);

	FIND_PROP_DATA(td);
	int bit_count = MatchFieldAsGetInteger(td->fieldType);
	CHECK_SET_PROP_DATA_OFFSET();

	if (bit_count == 0)
	{
		return 0;
	}

	if (element < 0 || element >= td->fieldSize)
	{
		return 0;
	}

	if (bit_count < 1)
	{
		bit_count = 32;
	}

	if (bit_count >= 17)
	{
		return *(int32_t *)((uint8_t *)pEntity + offset);
	}
	else if (bit_count >= 9)
	{
		return *(int16_t *)((uint8_t *)pEntity + offset);
	}
	else if (bit_count >= 2)
	{
		return *(int8_t *)((uint8_t *)pEntity + offset);
	}
	else
	{
		return *(bool *)((uint8_t *)pEntity + offset) ? 1 : 0;
	}

	return 0;
}

float CEntData::GetEntDataFloat(edict_t *pEdict, const char *prop, int element)
{
	typedescription_t *td;
	int offset;

	CBaseEntity *pEntity = gameents->EdictToBaseEntity(pEdict);

	FIND_PROP_DATA(td);
	CHECK_SET_PROP_DATA_OFFSET();

	if (td->fieldType != FIELD_FLOAT
		&& td->fieldType != FIELD_TIME)
		return 0.0f;

	return *(float *)((uint8_t *)pEntity + offset);
}

const char *CEntData::GetEntDataString(edict_t *pEdict, const char *prop, int element)
{
	int offset;
	const char *src;

	CBaseEntity *pEntity = gameents->EdictToBaseEntity(pEdict);

	bool bIsStringIndex = false;
	typedescription_t *td;

	FIND_PROP_DATA(td);

	if (td->fieldType != FIELD_CHARACTER
		&& td->fieldType != FIELD_STRING
		&& td->fieldType != FIELD_MODELNAME
		&& td->fieldType != FIELD_SOUNDNAME)
	{
		return NULL;
	}

	bIsStringIndex = (td->fieldType != FIELD_CHARACTER);

	if (element != 0)
	{
		if (bIsStringIndex)
		{
			if (element < 0 || element >= td->fieldSize)
			{
				return NULL;
			}
		}
		else
		{
			return NULL;
		}
	}

	offset = info.actual_offset;
	if (bIsStringIndex)
	{
		offset += (element * (td->fieldSizeInBytes / td->fieldSize));

		string_t idx;

		idx = *(string_t *)((uint8_t *)pEntity + offset);
		src = (idx == NULL_STRING) ? "" : STRING(idx);
	}
	else
	{
		src = (char *)((uint8_t *)pEntity + offset);
	}

	return src;
}

Vector CEntData::GetEntDataVector(edict_t *pEdict, const char *prop, int element)
{
	typedescription_t *td;
	int offset;

	CBaseEntity *pEntity = gameents->EdictToBaseEntity(pEdict);

	FIND_PROP_DATA(td);
	CHECK_SET_PROP_DATA_OFFSET();

	if (td->fieldType != FIELD_VECTOR
		&& td->fieldType != FIELD_POSITION_VECTOR)
		return Vector(0.0f);

	return *(Vector *)((uint8_t *)pEntity + offset);
}

CBaseEntity *CEntData::GetEntDataEnt(edict_t *pEdict, const char *prop, int element)
{
	typedescription_t *td;
	int offset;

	CBaseEntity *pEntity = gameents->EdictToBaseEntity(pEdict);

	FIND_PROP_DATA(td);
	CHECK_SET_PROP_DATA_OFFSET();

	PropEntType type;
	switch (td->fieldType)
	{
		case FIELD_EHANDLE:
		{
			type = PropEnt_Handle;
			break;
		}
		case FIELD_CLASSPTR:
		{
			type = PropEnt_Entity;
			break;
		}
		case FIELD_EDICT:
		{
			type = PropEnt_Edict;
			break;
		}
		default:
			return NULL;
	}

	switch (type)
	{
		case PropEnt_Handle:
		{
			CBaseHandle &hndl = *(CBaseHandle *)((uint8_t *)pEntity + offset);
			CBaseEntity *pHandleEntity = gamehelpers->ReferenceToEntity(hndl.GetEntryIndex());

			if (!pHandleEntity || hndl != reinterpret_cast<IHandleEntity *>(pHandleEntity)->GetRefEHandle())
				return NULL;

			return pHandleEntity;
		}
		case PropEnt_Entity:
		{
			CBaseEntity *pOther = *(CBaseEntity **)((uint8_t *)pEntity + offset);
			if (pOther == NULL)
				return NULL;

			return pOther;
		}
		case PropEnt_Edict:
		{
			edict_t *pEdict = *(edict_t **)((uint8_t *)pEntity + offset);
			if (!pEdict || pEdict->IsFree())
				return NULL;

			return gameents->EdictToBaseEntity(pEdict);
		}
	}

	return NULL;
}
