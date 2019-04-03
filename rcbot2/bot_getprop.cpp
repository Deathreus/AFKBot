
#include "engine_wrappers.h"
#include "bot_const.h"
#include "bot_globals.h"
#include "bot_getprop.h"

#ifdef GetClassName
 #undef GetClassName
#endif


CClassInterfaceValue CClassInterface::g_GetProps[GET_PROPDATA_MAX];
bool CClassInterfaceValue::m_berror = false;

extern IServerGameEnts *gameents;
extern IServerTools *servertools;


void CClassInterfaceValue::Init(char *key, char *value, unsigned int preoffset)
{
	m_class = CStrings::GetString(key);
	m_value = CStrings::GetString(value);
	m_data = NULL;
	m_preoffset = preoffset;
	m_offset = 0;
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

void CClassInterfaceValue::GetData(void *edict, bool bIsEdict)
{
	static IServerUnknown *pUnknown;
	static CBaseEntity *pEntity;

	if(!m_offset || (edict == NULL))
	{
		m_data = NULL;
		m_berror = true;
		return;
	}

	if(bIsEdict)
	{
		edict_t *pEdict = reinterpret_cast<edict_t*>(edict);

		pUnknown = (IServerUnknown *)pEdict->GetUnknown();

		if(!pUnknown)
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
	else smutils->LogError(myself, "Warning: Couldn't find class %s", m_class);
#endif

	if (m_offset > 0)
		m_offset += m_preoffset;
#ifdef _DEBUG	
	else smutils->LogError(myself, "Warning: Couldn't find getprop %s for class %s", m_value, m_class);
#endif
}
/* Find and save all offsets at load to save CPU */
void CClassInterface::Init()
{
	//	DEFINE_GETPROP			ID						Class			Variable	Offset
	DEFINE_GETPROP(GETPROP_TEAM, "CBaseEntity", "m_iTeamNum", 0);
	DEFINE_GETPROP(GETPROP_SIMULATIONTIME, "CBaseEntity", "m_flSimulationTime", 0);
	DEFINE_GETPROP(GETPROP_EFFECTS, "CBaseEntity", "m_fEffects", 0);
	DEFINE_GETPROP(GETPROP_ENTITYFLAGS, "CBasePlayer", "m_fFlags", 0);
	DEFINE_GETPROP(GETPROP_ENTOWNER, "CBaseEntity", "m_hOwnerEntity", 0);
	DEFINE_GETPROP(GETPROP_PLAYERHEALTH, "CBasePlayer", "m_iHealth", 0);
	DEFINE_GETPROP(GETPROP_AMMO, "CBasePlayer", "m_iAmmo", 0);
	DEFINE_GETPROP(GETPROP_VELOCITY, "CBasePlayer", "m_vecVelocity[0]", 0);
	DEFINE_GETPROP(GETPROP_MAXSPEED, "CBasePlayer", "m_flMaxspeed", 0);
	DEFINE_GETPROP(GETPROP_GROUND_ENTITY, "CBasePlayer", "m_hGroundEntity", 0);
	DEFINE_GETPROP(GETPROP_ORIGIN, "CBasePlayer", "m_vecOrigin", 0);
	DEFINE_GETPROP(GETPROP_WATERLEVEL, "CBasePlayer", "m_nWaterLevel", 0);
	DEFINE_GETPROP(GETPROP_SEQUENCE, "CBaseAnimating", "m_nSequence", 0);
	DEFINE_GETPROP(GETPROP_CYCLE, "CBaseAnimating", "m_flCycle", 0);
	DEFINE_GETPROP(GETPROP_WEAPONLIST, "CBaseCombatCharacter", "m_hMyWeapons", 0);
	DEFINE_GETPROP(GETPROP_CURRENTWEAPON, "CBaseCombatCharacter", "m_hActiveWeapon", 0);
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
	DEFINE_GETPROP(GETPROP_TF2_EXTRAWEARABLE, "CTFWeaponBase", "m_hExtraWearable", 0);
	DEFINE_GETPROP(GETPROP_TF2_EXTRAWEARABLEVIEWMODEL, "CTFWeaponBase", "m_hExtraWearableViewModel", 0);
	DEFINE_GETPROP(GETPROP_TF2_MEDIEVALMODE, "CTFGameRulesProxy", "m_bPlayingMedieval", 0);
	DEFINE_GETPROP(GETPROP_TF2_ROUNDSTATE, "CTFGameRulesProxy", "m_iRoundState", 0);
	DEFINE_GETPROP(GETPROP_MVMHASMINPLAYERSREADY, "CTFGameRulesProxy", "m_bHaveMinPlayersToEnableReady", 0);
	DEFINE_GETPROP(GETPROP_TF2SCORE, "CTFPlayerResource", "m_iTotalScore", 0);
	DEFINE_GETPROP(GETPROP_TF2_RAGEMETER, "CTFPlayer", "m_flRageMeter", 0);
	DEFINE_GETPROP(GETPROP_TF2_RAGEDRAINING, "CTFPlayer", "m_bRageDraining", 0);
	DEFINE_GETPROP(GETPROP_TF2_INUPGRADEZONE, "CTFPlayer", "m_bInUpgradeZone", 0);
	DEFINE_GETPROP(GETPROP_TF2_ENERGYDRINKMETER, "CTFPlayer", "m_flEnergyDrinkMeter", 0);
	DEFINE_GETPROP(GETPROP_TF2_ACTIVEWEAPON, "CTFPlayer", "m_hActiveWeapon", 0);
	DEFINE_GETPROP(GETPROP_TF2DESIREDCLASS, "CTFPlayer", "m_iDesiredPlayerClass", 0);
	DEFINE_GETPROP(GETPROP_TF2_NUMHEALERS, "CTFPlayer", "m_nNumHealers", 0);
	DEFINE_GETPROP(GETPROP_TF2CLASS, "CTFPlayer", "m_iClass", 0);
	DEFINE_GETPROP(GETPROP_TF2SPYMETER, "CTFPlayer", "m_flCloakMeter", 0);
	DEFINE_GETPROP(GETPROP_TF2_PCTINVISIBLE, "CTFPlayer", "m_flInvisChangeCompleteTime", -8);
	DEFINE_GETPROP(GETPROP_TF2SPYDISGUISED_TEAM, "CTFPlayer", "m_nDisguiseTeam", 0);
	DEFINE_GETPROP(GETPROP_TF2SPYDISGUISED_CLASS, "CTFPlayer", "m_nDisguiseClass", 0);
	DEFINE_GETPROP(GETPROP_TF2SPYDISGUISED_TARGET_INDEX, "CTFPlayer", "m_iDisguiseTargetIndex", 0);
	DEFINE_GETPROP(GETPROP_TF2SPYDISGUISED_DIS_HEALTH, "CTFPlayer", "m_iDisguiseHealth", 0);
	DEFINE_GETPROP(GETPROP_TF2_TAUNTYAW, "CTFPlayer", "m_flTauntYaw", 0);
	DEFINE_GETPROP(GETPROP_TF2_HIGHFIVE, "CTFPlayer", "m_bIsReadyToHighFive", 0);
	DEFINE_GETPROP(GETPROP_TF2_HIGHFIVE_PARTNER, "CTFPlayer", "m_hHighFivePartner", 0);
	DEFINE_GETPROP(GETPROP_TF2_ISCARRYINGOBJ, "CTFPlayer", "m_bCarryingObject", 0);
	DEFINE_GETPROP(GETPROP_TF2_GETCARRIEDOBJ, "CTFPlayer", "m_hCarriedObject", 0);
	DEFINE_GETPROP(GETPROP_TF2_ATTRIBUTELIST, "CTFPlayer", "m_AttributeList", 0);
	DEFINE_GETPROP(GETPROP_CONSTRAINT_SPEED, "CTFPlayer", "m_flConstraintSpeedFactor", 0);
	DEFINE_GETPROP(GETPROP_MVMCURRENCY, "CTFPlayer", "m_nCurrency", 0);
	DEFINE_GETPROP(GETPROP_MVMINUPGZONE, "CTFPlayer", "m_bInUpgradeZone", 0);
	DEFINE_GETPROP(GETPROP_TF2_LASTDMGTYPE, "CTFPlayer", "m_flMvMLastDamageTime", 20);
	DEFINE_GETPROP(GETPROP_TF2BUILDER_TYPE, "CTFWeaponBuilder", "m_iObjectType", 0);
	DEFINE_GETPROP(GETPROP_TF2BUILDER_MODE, "CTFWeaponBuilder", "m_iObjectMode", 0);
	DEFINE_GETPROP(GETPROP_TF2MEDIGUN_HEALING, "CWeaponMedigun", "m_bHealing", 0);
	DEFINE_GETPROP(GETPROP_TF2MEDIGUN_TARGETTING, "CWeaponMedigun", "m_hHealingTarget", 0);
	DEFINE_GETPROP(GETPROP_TF2UBERCHARGE_LEVEL, "CWeaponMedigun", "m_flChargeLevel", 0);
	DEFINE_GETPROP(GETPROP_TF2MEDIGUN_RESIST_TYPE, "CWeaponMedigun", "m_nChargeResistType", 0);
	DEFINE_GETPROP(GETPROP_TF2OBJECT_HEALTH, "CBaseObject", "m_iHealth", 0);
	DEFINE_GETPROP(GETPROP_TF2OBJECT_CARRIED, "CBaseObject", "m_bCarried", 0);
	DEFINE_GETPROP(GETPROP_TF2OBJECT_UPGRADELEVEL, "CBaseObject", "m_iUpgradeLevel", 0);
	DEFINE_GETPROP(GETPROP_TF2OBJECT_UPGRADEMETAL, "CBaseObject", "m_iUpgradeMetal", 0);
	DEFINE_GETPROP(GETPROP_TF2OBJECT_MAXHEALTH, "CBaseObject", "m_iMaxHealth", 0);
	DEFINE_GETPROP(GETPROP_TF2OBJECT_PLACING, "CBaseObject", "m_bPlacing", 0);
	DEFINE_GETPROP(GETPROP_TF2OBJECT_BUILDING, "CBaseObject", "m_bBuilding", 0);
	DEFINE_GETPROP(GETPROP_TF2MINIBUILDING, "CBaseObject", "m_bMiniBuilding", 0);
	DEFINE_GETPROP(GETPROP_TF2OBJECT_MODE, "CBaseObject", "m_iObjectMode", 0);
	DEFINE_GETPROP(GETPROP_TF2OBJECT_TYPE, "CBaseObject", "m_iObjectType", 0);
	DEFINE_GETPROP(GETPROP_TF2SENTRY_ENEMY, "CObjectSentrygun", "m_hEnemy", 0);
	DEFINE_GETPROP(GETPROP_TF2SENTRY_SHELLS, "CObjectSentrygun", "m_iAmmoShells", 0);
	DEFINE_GETPROP(GETPROP_TF2SENTRY_ROCKETS, "CObjectSentrygun", "m_iAmmoRockets", 0);
	DEFINE_GETPROP(GETPROP_TF2TELEPORT_RECHARGETIME, "CObjectTeleporter", "m_flRechargeTime", 0);
	DEFINE_GETPROP(GETPROP_TF2TELEPORT_RECHARGEDURATION, "CObjectTeleporter", "m_flCurrentRechargeDuration", 0);
	DEFINE_GETPROP(GETPROP_TF2DISPENSER_METAL, "CObjectDispenser", "m_iAmmoMetal", 0);
	DEFINE_GETPROP(GETPROP_PIPEBOMB_OWNER, "CTFGrenadePipebombProjectile", "m_hThrower", 0);

	for (short int i = 0; i < GET_PROPDATA_MAX; i++)
	{
		g_GetProps[i].FindOffset();
	}
}

int CClassInterface::GetTF2Score(edict_t *edict)
{
	edict_t *res = CTeamFortress2Mod::FindResourceEntity();
	if (res != NULL)
	{
		return GetEntSend<int>(res, "m_iTotalScore", ENTINDEX(edict));
	}

	return 0;
}

bool CClassInterface::IsMoveType(edict_t *edict, byte movetype)
{
	return (GetEntData<byte>(edict, "m_MoveType") == movetype);
}

int CClassInterface::GetMoveType(edict_t *edict)
{
	return GetEntData<byte>(edict, "m_MoveType");
}


namespace SourceMod
{
#if SOURCE_ENGINE >= SE_ORANGEBOX
// sourcemod/core/HalfLife2.cpp
string_t AllocPooledString(const char *pszValue)
{
	CBaseEntity *pEntity = servertools->FirstEntity();
	datamap_t *pDataMap = gamehelpers->GetDataMap(pEntity);
	Assert( pDataMap );

	static int offset = -1;
	if(offset == -1)
	{
		sm_datatable_info_t info;
		bool found = gamehelpers->FindDataMapInfo(pDataMap, "m_iName", &info);
		Assert( found );
		offset = info.actual_offset;
	}

	string_t *pProp = (string_t *)((intptr_t)pEntity + offset);
	string_t backup = *pProp;
	servertools->SetKeyValue(pEntity, "targetname", pszValue);
	string_t newString = *pProp;
	*pProp = backup;

	return newString;
}
#endif

int MatchFieldAsInteger(int field_type)
{
	switch(field_type)
	{
		case FIELD_TICK:
		case FIELD_MODELINDEX:
		case FIELD_MATERIALINDEX:
		case FIELD_INTEGER:
		case FIELD_COLOR32:
			return 32;
		case FIELD_SHORT:
			return 16;
		case FIELD_CHARACTER:
			return 8;
		case FIELD_BOOLEAN:
			return 1;
		default:
			return 0;
	}
}

enum PropEntType
{
	PropEnt_Handle,
	PropEnt_Entity,
	PropEnt_Edict,
	PropEnt_Variant
};

int GetEntPropArraySize(edict_t *pEdict, const char *prop)
{
	CBaseEntity *pEntity = gameents->EdictToBaseEntity(pEdict);
	Assert( pEntity );

	IServerNetworkable *pNetwork = pEdict->GetNetworkable();
	if(!pNetwork)
	{
		smutils->LogError(myself, "Edict %d is not networkable", gamehelpers->EntityToReference(pEntity));
		return 0;
	}

	sm_sendprop_info_t info;
	if(!gamehelpers->FindSendPropInfo(pNetwork->GetServerClass()->GetName(), prop, &info))
	{
		smutils->LogError(myself, "Prop \"%s\" not found on edict(%d)", prop, gamehelpers->EntityToReference(pEntity));
		return 0;
	}
	
	if(info.prop->GetType() != DPT_DataTable)
	{
		smutils->LogError(myself, "Property \"%s\" is not an array.", prop);
		return 0;
	}

	SendTable *pTable = info.prop->GetDataTable();
	if(!pTable)
	{
		smutils->LogError(myself, "Error looking up DataTable for property \"%s\".", prop);
		return 0;
	}

	return pTable->GetNumProps();
}

const char *GetEntSendString(edict_t *pEdict, const char *prop, int element)
{
	CBaseEntity *pEntity = gameents->EdictToBaseEntity(pEdict);
	Assert( pEntity );

	FIND_PROP_SEND();

	if(pProp->GetType() != DPT_String)
	{
		return 0;
	}
	else if(element != 0)
	{
		return 0;
	}

	char *src;
	if(pProp->GetProxyFn())
	{
		DVariant var;
		pProp->GetProxyFn()(pProp, pEntity, (const void *)((intptr_t)pEntity + offset), &var, element, gamehelpers->EntityToReference(pEntity));
		src = var.m_pString;
	}
	else
	{
		src = (char *)((uint8_t *)pEntity + offset);
	}

	return src;
}

bool SetEntSendString(edict_t *pEdict, const char *prop, const char *value, int element)
{
	CBaseEntity *pEntity = gameents->EdictToBaseEntity(pEdict);
	Assert( pEntity );

	FIND_PROP_SEND();

	if(pProp->GetType() != DPT_String)
	{
		return false;
	}
	else if(element != 0)
	{
		return false;
	}

	bool bIsStringIndex = false;
	if(pProp->GetProxyFn())
	{
		DVariant var;
		pProp->GetProxyFn()(pProp, pEntity, (const void *)((intptr_t)pEntity + offset), &var, element, gamehelpers->EntityToReference(pEntity));
		if(var.m_pString == ((string_t *)((intptr_t)pEntity + offset))->ToCStr())
		{
			bIsStringIndex = true;
		}
	}

	// Only used if not string index.
	const int maxlen = DT_MAX_STRING_BUFFERSIZE;

	if(bIsStringIndex)
	{
	#if SOURCE_ENGINE < SE_ORANGEBOX
		smutils->LogError(myself, "Changing string_t's is not supported by this mod.  Property %s", prop);
		return false;
	#else
		*(string_t *)((intptr_t)pEntity + offset) = AllocPooledString(value);
	#endif
	}
	else
	{
		char *dest = (char *)((uint8_t *)pEntity + offset);
		ke::SafeStrcpy(dest, maxlen, value);
	}

	gamehelpers->SetEdictStateChanged(pEdict, offset);

	return true;
}

edict_t *GetEntSendEnt(edict_t *pEdict, const char *prop, int element)
{
	CBaseEntity *pEntity = gameents->EdictToBaseEntity(pEdict);
	Assert( pEntity );

	FIND_PROP_SEND();
	FIND_PROP_IF_SENDTABLE();

	if(pProp->GetType() != DPT_Int)
	{
		return 0;
	}
	else if(element != 0)
	{
		return 0;
	}

	CBaseHandle hndl = *(CBaseHandle *)((uint8_t *)pEntity + offset);
	CBaseEntity *pHandleEntity = gamehelpers->ReferenceToEntity(hndl.GetEntryIndex());

	if(!pHandleEntity || hndl != reinterpret_cast<IHandleEntity *>(pHandleEntity)->GetRefEHandle())
		return NULL;

	return gameents->BaseEntityToEdict(pHandleEntity);
}

bool SetEntSendEnt(edict_t *pEdict, const char *prop, edict_t *value, int element)
{
	CBaseEntity *pEntity = gameents->EdictToBaseEntity(pEdict);
	Assert( pEntity );

	FIND_PROP_SEND();
	FIND_PROP_IF_SENDTABLE();

	if(pProp->GetType() != DPT_Int)
	{
		return false;
	}
	else if(element != 0)
	{
		return false;
	}

	CBaseEntity *pOther = gameents->EdictToBaseEntity(value);
	Assert( pOther );
	if(pOther)
	{
		CBaseHandle *hndl = (CBaseHandle *)((uint8_t *)pEntity + offset);
		hndl->Set((IHandleEntity *)pOther);

		if(pEdict != NULL)
		{
			gamehelpers->SetEdictStateChanged(pEdict, offset);
		}

		return true;
	}

	return false;
}

const char *GetEntDataString(edict_t *pEdict, const char *prop, int element)
{
	CBaseEntity *pEntity = gameents->EdictToBaseEntity(pEdict);
	Assert( pEntity );

	FIND_PROP_DATA();

	if(td->fieldType != FIELD_CHARACTER
	   && td->fieldType != FIELD_STRING
	   && td->fieldType != FIELD_MODELNAME
	   && td->fieldType != FIELD_SOUNDNAME)
	{
		return NULL;
	}

	bool bIsStringIndex = (td->fieldType != FIELD_CHARACTER);

	if(element != 0)
	{
		if(bIsStringIndex)
		{
			if(element < 0 || element >= td->fieldSize)
			{
				return NULL;
			}
		}
		else
		{
			return NULL;
		}
	}

	int offset = info.actual_offset;

	char *src;
	if(bIsStringIndex)
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

bool SetEntDataString(edict_t *pEdict, const char *prop, const char *value, int element)
{
	CBaseEntity *pEntity = gameents->EdictToBaseEntity(pEdict);
	Assert( pEntity );

	FIND_PROP_DATA();

	if(td->fieldType != FIELD_CHARACTER
	   && td->fieldType != FIELD_STRING
	   && td->fieldType != FIELD_MODELNAME
	   && td->fieldType != FIELD_SOUNDNAME)
	{
		return false;
	}

	bool bIsStringIndex = (td->fieldType != FIELD_CHARACTER);

	if(element != 0)
	{
		if(bIsStringIndex)
		{
			if(element < 0 || element >= td->fieldSize)
			{
				return false;
			}
		}
		else
		{
			return false;
		}
	}

	int offset = info.actual_offset;

	int maxlen;
	if(bIsStringIndex)
	{
		offset += (element * (td->fieldSizeInBytes / td->fieldSize));
	}
	else
	{
		maxlen = td->fieldSize;
	}

	SET_TYPE_IF_VARIANT(FIELD_STRING);

	if(bIsStringIndex)
	{
	#if SOURCE_ENGINE < SE_ORANGEBOX
		smutils->LogError(myself, "Changing string_t's is not supported by this mod.  Property %s", prop);
		return false;
	#else
		*(string_t *)((intptr_t)pEntity + offset) = AllocPooledString(value);
	#endif
	}
	else
	{
		char *dest = (char *)((uint8_t *)pEntity + offset);
		ke::SafeStrcpy(dest, maxlen, value);
	}

	return true;
}

edict_t *GetEntDataEnt(edict_t *pEdict, const char *prop, int element)
{
	CBaseEntity *pEntity = gameents->EdictToBaseEntity(pEdict);
	Assert( pEntity );

	FIND_PROP_DATA();
	CHECK_SET_PROP_DATA_OFFSET();

	PropEntType type;
	switch(td->fieldType)
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
		case FIELD_CUSTOM:
		{
			if((td->flags & FTYPEDESC_OUTPUT) == FTYPEDESC_OUTPUT)
			{
				type = PropEnt_Variant;
				break;
			}
		}
		default:
			return NULL;
	}

	CHECK_TYPE_VALID_IF_VARIANT(FIELD_EHANDLE, "ehandle");

	switch(type)
	{
		case PropEnt_Handle:
		case PropEnt_Variant:
		{
			CBaseHandle hndl;
			if(type == PropEnt_Handle)
			{
				hndl = *(CBaseHandle *)((uint8_t *)pEntity + offset);
			}
			else
			{
				variant_t *pVariant = (variant_t *)((uint8_t *)pEntity + offset);
				hndl = pVariant->eVal;
			}

			CBaseEntity *pHandleEntity = gamehelpers->ReferenceToEntity(hndl.GetEntryIndex());

			if(!pHandleEntity || hndl != reinterpret_cast<IHandleEntity *>(pHandleEntity)->GetRefEHandle())
				return NULL;

			return gameents->BaseEntityToEdict(pHandleEntity);
		}
		case PropEnt_Entity:
		{
			CBaseEntity *pOther = *(CBaseEntity **)((uint8_t *)pEntity + offset);
			if(pOther == NULL)
				return NULL;

			return gameents->BaseEntityToEdict(pOther);
		}
		case PropEnt_Edict:
		{
			edict_t *pOther = *(edict_t **)((uint8_t *)pEntity + offset);
			if(!pOther || pOther->IsFree())
				return NULL;

			return pOther;
		}
	}

	return NULL;
}

bool SetEntDataEnt(edict_t *pEdict, const char *prop, edict_t *value, int element)
{
	CBaseEntity *pEntity = gameents->EdictToBaseEntity(pEdict);
	Assert( pEntity );

	FIND_PROP_DATA();
	CHECK_SET_PROP_DATA_OFFSET();

	PropEntType type;
	switch(td->fieldType)
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
		case FIELD_CUSTOM:
		{
			if((td->flags & FTYPEDESC_OUTPUT) == FTYPEDESC_OUTPUT)
			{
				type = PropEnt_Variant;
				break;
			}
		}
		default:
			return false;
	}

	SET_TYPE_IF_VARIANT(FIELD_EHANDLE);

	CBaseEntity *pOther = gameents->EdictToBaseEntity(value);
	Assert( pOther );
	if(pOther)
	{
		switch(type)
		{
			case PropEnt_Handle:
			case PropEnt_Variant:
			{
				CBaseHandle *hndl;
				if(type == PropEnt_Handle)
				{
					hndl = (CBaseHandle *)((uint8_t *)pEntity + offset);
				}
				else
				{
					variant_t *pVariant = (variant_t *)((uint8_t *)pEntity + offset);
					hndl = &pVariant->eVal;
				}

				if(hndl)
				{
					hndl->Set((IHandleEntity *)pOther);
					return true;
				}

				break;
			}
			case PropEnt_Entity:
			{
				*(CBaseEntity **)((uint8_t *)pEntity + offset) = pOther;
				return true;
			}
			case PropEnt_Edict:
			{
				*(edict_t **)((uint8_t *)pEntity + offset) = value;
				return true;
			}
		}
	}

	return false;
}

};