#ifndef __RCBOT_GETPROP_H__
#define __RCBOT_GETPROP_H__

#include "bot_gamerules.h"
#include "server_class.h"

class CTFObjectiveResource;
class CTeamRoundTimer;
class CAttributeList;

typedef enum
{
	TELE_ENTRANCE = 0,
	TELE_EXIT
}eTeleMode;

typedef enum
{
	GETPROP_UNDEF = -1,
	GETPROP_TEAM = 0,
	GETPROP_PLAYERHEALTH,
	GETPROP_EFFECTS,
	GETPROP_AMMO,
	GETPROP_VELOCITY,
	GETPROP_CURRENTWEAPON,
	GETPROP_MAXSPEED,
	GETPROP_CONSTRAINT_SPEED,
	GETPROP_WEAPONLIST,
	GETPROP_WEAPONSTATE,
	GETPROP_WEAPONCLIP1,
	GETPROP_WEAPONCLIP2,
	GETPROP_WEAPON_AMMOTYPE1,
	GETPROP_WEAPON_AMMOTYPE2,
	GETPROP_SIMULATIONTIME,
	GETPROP_SEQUENCE,
	GETPROP_CYCLE,
	GETPROP_ENTITYFLAGS,
	GETPROP_MOVETYPE,
	GETPROP_ENTOWNER,
	GETPROP_GROUND_ENTITY,
	GETPROP_ORIGIN,
	GETPROP_WATERLEVEL,
	GETPROP_SENTRY_ENEMY,
	GETPROP_TF2OBJECTSHELLS,
	GETPROP_TF2OBJECTROCKETS,
	GETPROP_TF2SCORE,
	GETPROP_TF2_NUMHEALERS,
	GETPROP_TF2_CONDITIONS,
	GETPROP_TF2CLASS,
	GETPROP_TF2SPYMETER,
	GETPROP_TF2SPYDISGUISED_TEAM,
	GETPROP_TF2SPYDISGUISED_CLASS,
	GETPROP_TF2SPYDISGUISED_TARGET_INDEX,
	GETPROP_TF2SPYDISGUISED_DIS_HEALTH,
	GETPROP_TF2MEDIGUN_HEALING,
	GETPROP_TF2MEDIGUN_TARGETTING,
	GETPROP_TF2TELEPORTERMODE,
	GETPROP_TF2UBERCHARGE_LEVEL,
	GETPROP_TF2SENTRYHEALTH,
	GETPROP_TF2DISPENSERHEALTH,
	GETPROP_TF2TELEPORTERHEALTH,
	GETPROP_TF2OBJECTCARRIED,
	GETPROP_TF2OBJECTUPGRADELEVEL,
	GETPROP_TF2OBJECTUPGRADEMETAL,
	GETPROP_TF2OBJECTMAXHEALTH,
	GETPROP_TF2DISPMETAL,
	GETPROP_TF2MINIBUILDING,
	GETPROP_TF2OBJECTBUILDING,
	GETPROP_TF2_TELEPORT_RECHARGETIME,
	GETPROP_TF2_TELEPORT_RECHARGEDURATION,
	GETPROP_TF2_OBJTR_m_vCPPositions,
	GETPROP_TF2_OBJTR_m_bCPIsVisible,
	GETPROP_TF2_OBJTR_m_iTeamIcons,
	GETPROP_TF2_OBJTR_m_iTeamOverlays,
	GETPROP_TF2_OBJTR_m_iTeamReqCappers,
	GETPROP_TF2_OBJTR_m_flTeamCapTime,
	GETPROP_TF2_OBJTR_m_iPreviousPoints,
	GETPROP_TF2_OBJTR_m_bTeamCanCap,
	GETPROP_TF2_OBJTR_m_iTeamBaseIcons,
	GETPROP_TF2_OBJTR_m_iBaseControlPoints,
	GETPROP_TF2_OBJTR_m_bInMiniRound,
	GETPROP_TF2_OBJTR_m_iWarnOnCap,
	GETPROP_TF2_OBJTR_m_iCPGroup,
	GETPROP_TF2_OBJTR_m_bCPLocked,
	GETPROP_TF2_OBJTR_m_bTrackAlarm,
	GETPROP_TF2_OBJTR_m_flUnlockTimes,
	GETPROP_TF2_OBJTR_m_flCPTimerTimes,
	GETPROP_TF2_OBJTR_m_iNumTeamMembers,
	GETPROP_TF2_OBJTR_m_iCappingTeam,
	GETPROP_TF2_OBJTR_m_iTeamInZone,
	GETPROP_TF2_OBJTR_m_bBlocked,
	GETPROP_TF2_OBJTR_m_iOwner,
	GETPROP_TF2_OBJTR_m_bCPCapRateScalesWithPlayers,
	GETPROP_TF2_OBJTR_m_iNumControlPoints,
	GETPROP_TF2_OBJTR_m_bPlayingMiniRounds,
	GETPROP_TF2_RNDTM_m_flTimerEndTime,
	GETPROP_TF2_RNDTM_m_nSetupTimeLength,
	GETPROP_PIPEBOMB_OWNER,
	GETPROP_TF2_TAUNTYAW,
	GETPROP_TF2_HIGHFIVE,
	GETPROP_TF2_HIGHFIVE_PARTNER,
	GETPROP_SENTRYGUN_PLACING,
	GETPROP_TF2_ISCARRYINGOBJ,
	GETPROP_TF2_GETCARRIEDOBJ,
	GETPROP_TF2_ATTRIBUTELIST,
	GETPROP_TF2_ITEMDEFINITIONINDEX,
	GETPROP_TF2_DISGUISEWEARABLE,
	GETPROP_TF2_ENTITYLEVEL,
	GETPROP_TF2_RAGEMETER,
	GETPROP_TF2_RAGEDRAINING,
	GETPROP_TF2_ENTITYQUALITY,
	GETPROP_TF2_WEAPON_INITIALIZED,
	GETPROP_TF2_INUPGRADEZONE,
	GETPROP_TF2_EXTRAWEARABLE,
	GETPROP_TF2_EXTRAWEARABLEVIEWMODEL,
	GETPROP_TF2_ENERGYDRINKMETER,
	GETPROP_TF2_MEDIEVALMODE,
	GETPROP_TF2_ACTIVEWEAPON,
	GETPROP_TF2_BUILDER_TYPE,
	GETPROP_TF2_BUILDER_MODE,
	GETPROP_TF2_CHARGE_RESIST_TYPE,
	GETPROP_TF2_ROUNDSTATE,
	GETPROP_TF2DESIREDCLASS,
	GETPROP_MVMHASMINPLAYERSREADY,
	GET_PROPDATA_MAX
}getpropdata_id;

class CClassInterfaceValue
{
public:
	CClassInterfaceValue()
	{
		m_data = NULL;
		m_class = NULL;
		m_value = NULL;
		m_offset = 0;
	}

	CClassInterfaceValue(char *key, char *value, unsigned int preoffset)
	{
		Init(key, value, preoffset);
	}

	void Init(char *key, char *value, unsigned int preoffset = 0);

	void FindOffset();

	void GetData(void *edict, bool bIsEdict = true);

	edict_t *GetEntity(edict_t *edict);

	CBaseHandle *GetEntityHandle(edict_t *edict);

	inline bool GetBool(void *edict, bool defaultvalue, bool bIsEdict = true)
	{
		GetData(edict, bIsEdict);

		if (!m_data)
			return defaultvalue;

		try
		{
			return *((bool*)m_data);
		}

		catch (...)
		{
			return defaultvalue;
		}
	}

	inline bool *GetBoolPointer(edict_t *edict)
	{
		GetData(edict);

		if (!m_data)
			return NULL;

		return ((bool*)m_data);
	}

	inline void *GetVoidPointer(edict_t *edict)
	{
		GetData(edict);

		if (!m_data)
			return NULL;

		return m_data;
	}

	inline float GetFloat(edict_t *edict, float defaultvalue)
	{
		GetData(edict);

		if (!m_data)
			return defaultvalue;

		return *((float*)m_data);
	}

	inline float *GetFloatPointer(edict_t *edict)
	{
		GetData(edict);

		if (!m_data)
			return NULL;

		return ((float*)m_data);
	}

	inline char *GetString(edict_t *edict)
	{
		GetData(edict);

		return (char*)m_data;
	}

	inline bool GetVector(edict_t *edict, Vector *v)
	{
		static float *x;

		GetData(edict);

		if (m_data)
		{
			x = (float*)m_data;
			*v = Vector(*x, *(x + 1), *(x + 2));

			return true;
		}

		return false;
	}

	inline Vector *GetVectorPointer(edict_t *edict)
	{
		GetData(edict);

		if (m_data)
		{
			return (Vector*)m_data;
		}

		return NULL;
	}

	inline int GetInt(void *edict, int defaultvalue, bool bIsEdict = true)
	{
		GetData(edict, bIsEdict);

		if (!m_data)
			return defaultvalue;

		try
		{
			return *((int*)m_data);
		}

		catch (...)
		{
			return defaultvalue;
		}
	}

	inline int *GetIntPointer(edict_t *edict)
	{
		GetData(edict);

		return (int*)m_data;
	}

	inline byte *GetBytePointer(edict_t *edict)
	{
		GetData(edict);

		return (byte*)m_data;
	}

	inline float GetFloatFromInt(edict_t *edict, float defaultvalue)
	{
		GetData(edict);

		if (!m_data)
			return defaultvalue;

		return (float)(*(int *)m_data);
	}

	static void ResetError() { m_berror = false; }
	static bool IsError() { return m_berror; }

	int GetOffset()
	{
		return m_offset;
	}
private:
	unsigned int m_offset;
	unsigned int m_preoffset;
	void *m_data;
	char *m_class;
	char *m_value;

	static bool m_berror;
};


extern CClassInterfaceValue g_GetProps[GET_PROPDATA_MAX];

#define DEFINE_GETPROP(id, classname, value, preoffs) \
 g_GetProps[id] = CClassInterfaceValue( CClassInterfaceValue ( classname, value, preoffs ) )

class CClassInterface
{
public:
	static void Init();

	static edict_t *FindEntityByNetClass(int start, const char *classname);
	static edict_t *FindEntityByNetClassNearest(Vector vstart, const char *classname, float fMinDist = 8192.0f);
	static edict_t *FindEntityByClassname(int start, const char *classname);
	static edict_t *FindEntityByClassnameNearest(Vector vstart, const char *classname, float fMinDist = 8192.0f, edict_t *pOwner = NULL);

	// TF2
	static int GetTF2Score(edict_t *edict);

	static void SetupCTeamRoundTimer(CTeamRoundTimer *pTimer);

	inline static float GetRageMeter(edict_t *edict) { return g_GetProps[GETPROP_TF2_RAGEMETER].GetFloat(edict, 0); }

	inline static int GetTeam(edict_t *edict) { return g_GetProps[GETPROP_TEAM].GetInt(edict, 0); }

	inline static float GetPlayerHealth(edict_t *edict) { return g_GetProps[GETPROP_PLAYERHEALTH].GetFloatFromInt(edict, 0); }

	inline static int GetEffects(edict_t *edict) { return g_GetProps[GETPROP_EFFECTS].GetInt(edict, 0); }

	inline static int *GetAmmoList(edict_t *edict) { return g_GetProps[GETPROP_AMMO].GetIntPointer(edict); }

	inline static int GetTF2NumHealers(edict_t *edict) { return g_GetProps[GETPROP_TF2_NUMHEALERS].GetInt(edict, 0); }

	inline static int GetTF2Conditions(edict_t *edict) { return g_GetProps[GETPROP_TF2_CONDITIONS].GetInt(edict, 0); }

	inline static bool GetVelocity(edict_t *edict, Vector *v) { return g_GetProps[GETPROP_VELOCITY].GetVector(edict, v); }

	inline static int GetTF2Class(edict_t *edict) { return g_GetProps[GETPROP_TF2CLASS].GetInt(edict, 0); }

	inline static edict_t *GetExtraWearable(edict_t *edict) { return g_GetProps[GETPROP_TF2_EXTRAWEARABLE].GetEntity(edict); }

	inline static edict_t *GetExtraWearableViewModel(edict_t *edict) { return g_GetProps[GETPROP_TF2_EXTRAWEARABLEVIEWMODEL].GetEntity(edict); }

	inline static float TF2_GetEnergyDrinkMeter(edict_t * edict) { return g_GetProps[GETPROP_TF2_ENERGYDRINKMETER].GetFloat(edict, 0); }

	inline static void SetInitialized(edict_t *edict)
	{
		bool *m_bInitialized = g_GetProps[GETPROP_TF2_WEAPON_INITIALIZED].GetBoolPointer(edict);

		*m_bInitialized = true;
	}

	inline static edict_t *TF2_GetActiveWeapon(edict_t *edict) { return g_GetProps[GETPROP_TF2_ACTIVEWEAPON].GetEntity(edict); }
	
	static bool TF2_SetActiveWeapon(edict_t *edict, edict_t *pWeapon)
	{
		CBaseHandle *pHandle = g_GetProps[GETPROP_TF2_ACTIVEWEAPON].GetEntityHandle(edict);
		pHandle->Set(pWeapon->GetNetworkable()->GetEntityHandle());
	}

	inline static void TF2_SetBuilderType(edict_t *pBuilder, int itype)
	{
		int *pitype = g_GetProps[GETPROP_TF2_BUILDER_TYPE].GetIntPointer(pBuilder);

		*pitype = itype;
	}

	inline static int GetChargeResistType(edict_t *pMedigun)
	{
		return g_GetProps[GETPROP_TF2_CHARGE_RESIST_TYPE].GetInt(pMedigun, 0);
	}

	inline static void TF2_SetBuilderMode(edict_t *pBuilder, int imode)
	{
		int *pitype = g_GetProps[GETPROP_TF2_BUILDER_MODE].GetIntPointer(pBuilder);

		*pitype = imode;
	}
	
	inline static int GetTF2DesiredClass(edict_t *edict) { return g_GetProps[GETPROP_TF2DESIREDCLASS].GetInt(edict, 0); }

	inline static void SetTF2Class(edict_t *edict, int _class)
	{
		int* p = g_GetProps[GETPROP_TF2DESIREDCLASS].GetIntPointer(edict);
		if (p != NULL) *p = _class;
	}
	
	inline static float GetTF2SpyCloakMeter(edict_t *edict) { return g_GetProps[GETPROP_TF2SPYMETER].GetFloat(edict, 0); }

	inline static int GetWaterLevel(edict_t *edict) { return g_GetProps[GETPROP_WATERLEVEL].GetInt(edict, 0); }

	inline static void UpdateSimulationTime(edict_t *edict)
	{
		float *m_flSimulationTime = g_GetProps[GETPROP_SIMULATIONTIME].GetFloatPointer(edict);

		if (m_flSimulationTime)
			*m_flSimulationTime = gpGlobals->curtime;
	}

	static bool GetTF2SpyDisguised(edict_t *edict, int *_class, int *_team, int *_index, int *_health)
	{
		CClassInterfaceValue::ResetError();
		if (_team)
			*_team = g_GetProps[GETPROP_TF2SPYDISGUISED_TEAM].GetInt(edict, 0);

		if (_class)
			*_class = g_GetProps[GETPROP_TF2SPYDISGUISED_CLASS].GetInt(edict, 0);

		if (_index)
			*_index = g_GetProps[GETPROP_TF2SPYDISGUISED_TARGET_INDEX].GetInt(edict, 0);

		if (_health)
			*_health = g_GetProps[GETPROP_TF2SPYDISGUISED_DIS_HEALTH].GetInt(edict, 0);

		return !CClassInterfaceValue::IsError();
	}

	inline static void SetEntityIndex_Level_Quality(edict_t *edict, int iIndex, int iLevel = 0, int iQuality = 0)
	{
		int *pdata = g_GetProps[GETPROP_TF2_ITEMDEFINITIONINDEX].GetIntPointer(edict);

		if (pdata)
			*pdata = iIndex;

		if (iLevel)
		{
			int *pdata = g_GetProps[GETPROP_TF2_ENTITYLEVEL].GetIntPointer(edict);

			if (pdata)
				*pdata = iLevel;
		}
		if (iQuality)
		{
			int *pdata = g_GetProps[GETPROP_TF2_ENTITYQUALITY].GetIntPointer(edict);

			if (pdata)
				*pdata = iQuality;
		}
	}

	inline static bool IsCarryingObj(edict_t *edict) { return g_GetProps[GETPROP_TF2_ISCARRYINGOBJ].GetBool(edict, false); }

	inline static edict_t *GetCarriedObj(edict_t *edict) { return g_GetProps[GETPROP_TF2_GETCARRIEDOBJ].GetEntity(edict); }

	inline static bool GetMedigunHealing(edict_t *edict) { return g_GetProps[GETPROP_TF2MEDIGUN_HEALING].GetBool(edict, false); }

	inline static edict_t *GetMedigunTarget(edict_t *edict) { return g_GetProps[GETPROP_TF2MEDIGUN_TARGETTING].GetEntity(edict); }

	inline static edict_t *GetSentryEnemy(edict_t *edict) { return g_GetProps[GETPROP_SENTRY_ENEMY].GetEntity(edict); }

	inline static edict_t *GetOwner(edict_t *edict) { return g_GetProps[GETPROP_ENTOWNER].GetEntity(edict); }

	inline static bool IsMedigunTargetting(edict_t *pgun, edict_t *ptarget) { return (g_GetProps[GETPROP_TF2MEDIGUN_TARGETTING].GetEntity(pgun) == ptarget); }

	//static void SetTickBase (edict_t *edict, int tickbase) { return ; }

	inline static int IsTeleporterMode(edict_t *edict, eTeleMode mode) { return (g_GetProps[GETPROP_TF2TELEPORTERMODE].GetInt(edict, -1) == (int)mode); }

	inline static edict_t *GetCurrentWeapon(edict_t *player) { return g_GetProps[GETPROP_CURRENTWEAPON].GetEntity(player); }

	inline static int GetUberChargeLevel(edict_t *pWeapon) { return (int)(g_GetProps[GETPROP_TF2UBERCHARGE_LEVEL].GetFloat(pWeapon, 0)*100.0); }

	inline static float GetSentryHealth(edict_t *edict) { return g_GetProps[GETPROP_TF2SENTRYHEALTH].GetFloatFromInt(edict, 100); }

	inline static float GetDispenserHealth(edict_t *edict) { return g_GetProps[GETPROP_TF2DISPENSERHEALTH].GetFloatFromInt(edict, 100); }

	inline static float GetTeleporterHealth(edict_t *edict) { return g_GetProps[GETPROP_TF2TELEPORTERHEALTH].GetFloatFromInt(edict, 100); }

	inline static bool IsObjectCarried(edict_t *edict) { return g_GetProps[GETPROP_TF2OBJECTCARRIED].GetBool(edict, false); }

	inline static int GetTF2UpgradeLevel(edict_t *edict) { return g_GetProps[GETPROP_TF2OBJECTUPGRADELEVEL].GetInt(edict, 0); }

	inline static int GetTF2SentryUpgradeMetal(edict_t *edict) { return g_GetProps[GETPROP_TF2OBJECTUPGRADEMETAL].GetInt(edict, 0); }
	inline static int GetTF2SentryShells(edict_t *edict) { return g_GetProps[GETPROP_TF2OBJECTSHELLS].GetInt(edict, 0); }

	inline static int GetTF2SentryRockets(edict_t *edict) { return g_GetProps[GETPROP_TF2OBJECTROCKETS].GetInt(edict, 0); }

	static bool GetTF2ObjectiveResource(CTFObjectiveResource *pResource);

	inline static float GetTF2TeleRechargeTime(edict_t *edict) { return g_GetProps[GETPROP_TF2_TELEPORT_RECHARGETIME].GetFloat(edict, 0); }

	inline static float GetTF2TeleRechargeDuration(edict_t *edict) { return g_GetProps[GETPROP_TF2_TELEPORT_RECHARGEDURATION].GetFloat(edict, 0); }

	inline static int GetTF2GetBuildingMaxHealth(edict_t *edict) { return g_GetProps[GETPROP_TF2OBJECTMAXHEALTH].GetInt(edict, 0); }

	inline static int GetTF2DispMetal(edict_t *edict) { return g_GetProps[GETPROP_TF2DISPMETAL].GetInt(edict, 0); }

	inline static bool GetTF2BuildingIsMini(edict_t *edict) { return g_GetProps[GETPROP_TF2MINIBUILDING].GetBool(edict, false); }

	inline static float GetMaxSpeed(edict_t *edict) { return g_GetProps[GETPROP_MAXSPEED].GetFloat(edict, 0); }

	inline static float GetSpeedFactor(edict_t *edict) { return g_GetProps[GETPROP_CONSTRAINT_SPEED].GetFloat(edict, 0); }

	inline static bool IsObjectBeingBuilt(edict_t *edict) { return g_GetProps[GETPROP_TF2OBJECTBUILDING].GetBool(edict, false); }

	inline static edict_t *GetGroundEntity(edict_t *edict) { return g_GetProps[GETPROP_GROUND_ENTITY].GetEntity(edict); }

	inline static CBaseHandle *GetWeaponList(edict_t *player) { return g_GetProps[GETPROP_WEAPONLIST].GetEntityHandle(player); }
	inline static int GetWeaponState(edict_t *pgun) { return g_GetProps[GETPROP_WEAPONSTATE].GetInt(pgun, 0); }

	inline static edict_t *GetPipeBombOwner(edict_t *pPipeBomb) { return g_GetProps[GETPROP_PIPEBOMB_OWNER].GetEntity(pPipeBomb); }

	inline static int *GetWeaponClip1Pointer(edict_t *pgun) { return g_GetProps[GETPROP_WEAPONCLIP1].GetIntPointer(pgun); }

	inline static int *GetWeaponClip2Pointer(edict_t *pgun) { return g_GetProps[GETPROP_WEAPONCLIP2].GetIntPointer(pgun); }

	inline static CAttributeList *GetAttributeList(edict_t *player) { return (CAttributeList*)g_GetProps[GETPROP_TF2_ATTRIBUTELIST].GetVoidPointer(player); }

	inline static int GetOffset(int id) { return g_GetProps[id].GetOffset(); }

	inline static void GetWeaponClip(edict_t *pgun, int *iClip1, int *iClip2) { *iClip1 = g_GetProps[GETPROP_WEAPONCLIP1].GetInt(pgun, 0); *iClip2 = g_GetProps[GETPROP_WEAPONCLIP2].GetInt(pgun, 0); }

	inline static void GetAmmoTypes(edict_t *pgun, int *iAmmoType1, int *iAmmoType2) { *iAmmoType1 = g_GetProps[GETPROP_WEAPON_AMMOTYPE1].GetInt(pgun, -1); *iAmmoType2 = g_GetProps[GETPROP_WEAPON_AMMOTYPE2].GetInt(pgun, -1); }

	inline static float GetAnimCycle(edict_t *edict)
	{
		return g_GetProps[GETPROP_CYCLE].GetFloat(edict, 0);
	}

	inline static void GetAnimatingInfo(edict_t *edict, float *flCycle, int *iSequence)
	{
		*flCycle = g_GetProps[GETPROP_CYCLE].GetFloat(edict, 0);
		*iSequence = g_GetProps[GETPROP_SEQUENCE].GetInt(edict, false);
	}

	inline static int GetPlayerFlags(edict_t *player) { return g_GetProps[GETPROP_ENTITYFLAGS].GetInt(player, 0); }

	inline static int *GetPlayerFlagsPointer(edict_t *player) { return g_GetProps[GETPROP_ENTITYFLAGS].GetIntPointer(player); }

	inline static Vector *GetOrigin(edict_t *pPlayer)
	{
		return g_GetProps[GETPROP_ORIGIN].GetVectorPointer(pPlayer);
	}

	inline static void SetOrigin(edict_t *pPlayer, Vector vOrigin)
	{
		Vector *vEntOrigin = g_GetProps[GETPROP_ORIGIN].GetVectorPointer(pPlayer);

		*vEntOrigin = vOrigin;
	}

	inline static float GetTF2TauntYaw(edict_t *edict) { return g_GetProps[GETPROP_TF2_TAUNTYAW].GetFloat(edict, 0); }
	inline static bool GetTF2HighFiveReady(edict_t *edict) { return g_GetProps[GETPROP_TF2_HIGHFIVE].GetBool(edict, false); }
	inline static edict_t *GetHighFivePartner(edict_t *edict) { return g_GetProps[GETPROP_TF2_HIGHFIVE_PARTNER].GetEntity(edict); }

	inline static bool IsMoveType(edict_t *pent, int movetype)
	{
		return ((g_GetProps[GETPROP_MOVETYPE].GetInt(pent, 0) & 15) == movetype);
	}

	inline static int GetMoveType(edict_t *pent)
	{
		return (g_GetProps[GETPROP_MOVETYPE].GetInt(pent, 0) & 15);
	}

	inline static byte *GetMoveTypePointer(edict_t *pent)
	{
		return (g_GetProps[GETPROP_MOVETYPE].GetBytePointer(pent));
	}

	inline static bool IsSentryGunBeingPlaced(edict_t *pSentry)
	{
		return g_GetProps[GETPROP_SENTRYGUN_PLACING].GetBool(pSentry, false);
	}

	inline static bool TF2_MVMMinPlayersToReady()
	{
		return CGameRulesObject::GameRules_GetProp("m_bHaveMinPlayersToEnableReady") == 1;
	}

	inline static bool TF2_MVMIsPlayerReady(int client)
	{
		return CGameRulesObject::GameRules_GetProp("m_bPlayerReady", 4, client) == 1;
	}

private:
	static CClassInterfaceValue g_GetProps[GET_PROPDATA_MAX];

};

namespace SourceMod {

enum PropEntType
{
	PropEnt_Handle,
	PropEnt_Entity,
	PropEnt_Edict,
};

inline int MatchFieldAsGetInteger(int field_type)
{
	switch (field_type)
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

	return 0;
}

class CEntData
{
public:
	static int32_t GetEntProp(edict_t *pEdict, const char *prop, int element = 0);
	static float GetEntPropFloat(edict_t *pEdict, const char *prop, int element = 0);
	static const char *GetEntPropString(edict_t *pEdict, const char *prop, int element = 0);
	static Vector GetEntPropVector(edict_t *pEdict, const char *prop, int element = 0);
	static CBaseEntity *GetEntPropEnt(edict_t *pEdict, const char *prop, int element = 0);

	static int32_t GetEntData(edict_t *pEdict, const char *prop, int element = 0);
	static float GetEntDataFloat(edict_t *pEdict, const char *prop, int element = 0);
	static const char *GetEntDataString(edict_t *pEdict, const char *prop, int element = 0);
	static Vector GetEntDataVector(edict_t *pEdict, const char *prop, int element = 0);
	static CBaseEntity *GetEntDataEnt(edict_t *pEdict, const char *prop, int element = 0);
};

}//namespace SourceMod

#endif
