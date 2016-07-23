#ifndef __RCBOT_GETPROP_H__
#define __RCBOT_GETPROP_H__

typedef enum
{
	TELE_ENTRANCE = 0,
	TELE_EXIT
}eTeleMode;

typedef enum
{
	GETPROP_UNDEF = -1,
	GETPROP_ENTITY_FLAGS = 0,
	GETPROP_ALL_ENTOWNER,
	GETPROP_GROUND_ENTITY,
	GETPROP_ORIGIN,
	GETPROP_TAKEDAMAGE,
	GETPROP_WATERLEVEL,
	GETPROP_SIMULATIONTIME,
	GETPROP_TEAM,
	GETPROP_PLAYERHEALTH,
	GETPROP_EFFECTS,
	GETPROP_AMMO,
	GETPROP_MOVETYPE,
	GETPROP_VELOCITY,
	GETPROP_CURRENTWEAPON,
	GETPROP_MAXSPEED,
	GETPROP_CONSTRAINT_SPEED,
	GETPROP_ENTITYFLAGS,
	GETPROP_SEQUENCE,
	GETPROP_CYCLE,
	GETPROP_HL2DM_PHYSCANNON_ATTACHED,
	GETPROP_HL2DM_PHYSCANNON_OPEN,
	GETPROP_HL2DM_PLAYER_AUXPOWER,
	GETPROP_HL2DM_LADDER_ENT,
	GETPROP_WEAPONLIST,
	GETPROP_WEAPONSTATE,
	GETPROP_WEAPONCLIP1,
	GETPROP_WEAPONCLIP2,
	GETPROP_WEAPON_AMMOTYPE1,
	GETPROP_WEAPON_AMMOTYPE2,
	GETPROP_DOD_PLAYERCLASS,
	GETPROP_DOD_DES_PLAYERCLASS,
	GETPROP_DOD_STAMINA,
	GETPROP_DOD_PRONE,
	GETPROP_DOD_CP_NUMCAPS,
	GETPROP_DOD_CP_POSITIONS,
	GETPROP_DOD_CP_ALLIES_REQ_CAP,
	GETPROP_DOD_CP_AXIS_REQ_CAP,
	GETPROP_DOD_CP_NUM_AXIS,
	GETPROP_DOD_CP_NUM_ALLIES,
	GETPROP_DOD_CP_OWNER,
	GETPROP_DOD_SNIPER_ZOOMED,
	GETPROP_DOD_MACHINEGUN_DEPLOYED,
	GETPROP_DOD_ROCKET_DEPLOYED,
	GETPROP_DOD_SEMI_AUTO,
	GETPROP_DOD_GREN_THROWER,
	GETPROP_DOD_SCORE,
	GETPROP_DOD_OBJSCORE,
	GETPROP_DOD_DEATHS,
	GETPROP_DOD_SMOKESPAWN_TIME,
	GETPROP_DOD_ROUNDTIME,
	GETPROP_DOD_K98ZOOM,
	GETPROP_DOD_GARANDZOOM,
	GETPROP_DOD_ALLIESBOMBING,
	GETPROP_DOD_AXISBOMBING,
	GETPROP_DOD_BOMBSPLANTED,
	GETPROP_DOD_BOMBSREQ,
	GETPROP_DOD_BOMBSDEFUSED,
	GETPROP_DOD_BOMBSREMAINING,
	GETPROP_DOD_PLANTINGBOMB,
	GETPROP_DOD_DEFUSINGBOMB,
	GETPROP_DOD_BOMB_STATE,
	GETPROP_DOD_BOMB_TEAM,
	GETPROP_DOD_CP_VISIBLE,
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
	GETPROP_TF2OBJECTSHELLS,
	GETPROP_TF2OBJECTROCKETS,
	GETPROP_TF2DISPMETAL,
	GETPROP_TF2OBJECTBUILDING,
	GETPROP_TF2MINIBUILDING,
	GETPROP_TF2SCORE,
	GETPROP_SENTRY_ENEMY,
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
	GETPROP_TF2_RNDTM_m_bInSetup,
	GETPROP_PIPEBOMB_OWNER,
	GETPROP_SENTRYGUN_PLACING,
	GETPROP_TF2_TAUNTYAW,
	GETPROP_TF2_HIGHFIVE,
	GETPROP_TF2_HIGHFIVE_PARTNER,
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
	GET_PROPDATA_MAX
}getpropdata_id;

bool UTIL_FindSendPropInfo(ServerClass *pInfo, const char *szType, unsigned int *offset);
ServerClass *UTIL_FindServerClass(const char *name);
void UTIL_FindServerClassPrint(const char*name_cmd);
void UTIL_FindServerClassnamePrint(const char *name_cmd);
void UTIL_FindPropPrint(const char *prop_name);

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

	inline Vector *GetVectorPointer(edict_t *edict)
	{
		GetData(edict);

		if (m_data)
		{
			return (Vector*)m_data;
		}

		return NULL;
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
class CTFObjectiveResource;
class CTeamRoundTimer;
class CAttributeList;
#define DEFINE_GETPROP(id,classname,value,preoffs)\
 g_GetProps[id] = CClassInterfaceValue( CClassInterfaceValue ( classname, value, preoffs ) )

class CClassInterface
{
public:
	static void Init();

	static const char *FindEntityNetClass(int start, const char *classname);
	static edict_t *FindEntityByNetClass(int start, const char *classname);
	static edict_t *FindEntityByNetClassNearest(Vector vstart, const char *classname);
	static edict_t *FindEntityByClassnameNearest(Vector vstart, const char *classname, float fMinDist = 8192.0f, edict_t *pOwner = NULL);

	// TF2
	static int GetTF2Score(edict_t *edict);
	static void SetupCTeamRoundTimer(CTeamRoundTimer *pTimer);
	inline static float GetRageMeter(edict_t *edict) { return g_GetProps[GETPROP_TF2_RAGEMETER].GetFloat(edict, 0); }
	inline static int GetFlags(edict_t *edict) { return g_GetProps[GETPROP_ENTITY_FLAGS].GetInt(edict, 0); }
	inline static int GetTeam(edict_t *edict) { return g_GetProps[GETPROP_TEAM].GetInt(edict, 0); }
	inline static float GetPlayerHealth(edict_t *edict) { return g_GetProps[GETPROP_PLAYERHEALTH].GetFloatFromInt(edict, 0); }
	inline static int GetEffects(edict_t *edict) { return g_GetProps[GETPROP_EFFECTS].GetInt(edict, 0); }
	inline static int *GetAmmoList(edict_t *edict) { return g_GetProps[GETPROP_AMMO].GetIntPointer(edict); }
	//static unsigned int FindOffset(const char *szType,const char *szClass);
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
	// set weapon
	static bool TF2_SetActiveWeapon(edict_t *edict, edict_t *pWeapon)
	{
		CBaseHandle *pHandle = g_GetProps[GETPROP_TF2_ACTIVEWEAPON].GetEntityHandle(edict);
		pHandle->Set(pWeapon->GetNetworkable()->GetEntityHandle());
	}
	inline static void TF2_SetBuilderType(edict_t *pBuilder, int itype)
	{
		int *pitype = g_GetProps[GETPROP_TF2_BUILDER_TYPE].GetIntPointer(pBuilder);

		*pitype = itype;
		//, ]
	}
	inline static void TF2_SetBuilderMode(edict_t *pBuilder, int imode)
	{
		int *pitype = g_GetProps[GETPROP_TF2_BUILDER_MODE].GetIntPointer(pBuilder);

		*pitype = imode;
		//GETPROP_TF2_BUILDER_MODE, ]
	}
	inline static int GetChargeResistType(edict_t *pMedigun)
	{
		return g_GetProps[GETPROP_TF2_CHARGE_RESIST_TYPE].GetInt(pMedigun, 0);
	}
	inline static int GetTF2DesiredClass(edict_t *edict) { return g_GetProps[GETPROP_TF2DESIREDCLASS].GetInt(edict, 0); }
	inline static void SetTF2Class(edict_t *edict, int _class)
	{
		int* p = g_GetProps[GETPROP_TF2DESIREDCLASS].GetIntPointer(edict);
		if (p != NULL) *p = _class;
	}
	inline static bool TF2_IsMedievalMode(void*gamerules) { if (!gamerules) return false; return g_GetProps[GETPROP_TF2_MEDIEVALMODE].GetBool(gamerules, false, false); }
	inline static int TF2_GetRoundState(void *gamerules) { if (!gamerules) return false; return g_GetProps[GETPROP_TF2_ROUNDSTATE].GetInt(gamerules, 0, 0); }
	inline static float GetTF2SpyCloakMeter(edict_t *edict) { return g_GetProps[GETPROP_TF2SPYMETER].GetFloat(edict, 0); }
	inline static int GetWaterLevel(edict_t *edict) { return g_GetProps[GETPROP_WATERLEVEL].GetInt(edict, 0); }
	inline static void UpdateSimulationTime(edict_t *edict)
	{
		float *m_flSimulationTime = g_GetProps[GETPROP_SIMULATIONTIME].GetFloatPointer(edict);

		if (m_flSimulationTime)
			*m_flSimulationTime = gpGlobals->curtime;
	}

	inline static bool *GetDODCPVisible(edict_t *pResource) { return g_GetProps[GETPROP_DOD_CP_VISIBLE].GetBoolPointer(pResource); }
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
	inline static edict_t *GetOwner(edict_t *edict) { return g_GetProps[GETPROP_ALL_ENTOWNER].GetEntity(edict); }
	inline static bool IsMedigunTargetting(edict_t *pgun, edict_t *ptarget) { return (g_GetProps[GETPROP_TF2MEDIGUN_TARGETTING].GetEntity(pgun) == ptarget); }
	//static void SetTickBase ( edict_t *edict, int tickbase ) { return ;
	inline static int IsTeleporterMode(edict_t *edict, eTeleMode mode) { return (g_GetProps[GETPROP_TF2TELEPORTERMODE].GetInt(edict, -1) == (int)mode); }
	inline static edict_t *GetCurrentWeapon(edict_t *player) { return g_GetProps[GETPROP_CURRENTWEAPON].GetEntity(player); }
	inline static int GetUberChargeLevel(edict_t *pWeapon) { return (int)(g_GetProps[GETPROP_TF2UBERCHARGE_LEVEL].GetFloat(pWeapon, 0)*100.0); }
	//static void Test ();
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
	inline static edict_t *GravityGunObject(edict_t *pgun) { return g_GetProps[GETPROP_HL2DM_PHYSCANNON_ATTACHED].GetEntity(pgun); }
	inline static bool GravityGunOpen(edict_t *pgun) { return g_GetProps[GETPROP_HL2DM_PHYSCANNON_OPEN].GetBool(pgun, false); }
	inline static float AuxPower(edict_t *player) { return g_GetProps[GETPROP_HL2DM_PLAYER_AUXPOWER].GetFloat(player, 0); }
	inline static edict_t *OnLadder(edict_t *player) { return g_GetProps[GETPROP_HL2DM_LADDER_ENT].GetEntity(player); }
	inline static CBaseHandle *GetWeaponList(edict_t *player) { return g_GetProps[GETPROP_WEAPONLIST].GetEntityHandle(player); }
	inline static int GetWeaponState(edict_t *pgun) { return g_GetProps[GETPROP_WEAPONSTATE].GetInt(pgun, 0); }

	inline static edict_t *GetPipeBombOwner(edict_t *pPipeBomb) { return g_GetProps[GETPROP_PIPEBOMB_OWNER].GetEntity(pPipeBomb); }

	inline static int GetDODBombState(edict_t *pBombTarget) { return g_GetProps[GETPROP_DOD_BOMB_STATE].GetInt(pBombTarget, 0); }
	inline static int GetDODBombTeam(edict_t *pBombTarget) { return g_GetProps[GETPROP_DOD_BOMB_TEAM].GetInt(pBombTarget, 0); }
	inline static int *GetWeaponClip1Pointer(edict_t *pgun) { return g_GetProps[GETPROP_WEAPONCLIP1].GetIntPointer(pgun); }
	inline static int *GetWeaponClip2Pointer(edict_t *pgun) { return g_GetProps[GETPROP_WEAPONCLIP2].GetIntPointer(pgun); }
	inline static CAttributeList *GetAttributeList(edict_t *player) { return (CAttributeList*)g_GetProps[GETPROP_TF2_ATTRIBUTELIST].GetVoidPointer(player); }
	inline static int GetOffset(int id) { return g_GetProps[id].GetOffset(); }
	inline static void GetWeaponClip(edict_t *pgun, int *iClip1, int *iClip2) { *iClip1 = g_GetProps[GETPROP_WEAPONCLIP1].GetInt(pgun, 0); *iClip2 = g_GetProps[GETPROP_WEAPONCLIP2].GetInt(pgun, 0); }
	inline static void GetAmmoTypes(edict_t *pgun, int *iAmmoType1, int *iAmmoType2) { *iAmmoType1 = g_GetProps[GETPROP_WEAPON_AMMOTYPE1].GetInt(pgun, -1); *iAmmoType2 = g_GetProps[GETPROP_WEAPON_AMMOTYPE2].GetInt(pgun, -1); }

	inline static int GetPlayerClassDOD(edict_t *player) { return g_GetProps[GETPROP_DOD_PLAYERCLASS].GetInt(player, 0); }
	inline static void GetPlayerInfoDOD(edict_t *player, bool *m_bProne, float *m_flStamina)
	{
		*m_bProne = g_GetProps[GETPROP_DOD_PRONE].GetBool(player, false);
		if (m_flStamina)
			*m_flStamina = g_GetProps[GETPROP_DOD_STAMINA].GetFloat(player, 0);
	}

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

	inline static int GetDODNumControlPoints(edict_t *pResource)
	{
		return g_GetProps[GETPROP_DOD_CP_NUMCAPS].GetInt(pResource, 0);
	}

	inline static Vector *GetOrigin(edict_t *pPlayer)
	{
		return g_GetProps[GETPROP_ORIGIN].GetVectorPointer(pPlayer);
	}

	inline static void SetOrigin(edict_t *pPlayer, Vector vOrigin)
	{
		Vector *vEntOrigin = g_GetProps[GETPROP_ORIGIN].GetVectorPointer(pPlayer);

		*vEntOrigin = vOrigin;
	}

	inline static Vector *GetDODCP_Positions(edict_t *pResource)
	{
		return g_GetProps[GETPROP_DOD_CP_POSITIONS].GetVectorPointer(pResource);
	}

	inline static void GetDODFlagInfo(edict_t *pResource, int **m_iNumAxis, int **m_iNumAllies, int **m_iOwner, int **m_iNumAlliesReq, int **m_iNumAxisReq)
	{
		*m_iNumAxis = g_GetProps[GETPROP_DOD_CP_NUM_AXIS].GetIntPointer(pResource);
		*m_iNumAllies = g_GetProps[GETPROP_DOD_CP_NUM_ALLIES].GetIntPointer(pResource);
		*m_iOwner = g_GetProps[GETPROP_DOD_CP_OWNER].GetIntPointer(pResource);
		*m_iNumAlliesReq = g_GetProps[GETPROP_DOD_CP_ALLIES_REQ_CAP].GetIntPointer(pResource);
		*m_iNumAxisReq = g_GetProps[GETPROP_DOD_CP_AXIS_REQ_CAP].GetIntPointer(pResource);
	}

	inline static void 	GetDODBombInfo(edict_t *pResource, bool **m_bBombPlanted, int **m_iBombsRequired, int **m_iBombsRemaining, bool **m_bBombBeingDefused)
	{
		*m_bBombPlanted = g_GetProps[GETPROP_DOD_BOMBSPLANTED].GetBoolPointer(pResource);
		*m_iBombsRequired = g_GetProps[GETPROP_DOD_BOMBSREQ].GetIntPointer(pResource);
		*m_iBombsRemaining = g_GetProps[GETPROP_DOD_BOMBSREMAINING].GetIntPointer(pResource);
		*m_bBombBeingDefused = g_GetProps[GETPROP_DOD_BOMBSDEFUSED].GetBoolPointer(pResource);
	}

	inline static float GetTF2TauntYaw(edict_t *edict) { return g_GetProps[GETPROP_TF2_TAUNTYAW].GetFloat(edict, 0); }
	inline static bool GetTF2HighFiveReady(edict_t *edict) { return g_GetProps[GETPROP_TF2_HIGHFIVE].GetBool(edict, false); }
	inline static edict_t *GetHighFivePartner(edict_t *edict) { return g_GetProps[GETPROP_TF2_HIGHFIVE_PARTNER].GetEntity(edict); }

	inline static int GetDesPlayerClassDOD(edict_t *player) { return g_GetProps[GETPROP_DOD_DES_PLAYERCLASS].GetInt(player, 0); }

	inline static bool IsSniperWeaponZoomed(edict_t *weapon) { return g_GetProps[GETPROP_DOD_SNIPER_ZOOMED].GetBool(weapon, false); }
	inline static bool IsMachineGunDeployed(edict_t *weapon) { return g_GetProps[GETPROP_DOD_MACHINEGUN_DEPLOYED].GetBool(weapon, false); }
	inline static bool IsRocketDeployed(edict_t *weapon) { return g_GetProps[GETPROP_DOD_ROCKET_DEPLOYED].GetBool(weapon, false); }

	inline static bool IsMoveType(edict_t *pent, int movetype)
	{
		return ((g_GetProps[GETPROP_MOVETYPE].GetInt(pent, 0) & 15) == movetype);
	}

	inline static byte GetTakeDamage(edict_t *pent)
	{
		return (byte)(g_GetProps[GETPROP_TAKEDAMAGE].GetInt(pent, 0));
	}

	inline static byte *GetTakeDamagePointer(edict_t *pent)
	{
		return (g_GetProps[GETPROP_TAKEDAMAGE].GetBytePointer(pent));
	}

	inline static int GetMoveType(edict_t *pent)
	{
		return (g_GetProps[GETPROP_MOVETYPE].GetInt(pent, 0) & 15);
	}

	inline static byte *GetMoveTypePointer(edict_t *pent)
	{
		return (g_GetProps[GETPROP_MOVETYPE].GetBytePointer(pent));
	}

	inline static edict_t *GetGrenadeThrower(edict_t *gren)
	{
		return g_GetProps[GETPROP_DOD_GREN_THROWER].GetEntity(gren);
	}

	inline static int GetPlayerScoreDOD(edict_t *resource, edict_t *pPlayer)
	{
		int *score_array = g_GetProps[GETPROP_DOD_SCORE].GetIntPointer(resource);

		return (score_array != NULL) ? score_array[ENTINDEX(pPlayer)] : 0;
	}

	inline static int GetPlayerObjectiveScoreDOD(edict_t *resource, edict_t *pPlayer)
	{
		int *score_array = g_GetProps[GETPROP_DOD_OBJSCORE].GetIntPointer(resource);

		return (score_array != NULL) ? score_array[ENTINDEX(pPlayer)] : 0;
	}

	inline static int GetPlayerDeathsDOD(edict_t *resource, edict_t *pPlayer)
	{
		int *score_array = g_GetProps[GETPROP_DOD_DEATHS].GetIntPointer(resource);

		return (score_array != NULL) ? score_array[ENTINDEX(pPlayer)] : 0;
	}

	inline static float GetSmokeSpawnTime(edict_t *pSmoke)
	{
		return g_GetProps[GETPROP_DOD_SMOKESPAWN_TIME].GetFloat(pSmoke, 0);
	}

	inline static float GetRoundTime(edict_t *pGamerules)
	{
		return g_GetProps[GETPROP_DOD_ROUNDTIME].GetFloat(pGamerules, 0);
	}

	inline static bool IsGarandZoomed(edict_t *pGarand)
	{
		return g_GetProps[GETPROP_DOD_GARANDZOOM].GetBool(pGarand, false);
	}

	inline static bool IsK98Zoomed(edict_t *pK98)
	{
		return g_GetProps[GETPROP_DOD_K98ZOOM].GetBool(pK98, false);
	}
	// HL2DM
	//static void 

	inline static bool AreAlliesBombing(edict_t *pRes)
	{
		return g_GetProps[GETPROP_DOD_ALLIESBOMBING].GetBool(pRes, false);
	}
	inline static bool AreAxisBombing(edict_t *pRes)
	{
		return g_GetProps[GETPROP_DOD_AXISBOMBING].GetBool(pRes, false);
	}
	inline static int *IsBombPlantedList(edict_t *pRes)
	{
		return g_GetProps[GETPROP_DOD_BOMBSPLANTED].GetIntPointer(pRes);
	}
	inline static int *GetNumBombsRequiredList(edict_t *pRes)
	{
		return g_GetProps[GETPROP_DOD_BOMBSREQ].GetIntPointer(pRes);
	}
	inline static int *IsBombDefusingList(edict_t *pRes)
	{
		return g_GetProps[GETPROP_DOD_BOMBSDEFUSED].GetIntPointer(pRes);
	}
	inline static int *GetNumBombsRemaining(edict_t *pRes)
	{
		return g_GetProps[GETPROP_DOD_BOMBSREMAINING].GetIntPointer(pRes);
	}

	inline static bool IsPlayerDefusingBomb_DOD(edict_t *pPlayer)
	{
		return g_GetProps[GETPROP_DOD_DEFUSINGBOMB].GetBool(pPlayer, false);
	}

	inline static bool IsPlayerPlantingBomb_DOD(edict_t *pPlayer)
	{
		return g_GetProps[GETPROP_DOD_PLANTINGBOMB].GetBool(pPlayer, false);
	}

	inline static bool IsSentryGunBeingPlaced(edict_t *pSentry)
	{
		return g_GetProps[GETPROP_SENTRYGUN_PLACING].GetBool(pSentry, false);
	}

private:
	static CClassInterfaceValue g_GetProps[GET_PROPDATA_MAX];

};

#endif