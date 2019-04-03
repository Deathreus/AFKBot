#ifndef __RCBOT_GETPROP_H__
#define __RCBOT_GETPROP_H__

#include "bot_gamerules.h"

#include <server_class.h>

#include <limits>
#include <type_traits>
#include <typeinfo>

class CTFObjectiveResource;
class CTeamRoundTimer;
class CAttributeList;

typedef enum
{
	TELE_ENTRANCE = 0,
	TELE_EXIT
}eTeleMode;

namespace SourceMod
{
#define FIND_PROP_DATA() \
	datamap_t *pMap; \
	if ((pMap = gamehelpers->GetDataMap(pEntity)) == NULL) \
	{ \
		return {0}; \
	} \
	sm_datatable_info_t info; \
	if (!gamehelpers->FindDataMapInfo(pMap, prop, &info)) \
	{ \
		return {0}; \
	} \
	typedescription_t *td = info.prop

#define CHECK_SET_PROP_DATA_OFFSET() \
	if (element < 0 || element >= td->fieldSize) \
	{ \
		smutils->LogError(myself, "Element %d is outside of array bounds", element); \
		return {0}; \
	} \
	int offset = info.actual_offset + (element * (td->fieldSizeInBytes / td->fieldSize))

#define FIND_PROP_SEND() \
	IServerNetworkable *pNet = pEdict->GetNetworkable(); \
	if (!pNet) \
	{ \
		smutils->LogError(myself, "Edict %d is not networkable", \
						  gamehelpers->EntityToReference(pEntity)); \
		return {0}; \
	} \
	sm_sendprop_info_t info; \
	if (!gamehelpers->FindSendPropInfo(pNet->GetServerClass()->GetName(), prop, &info)) \
	{ \
		smutils->LogError(myself, "Prop '%s' not found on edict(%d)", \
						  prop, \
						  gamehelpers->EntityToReference(pEntity));	 \
		return {0}; \
	} \
	int offset = info.actual_offset; \
	SendProp *pProp = info.prop; \
	int bit_count = pProp->m_nBits

#define FIND_PROP_IF_SENDTABLE() \
	if(pProp->GetType() == DPT_DataTable) \
	{ \
		SendTable *pTable = pProp->GetDataTable(); \
		if (!pTable) \
		{ \
			smutils->LogError(myself, "Unknown error looking up SendTable."); \
			return {0}; \
		} \
		\
		int elementCount = pTable->GetNumProps(); \
		if (element < 0 || element >= elementCount) \
		{ \
			smutils->LogError(myself, "Element %d is outside of array bounds", element); \
			return {0}; \
		} \
		\
		pProp = pTable->GetProp(element); \
		offset += pProp->GetOffset(); \
	} \
	bit_count = pProp->m_nBits

#define CHECK_TYPE_VALID_IF_VARIANT(type, typeName) \
	if (td->fieldType == FIELD_CUSTOM && (td->flags & FTYPEDESC_OUTPUT) == FTYPEDESC_OUTPUT) \
	{ \
		variant_t *pVariant = (variant_t *)((intptr_t)pEntity + offset); \
		if (pVariant->fieldType != type) \
		{ \
			smutils->LogError(myself, "Variant value for %s is not %s (%d)", \
							  prop, \
							  typeName, \
							  pVariant->fieldType); \
			return {0}; \
		} \
	}

#define SET_TYPE_IF_VARIANT(type) \
	if (td->fieldType == FIELD_CUSTOM && (td->flags & FTYPEDESC_OUTPUT) == FTYPEDESC_OUTPUT) \
	{ \
		variant_t *pVariant = (variant_t *)((intptr_t)pEntity + offset); \
		pVariant->fieldType = type;	\
	}



int GetEntPropArraySize(edict_t *pEdict, const char *prop);
int MatchFieldAsInteger(int field_type);

template<typename T>
T GetEntSend(edict_t *pEdict, const char *prop, int element = 0)
{
	CBaseEntity *pEntity = gameents->EdictToBaseEntity(pEdict);
	Assert( pEntity );

	if(element && !GetEntPropArraySize(pEdict, prop))
	{
		return { 0 };
	}

	FIND_PROP_SEND();
	FIND_PROP_IF_SENDTABLE();

	const SendPropType type = pProp->GetType();

	if(type >= DPT_Int && type <= DPT_VectorXY)
	{
		if(type == DPT_Int)
		{
			if(!std::is_assignable<T&, int>::value)
			{
				smutils->LogError(myself, "SendProp expected to be an int, but we tried to use %s.  Property %s", typeid(T).name(), prop);
				return 0;
			}

			bool is_unsigned = ((pProp->GetFlags() & SPROP_UNSIGNED) == SPROP_UNSIGNED);
			if(pProp->GetFlags() & SPROP_VARINT)
			{
				bit_count = sizeof(int) * 8;
			}

			if(bit_count < 1)
			{
				bit_count = std::numeric_limits<T>::digits;
			}

			AFKBot::VerboseDebugMessage(__FUNCTION__ ": Property %s data: Type %d\tBits %u\tAddress %p\tValue %d", prop, type, bit_count, (T*)((uint8_t *)pEntity + offset), *(T*)((uint8_t *)pEntity + offset));

			if(bit_count >= 17)
			{
				return *(int32_t *)((uint8_t *)pEntity + offset);
			}
			else if(bit_count >= 9)
			{
				if(is_unsigned)
				{
					return *(uint16_t *)((uint8_t *)pEntity + offset);
				}
				else
				{
					return *(int16_t *)((uint8_t *)pEntity + offset);
				}
			}
			else if(bit_count >= 2)
			{
				if(is_unsigned)
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
				return *(bool *)((uint8_t *)pEntity + offset);
			}
		}

		AFKBot::VerboseDebugMessage(__FUNCTION__ ": Property %s data: Type %d\tBits %u\tAddress %p\tValue %d", prop, type, bit_count, (T*)((uint8_t *)pEntity + offset), *(T*)((uint8_t *)pEntity + offset));

		return *(T *)((uint8_t *)pEntity + offset);
	}

	return { 0 };
}

template<typename T>
bool SetEntSend(edict_t *pEdict, const char *prop, T value, int element = 0)
{
	CBaseEntity *pEntity = gameents->EdictToBaseEntity(pEdict);
	Assert( pEntity );

	if(element && !GetEntPropArraySize(pEdict, prop, element))
	{
		smutils->LogError(myself, "SendProp \"%s\" is not an array.", prop);
		return false;
	}

	FIND_PROP_SEND();
	FIND_PROP_IF_SENDTABLE();

	switch(pProp->GetType())
	{
		case DPT_Int:
		{
			if(!std::is_assignable<int&, T>::value)
			{
				smutils->LogError(myself, "SendProp expected to be an int, but we tried to use %s.  Property %s", typeid(T).name(), prop);
				return false;
			}

			if(pProp->GetFlags() & SPROP_VARINT)
			{
				bit_count = sizeof(int) * 8;
			}

			if(bit_count < 1)
			{
				bit_count = std::numeric_limits<T>::digits;
			}

			if(bit_count >= 17)
			{
				*(int32_t *)((uint8_t *)pEntity + offset) = (int32_t)value;
			}
			else if(bit_count >= 9)
			{
				*(int16_t *)((uint8_t *)pEntity + offset) = (int16_t)value;
			}
			else if(bit_count >= 2)
			{
				*(int8_t *)((uint8_t *)pEntity + offset) = (int8_t)value;
			}
			else
			{
				*(bool *)((uint8_t *)pEntity + offset) = (bool)value;
			}

			gamehelpers->SetEdictStateChanged(pEdict, offset);

			return true;
		}
		case DPT_Float:
		{
			*(float *)((uint8_t *)pEntity + offset) = value;

			gamehelpers->SetEdictStateChanged(pEdict, offset);

			return true;
		}
		case DPT_Vector:
		case DPT_VectorXY:
		{
			*(Vector *)((uint8_t *)pEntity + offset) = value;

			gamehelpers->SetEdictStateChanged(pEdict, offset);

			return true;
		}
	}

	return false;
}

const char *GetEntSendString(edict_t *pEdict, const char *prop, int element = 0);
bool SetEntSendString(edict_t *pEdict, const char *prop, const char *value, int element = 0);
edict_t *GetEntSendEnt(edict_t *pEdict, const char *prop, int element = 0);
bool SetEntSendEnt(edict_t *pEdict, const char *prop, edict_t *value, int element = 0);

template<typename T>
T GetEntData(edict_t *pEdict, const char *prop, int element = 0)
{
	CBaseEntity *pEntity = gameents->EdictToBaseEntity(pEdict);
	Assert( pEntity );

	FIND_PROP_DATA();
	int bit_count = MatchFieldAsInteger(td->fieldType);
	CHECK_SET_PROP_DATA_OFFSET();

	if(bit_count < 1)
	{
		bit_count = std::numeric_limits<T>::digits;
	}

	if(td->fieldType == FIELD_FLOAT || td->fieldType == FIELD_TIME
	|| td->fieldType == FIELD_VECTOR || td->fieldType == FIELD_POSITION_VECTOR)
	{
		AFKBot::VerboseDebugMessage(__FUNCTION__ ": Property %s data: Type %d\tBits %u\tAddress %p\tValue %d", prop, td->fieldType, bit_count, (T*)((uint8_t *)pEntity + offset), *(T*)((uint8_t *)pEntity + offset));
		return *(T *)((uint8_t *)pEntity + offset);
	}
	else
	{
		AFKBot::VerboseDebugMessage(__FUNCTION__ ": Property %s data: Type %d\tBits %u\tAddress %p\tValue %d", prop, td->fieldType, bit_count, (T*)((uint8_t *)pEntity + offset), *(T*)((uint8_t *)pEntity + offset));

		if(bit_count >= 17)
		{
			return *(int32_t *)((uint8_t *)pEntity + offset);
		}
		else if(bit_count >= 9)
		{
			return *(int16_t *)((uint8_t *)pEntity + offset);
		}
		else if(bit_count >= 2)
		{
			return *(int8_t *)((uint8_t *)pEntity + offset);
		}
		else
		{
			return *(bool *)((uint8_t *)pEntity + offset);
		}
	}

	return { 0 };
}

template<typename T>
bool SetEntData(edict_t *pEdict, const char *prop, T value, int element = 0)
{
	CBaseEntity *pEntity = gameents->EdictToBaseEntity(pEdict);
	Assert( pEntity );

	FIND_PROP_DATA();
	int bit_count = MatchFieldAsInteger(td->fieldType);
	CHECK_SET_PROP_DATA_OFFSET();

	if(bit_count < 1)
	{
		bit_count = std::numeric_limits<T>::digits;
	}

	if(td->fieldType == FIELD_VECTOR || td->fieldType == FIELD_POSITION_VECTOR)
	{
		if(td->fieldType == FIELD_CUSTOM && (td->flags & FTYPEDESC_OUTPUT) == FTYPEDESC_OUTPUT)
		{
			variant_t *pVariant = (variant_t *)((intptr_t)pEntity + offset);
			// Both of these are supported and we don't know which is intended. But, if it's already
			// a pos vector, we probably want to keep that.
			if(pVariant->fieldType != FIELD_POSITION_VECTOR)
			{
				pVariant->fieldType = FIELD_VECTOR;
			}
		}

		*(Vector *)((uint8_t *)pEntity + offset) = value;
	}
	else if(td->fieldType == FIELD_FLOAT || td->fieldType == FIELD_TIME)
	{
		SET_TYPE_IF_VARIANT(FIELD_FLOAT);

		*(float *)((uint8_t *)pEntity + offset) = value;
	}
	else
	{
		SET_TYPE_IF_VARIANT(FIELD_INTEGER);

		if(bit_count >= 17)
		{
			*(int32_t *)((uint8_t *)pEntity + offset) = (int32_t)value;
		}
		else if(bit_count >= 9)
		{
			*(int16_t *)((uint8_t *)pEntity + offset) = (int16_t)value;
		}
		else if(bit_count >= 2)
		{
			*(int8_t *)((uint8_t *)pEntity + offset) = (int8_t)value;
		}
		else
		{
			*(bool *)((uint8_t *)pEntity + offset) = (bool)value;
		}
	}

	return true;
}

const char *GetEntDataString(edict_t *pEdict, const char *prop, int element = 0);
bool SetEntDataString(edict_t *pEdict, const char *prop, const char *value, int element = 0);
edict_t *GetEntDataEnt(edict_t *pEdict, const char *prop, int element = 0);
bool SetEntDataEnt(edict_t *pEdict, const char *prop, edict_t *value, int element = 0);
};

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
	GETPROP_ENTOWNER,
	GETPROP_GROUND_ENTITY,
	GETPROP_ORIGIN,
	GETPROP_WATERLEVEL,
	GETPROP_TF2SENTRY_ENEMY,
	GETPROP_TF2SENTRY_SHELLS,
	GETPROP_TF2SENTRY_ROCKETS,
	GETPROP_TF2SCORE,
	GETPROP_TF2_NUMHEALERS,
	GETPROP_TF2CLASS,
	GETPROP_TF2SPYMETER,
	GETPROP_TF2SPYDISGUISED_TEAM,
	GETPROP_TF2SPYDISGUISED_CLASS,
	GETPROP_TF2SPYDISGUISED_TARGET_INDEX,
	GETPROP_TF2SPYDISGUISED_DIS_HEALTH,
	GETPROP_TF2MEDIGUN_HEALING,
	GETPROP_TF2MEDIGUN_TARGETTING,
	GETPROP_TF2OBJECT_MODE,
	GETPROP_TF2UBERCHARGE_LEVEL,
	GETPROP_TF2OBJECT_HEALTH,
	GETPROP_TF2OBJECT_CARRIED,
	GETPROP_TF2OBJECT_UPGRADELEVEL,
	GETPROP_TF2OBJECT_UPGRADEMETAL,
	GETPROP_TF2OBJECT_MAXHEALTH,
	GETPROP_TF2DISPENSER_METAL,
	GETPROP_TF2MINIBUILDING,
	GETPROP_TF2OBJECT_BUILDING,
	GETPROP_TF2TELEPORT_RECHARGETIME,
	GETPROP_TF2TELEPORT_RECHARGEDURATION,
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
	GETPROP_TF2OBJECT_PLACING,
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
	GETPROP_TF2BUILDER_TYPE,
	GETPROP_TF2BUILDER_MODE,
	GETPROP_TF2MEDIGUN_RESIST_TYPE,
	GETPROP_TF2_ROUNDSTATE,
	GETPROP_TF2DESIREDCLASS,
	GETPROP_MVMHASMINPLAYERSREADY,
	GETPROP_MVMCURRENCY,
	GETPROP_TF2OBJECT_TYPE,
	GETPROP_MVMINUPGZONE,
	GETPROP_TF2_PCTINVISIBLE,
	GETPROP_TF2_LASTDMGTYPE,
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

	static int GetTF2Score(edict_t *edict);

	inline static float GetRageMeter(edict_t *edict) { return g_GetProps[GETPROP_TF2_RAGEMETER].GetFloat(edict, 0); }

	inline static int GetTeam(edict_t *edict) { return g_GetProps[GETPROP_TEAM].GetInt(edict, 0); }

	inline static float GetPlayerHealth(edict_t *edict) { return g_GetProps[GETPROP_PLAYERHEALTH].GetFloatFromInt(edict, 0); }

	inline static int GetEffects(edict_t *edict) { return g_GetProps[GETPROP_EFFECTS].GetInt(edict, 0); }

	inline static int *GetAmmoList(edict_t *edict) { return g_GetProps[GETPROP_AMMO].GetIntPointer(edict); }

	inline static int GetTF2NumHealers(edict_t *edict) { return g_GetProps[GETPROP_TF2_NUMHEALERS].GetInt(edict, 0); }

	inline static bool GetVelocity(edict_t *edict, Vector *v) { return g_GetProps[GETPROP_VELOCITY].GetVector(edict, v); }

	inline static int GetTF2Class(edict_t *edict) { return g_GetProps[GETPROP_TF2CLASS].GetInt(edict, 0); }

	inline static edict_t *GetExtraWearable(edict_t *edict) { return g_GetProps[GETPROP_TF2_EXTRAWEARABLE].GetEntity(edict); }

	inline static edict_t *GetExtraWearableViewModel(edict_t *edict) { return g_GetProps[GETPROP_TF2_EXTRAWEARABLEVIEWMODEL].GetEntity(edict); }

	inline static float TF2_GetEnergyDrinkMeter(edict_t * edict) { return g_GetProps[GETPROP_TF2_ENERGYDRINKMETER].GetFloat(edict, 0); }

	inline static edict_t *TF2_GetActiveWeapon(edict_t *edict) { return g_GetProps[GETPROP_TF2_ACTIVEWEAPON].GetEntity(edict); }
	
	static bool TF2_SetActiveWeapon(edict_t *edict, edict_t *pWeapon)
	{
		CBaseHandle *pHandle = g_GetProps[GETPROP_TF2_ACTIVEWEAPON].GetEntityHandle(edict);
		pHandle->Set(pWeapon->GetNetworkable()->GetEntityHandle());
	}

	inline static void TF2_SetBuilderType(edict_t *pBuilder, int itype)
	{
		int *piType = g_GetProps[GETPROP_TF2BUILDER_TYPE].GetIntPointer(pBuilder);

		*piType = itype;
	}

	inline static int GetChargeResistType(edict_t *pMedigun)
	{
		return g_GetProps[GETPROP_TF2MEDIGUN_RESIST_TYPE].GetInt(pMedigun, 0);
	}

	inline static void TF2_SetBuilderMode(edict_t *pBuilder, int imode)
	{
		int *piType = g_GetProps[GETPROP_TF2BUILDER_MODE].GetIntPointer(pBuilder);

		*piType = imode;
	}
	
	inline static int GetTF2DesiredClass(edict_t *edict) { return g_GetProps[GETPROP_TF2DESIREDCLASS].GetInt(edict, 0); }

	inline static void SetTF2Class(edict_t *edict, int _class)
	{
		int *piClass = g_GetProps[GETPROP_TF2DESIREDCLASS].GetIntPointer(edict);
		if (piClass != NULL) *piClass = _class;
	}
	
	inline static float GetTF2SpyCloakMeter(edict_t *edict) { return g_GetProps[GETPROP_TF2SPYMETER].GetFloat(edict, 0); }

	inline static int GetWaterLevel(edict_t *edict) { return g_GetProps[GETPROP_WATERLEVEL].GetInt(edict, 0); }

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

	inline static bool IsCarryingObj(edict_t *edict) { return g_GetProps[GETPROP_TF2_ISCARRYINGOBJ].GetBool(edict, false); }

	inline static edict_t *GetCarriedObj(edict_t *edict) { return g_GetProps[GETPROP_TF2_GETCARRIEDOBJ].GetEntity(edict); }

	inline static bool GetMedigunHealing(edict_t *edict) { return g_GetProps[GETPROP_TF2MEDIGUN_HEALING].GetBool(edict, false); }

	inline static edict_t *GetMedigunTarget(edict_t *edict) { return g_GetProps[GETPROP_TF2MEDIGUN_TARGETTING].GetEntity(edict); }

	inline static edict_t *GetOwner(edict_t *edict) { return g_GetProps[GETPROP_ENTOWNER].GetEntity(edict); }

	inline static bool IsMedigunTargetting(edict_t *pgun, edict_t *ptarget) { return (g_GetProps[GETPROP_TF2MEDIGUN_TARGETTING].GetEntity(pgun) == ptarget); }

	inline static bool IsTeleporterMode(edict_t *edict, eTeleMode mode) { return (g_GetProps[GETPROP_TF2OBJECT_MODE].GetInt(edict, -1) == (int)mode); }

	inline static int GetObjectType(edict_t *edict) { return g_GetProps[GETPROP_TF2OBJECT_TYPE].GetInt(edict, -1); }

	inline static edict_t *GetCurrentWeapon(edict_t *player) { return g_GetProps[GETPROP_CURRENTWEAPON].GetEntity(player); }

	inline static int GetUberChargeLevel(edict_t *pWeapon) { return (int)(g_GetProps[GETPROP_TF2UBERCHARGE_LEVEL].GetFloat(pWeapon, 0)*100.0); }

	inline static int GetObjectHealth(edict_t *edict) { return g_GetProps[GETPROP_TF2OBJECT_HEALTH].GetInt(edict, 0); }
	inline static int GetObjectMaxHealth(edict_t *edict) { return g_GetProps[GETPROP_TF2OBJECT_MAXHEALTH].GetInt(edict, 0); }

	inline static bool IsObjectCarried(edict_t *edict) { return g_GetProps[GETPROP_TF2OBJECT_CARRIED].GetBool(edict, false); }
	inline static bool IsObjectBeingPlaced(edict_t *pSentry) { return g_GetProps[GETPROP_TF2OBJECT_PLACING].GetBool(pSentry, false); }

	inline static int GetObjectUpgradeLevel(edict_t *edict) { return g_GetProps[GETPROP_TF2OBJECT_UPGRADELEVEL].GetInt(edict, 0); }
	inline static int GetObjectUpgradeMetal(edict_t *edict) { return g_GetProps[GETPROP_TF2OBJECT_UPGRADEMETAL].GetInt(edict, 0); }

	inline static edict_t *GetSentryEnemy(edict_t *edict) { return g_GetProps[GETPROP_TF2SENTRY_ENEMY].GetEntity(edict); }
	inline static int GetSentryShells(edict_t *edict) { return g_GetProps[GETPROP_TF2SENTRY_SHELLS].GetInt(edict, 0); }
	inline static int GetSentryRockets(edict_t *edict) { return g_GetProps[GETPROP_TF2SENTRY_ROCKETS].GetInt(edict, 0); }

	inline static float GetTF2TeleRechargeTime(edict_t *edict) { return g_GetProps[GETPROP_TF2TELEPORT_RECHARGETIME].GetFloat(edict, 0); }
	inline static float GetTF2TeleRechargeDuration(edict_t *edict) { return g_GetProps[GETPROP_TF2TELEPORT_RECHARGEDURATION].GetFloat(edict, 0); }

	inline static int GetTF2DispMetal(edict_t *edict) { return g_GetProps[GETPROP_TF2DISPENSER_METAL].GetInt(edict, 0); }

	inline static bool GetTF2BuildingIsMini(edict_t *edict) { return g_GetProps[GETPROP_TF2MINIBUILDING].GetBool(edict, false); }

	inline static float GetMaxSpeed(edict_t *edict) { return g_GetProps[GETPROP_MAXSPEED].GetFloat(edict, 0); }

	inline static float GetSpeedFactor(edict_t *edict) { return g_GetProps[GETPROP_CONSTRAINT_SPEED].GetFloat(edict, 0); }

	inline static bool IsObjectBeingBuilt(edict_t *edict) { return g_GetProps[GETPROP_TF2OBJECT_BUILDING].GetBool(edict, false); }

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

	static bool IsMoveType(edict_t *edict, byte movetype);

	static int GetMoveType(edict_t *edict);

	inline static bool MvMMinPlayersToReady()
	{
		return CGameRulesObject::GetProperty("m_bHaveMinPlayersToEnableReady") == 1;
	}

	inline static bool MvMIsPlayerReady(int client)
	{
		return CGameRulesObject::GetProperty("m_bPlayerReady", 1, client) == 1;
	}

	inline static bool MvMIsPlayerAtUpgradeStation(edict_t *edict)
	{
		return g_GetProps[GETPROP_MVMINUPGZONE].GetBool(edict, false);
	}

private:
	static CClassInterfaceValue g_GetProps[GET_PROPDATA_MAX];
};

#endif