/*
 * ================================================================================
 * AFK Bot Extension
 * Copyright (C) 2015 Chris Moore (Deathreus).  All rights reserved.
 * ================================================================================
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
 */

/*
 *	Attributions & Thanks:
 *	=====================
 *	Cheeseh	-	Built the bot code that all of this runs on
 *	pimpinjuice -	Made the NavMesh parsing code to convert the bot from waypointing
 */

#include <stdio.h>

#include "extension.h"
#include <compat_wrappers.h>

#include <Color.h>
#include <time.h>

#include <KeyValues.h>

#include "rcbot2/bot_cvars.h"

#include "rcbot2/bot.h"
#include "rcbot2/bot_configfile.h"
#include "rcbot2/bot_globals.h"
#include "rcbot2/bot_profile.h"
#include "rcbot2/bot_waypoint.h"
#include "rcbot2/bot_getprop.h"
#include "rcbot2/bot_fortress.h"
#include "rcbot2/bot_event.h"
#include "rcbot2/bot_wpt_dist.h"
#include "rcbot2/bot_squads.h"
#include "rcbot2/bot_weapons.h"
#include "rcbot2/bot_waypoint_visibility.h"
#include "rcbot2/bot_gamerules.h"
//#include "rcbot2/bot_navmesh.h"

using namespace SourceMM;

CBotTF2 *g_pLastBot;

SH_DECL_HOOK6(IServerGameDLL, LevelInit, SH_NOATTRIB, 0, bool, char const *, char const *, char const *, char const *, bool, bool);
SH_DECL_HOOK3_void(IServerGameDLL, ServerActivate, SH_NOATTRIB, 0, edict_t *, int, int);
SH_DECL_HOOK1_void(IServerGameDLL, GameFrame, SH_NOATTRIB, 0, bool);
SH_DECL_HOOK0_void(IServerGameDLL, LevelShutdown, SH_NOATTRIB, 0);
SH_DECL_HOOK2_void(IServerGameClients, ClientActive, SH_NOATTRIB, 0, edict_t *, bool);
SH_DECL_HOOK1_void(IServerGameClients, ClientDisconnect, SH_NOATTRIB, 0, edict_t *);
SH_DECL_HOOK2_void(IServerGameClients, ClientPutInServer, SH_NOATTRIB, 0, edict_t *, char const *);
SH_DECL_HOOK2(IGameEventManager2, FireEvent, SH_NOATTRIB, 0, bool, IGameEvent *, bool);

SH_DECL_MANUALHOOK2_void(PlayerRunCmd, 0, 0, 0, CUserCmd*, IMoveHelper*);

AFKBot g_AFKBot;

SMEXT_LINK(&g_AFKBot);

CBaseEntity* (CBaseEntity::*TF2PlayerWeaponSlot)(int) = 0x0;
void (CBaseEntity::*TF2WeaponEquip)(CBaseEntity*) = 0x0;

IGameEventManager2 *gameevents = NULL;
IServerPluginCallbacks *vsp_callbacks = NULL;
ICvar *icvar = NULL;
IFileSystem *filesystem = NULL;  // file I/O
IPlayerInfoManager *playerinfomanager = NULL;  // game dll interface to interact with players
IServerPluginHelpers *helpers = NULL;  // special 3rd party plugin helpers from the engine
IServerGameClients* gameclients = NULL;
IEngineTrace *enginetrace = NULL;
IEffects *effects = NULL;
CGlobalVars *gpGlobals = NULL;
IVDebugOverlay *debugoverlay = NULL;
IServerGameEnts *servergameents = NULL; // for accessing the server game entities

static ConVar bot_version(BOT_VER_CVAR, SMEXT_CONF_VERSION, FCVAR_SPONLY|FCVAR_REPLICATED|FCVAR_DONTRECORD|FCVAR_NOTIFY, BOT_NAME_VER);
ConVar bot_enabled("afkbot_enabled", "1", FCVAR_NOTIFY, "Enable turning players into bots?", true, 0.0f, true, 1.0f);

IGameConfig *g_pGameConf = NULL;
ISDKTools *g_pSDKTools = NULL;

int bot_weaponequip_offset;
int bot_getweaponslot_offset;
int bot_playerruncmd_offset;

sp_nativeinfo_t g_ExtensionNatives[] =
{
	{ "SetClientAFKBot",	SetClientAFKBot },
	{ "IsClientAFKBot",		IsClientAFKBot },
	{ NULL,						NULL }
};

class CBotRecipientFilter : public IRecipientFilter
{
public:
	CBotRecipientFilter(edict_t *pPlayer)
	{
		m_iPlayerSlot = ENTINDEX(pPlayer);
	}

	bool IsReliable(void) const { return false; }
	bool IsInitMessage(void) const { return false; }

	int	GetRecipientCount(void) const { return 1; }
	int	GetRecipientIndex(int slot) const { return m_iPlayerSlot; }

private:
	int m_iPlayerSlot;
};

class CClientBroadcastRecipientFilter : public IRecipientFilter
{
public:

	CClientBroadcastRecipientFilter() 
	{
		m_iMaxCount = 0;

		for (int i = 0; i < MAX_PLAYERS; ++i) 
		{
			CClient* client = CClients::Get(i);

			if (client->IsUsed()) 
			{
				IPlayerInfo *p = playerinfomanager->GetPlayerInfo(client->GetPlayer());

				if (p->IsConnected() && !p->IsFakeClient()) 
				{
					m_iPlayerSlot[m_iMaxCount] = i;
					m_iMaxCount++;
				}
			}
		}
	}

	bool IsReliable(void) const { return false; }
	bool IsInitMessage(void) const { return false; }

	int	GetRecipientCount(void) const { return m_iMaxCount; }
	int	GetRecipientIndex(int slot) const { return m_iPlayerSlot[slot] + 1; }

private:

	int m_iMaxCount;
	int m_iPlayerSlot[MAX_PLAYERS];
};

CBaseEntity *AFKBot::TF2_GetPlayerWeaponSlot(edict_t *pPlayer, int iSlot)
{
	CBaseEntity *pEnt = servergameents->EdictToBaseEntity(pPlayer);
	unsigned int *mem = (unsigned int*)*(unsigned int*)pEnt;
	int offset = bot_getweaponslot_offset;

	*(unsigned int*)&TF2PlayerWeaponSlot = mem[offset];

	return (*pEnt.*TF2PlayerWeaponSlot)(iSlot);
}

void AFKBot::TF2_EquipWeapon(edict_t *pPlayer, CBaseEntity *pWeapon)
{
	CBaseEntity *pEnt = servergameents->EdictToBaseEntity(pPlayer);
	unsigned int *mem = (unsigned int*)*(unsigned int*)pEnt;
	int offset = bot_weaponequip_offset;

	*(unsigned int*)&TF2WeaponEquip = mem[offset];

	(*pEnt.*TF2WeaponEquip)(pWeapon);
}

void AFKBot::PlayerRunCmd(CUserCmd *ucmd, IMoveHelper *moveHelper)
{
	if (bot_stop.GetBool())
		RETURN_META(MRES_IGNORED);

	edict_t *pEdict = servergameents->BaseEntityToEdict(META_IFACEPTR(CBaseEntity));
	static CBot *pBot; pBot = CBots::GetBotPointer(pEdict);

	if (pBot)
	{
		if (!bot_enabled.GetBool())
		{
			CBots::MakeNotBot(pEdict);
			RETURN_META(MRES_IGNORED);
		}

		static CUserCmd *cmd;
		static CPlayerState *pl;

		cmd = pBot->GetUserCMD();
		// put the bot's commands into this move frame
		ucmd->buttons = cmd->buttons;
		ucmd->forwardmove = cmd->forwardmove;
		ucmd->impulse = cmd->impulse;
		ucmd->sidemove = cmd->sidemove;
		ucmd->upmove = cmd->upmove;
		ucmd->viewangles = cmd->viewangles;
		ucmd->weaponselect = cmd->weaponselect;
		ucmd->weaponsubtype = cmd->weaponsubtype;
		ucmd->tick_count = cmd->tick_count;
		ucmd->command_number = cmd->command_number;

		pl = gameclients->GetPlayerState(pEdict);
		pl->v_angle = cmd->viewangles;
		pl->fixangle = FIXANGLE_ABSOLUTE;

		g_pLastBot = (CBotTF2*)pBot;
	}

	RETURN_META(MRES_IGNORED);
}

void AFKBot::OnMaxPlayersChanged(int newvalue)
{
	CBotGlobals::SetClientMax(newvalue);
}

void AFKBot::ServerActivate(edict_t *pEdictList, int edictCount, int clientMax)
{
	CBotGlobals::SetClientMax(clientMax);
}

void AFKBot::ClientActive(edict_t *pClient, bool bLoadGame)
{
	CClients::ClientActive(pClient);

	CBotMod *pMod = CBotGlobals::GetCurrentMod();
	pMod->PlayerSpawned(pClient);

	CBaseEntity *pEntity = servergameents->EdictToBaseEntity(pClient);
	SH_ADD_MANUALHOOK_MEMFUNC(PlayerRunCmd, pEntity, this, &AFKBot::PlayerRunCmd, false);
}

void AFKBot::ClientPutInServer(edict_t *pClient, const char *playername)
{
	CClients::ClientConnected(pClient);
	CClients::Init(pClient);
}

void AFKBot::ClientDisconnect(edict_t *pClient)
{
	CClients::ClientDisconnected(pClient);

	CBaseEntity *pEntity = servergameents->EdictToBaseEntity(pClient);
	SH_REMOVE_MANUALHOOK_MEMFUNC(PlayerRunCmd, pEntity, this, &AFKBot::PlayerRunCmd, false);
}

void AFKBot::GameFrame(bool simulating)
{
	static CBotMod *currentmod;

	if (simulating && CBotGlobals::IsMapRunning())
	{
		CBots::BotThink();
		CClients::ClientThink();

		if (CWaypoints::GetVisiblity()->NeedToWorkVisibility())
		{
			CWaypoints::GetVisiblity()->WorkVisibility();
		}

		// Config Commands
		CBotConfigFile::DoNextCommand();
		currentmod = CBotGlobals::GetCurrentMod();

		currentmod->ModFrame();
	}
}

bool AFKBot::LevelInit(const char *pMapName,
	char const *pMapEntities,
	char const *pOldLevel,
	char const *pLandmarkName,
	bool loadGame,
	bool background)
{
	// Must set this
	CBotGlobals::SetMapName(pMapName);

	/*char *error[512];
	CNavMeshNavigator::Init(error, sizeof(error));*/

	CWaypointDistances::Reset();

	CWaypoints::Init();
	CWaypoints::Load();

	CBotGlobals::SetMapRunning(true);
	CBotConfigFile::Reset();

	if (mp_teamplay)
		CBotGlobals::SetTeamplay(mp_teamplay->GetBool());
	else
		CBotGlobals::SetTeamplay(false);

	CBotEvents::SetupEvents();

	CBots::MapInit();

	CBotMod *pMod = CBotGlobals::GetCurrentMod();

	if (pMod)
		pMod->MapInit();

	CBotSquads::FreeMemory();

	return true;
}

void AFKBot::LevelShutdown()
{
	CClients::Initall();
	CWaypointDistances::Save();

	CBots::FreeMapMemory();
	CWaypoints::Init();

	//CNavMeshNavigator::FreeMapMemory();

	CBotGlobals::SetMapRunning(false);
	CBotEvents::FreeMemory();
}

bool AFKBot::FireGameEvent(IGameEvent *pEvent, bool bDontBroadcast)
{
	static char szKey[128];
	static char szValue[128];

	CBotEvents::ExecuteEvent((void*)pEvent, TYPE_IGAMEEVENT);

	RETURN_META_VALUE(MRES_IGNORED, true);
}

bool AFKBot::SDK_OnLoad(char *error, size_t maxlength, bool late) {

	char conf_error[255] = "";
	if (!gameconfs->LoadGameConfigFile("tf2.afk", &g_pGameConf, conf_error, sizeof(conf_error)))
	{
		if (conf_error[0])
		{
			g_pSM->Format(error, maxlength, "Could not read tf2.afk.txt: %s", conf_error);
		}
		return false;
	}

	if (!g_pGameConf->GetOffset("PlayerRunCommand", &bot_playerruncmd_offset))
	{
		snprintf(error, maxlength, "Could not find offset for PlayerRunCmd");
		return false;
	} else {
		SH_MANUALHOOK_RECONFIGURE(PlayerRunCmd, bot_playerruncmd_offset, 0, 0);
	}

	if (!g_pGameConf->GetOffset("Weapon_Equip", &bot_weaponequip_offset))
	{
		snprintf(error, maxlength, "Could not find offset for Weapon_Equip");
		return false;
	}

	if (!g_pGameConf->GetOffset("Weapon_GetSlot", &bot_getweaponslot_offset))
	{
		snprintf(error, maxlength, "Could not find offset for Weapon_GetSlot");
		return false;
	}

	SM_GET_IFACE(SDKTOOLS, g_pSDKTools);
	playerhelpers->AddClientListener(this);
	MathLib_Init();

	CBotGlobals::SetMapName(STRING(gpGlobals->mapname));

	//CNavMeshNavigator::Init(error, maxlen);

	CGameRulesObject::GetGameRules(error, maxlength);

	CWaypointDistances::Reset();

	CWaypoints::Init();
	CWaypoints::Load();

	CBotGlobals::SetMapRunning(true);
	CBotConfigFile::Reset();

	if (mp_teamplay)
		CBotGlobals::SetTeamplay(mp_teamplay->GetBool());
	else
		CBotGlobals::SetTeamplay(false);

	CBotEvents::SetupEvents();

	CBots::MapInit();

	CBotMod *pMod = CBotGlobals::GetCurrentMod();

	if (pMod)
		pMod->MapInit();

	CBotSquads::FreeMemory();

	return true;
}

bool AFKBot::SDK_OnMetamodLoad(ISmmAPI *ismm, char *error, size_t maxlen, bool late)
{
	GET_V_IFACE_CURRENT(GetEngineFactory, enginetrace, IEngineTrace, INTERFACEVERSION_ENGINETRACE_SERVER);
	GET_V_IFACE_CURRENT(GetEngineFactory, gameevents, IGameEventManager2, INTERFACEVERSION_GAMEEVENTSMANAGER2);
	GET_V_IFACE_CURRENT(GetEngineFactory, helpers, IServerPluginHelpers, INTERFACEVERSION_ISERVERPLUGINHELPERS);
	GET_V_IFACE_CURRENT(GetEngineFactory, icvar, ICvar, CVAR_INTERFACE_VERSION);

	GET_V_IFACE_ANY(GetEngineFactory, filesystem, IFileSystem, FILESYSTEM_INTERFACE_VERSION);

	GET_V_IFACE_ANY(GetServerFactory, servergameents, IServerGameEnts, INTERFACEVERSION_SERVERGAMEENTS);
	GET_V_IFACE_ANY(GetServerFactory, gameclients, IServerGameClients, INTERFACEVERSION_SERVERGAMECLIENTS);
	GET_V_IFACE_ANY(GetServerFactory, playerinfomanager, IPlayerInfoManager, INTERFACEVERSION_PLAYERINFOMANAGER);

	GET_V_IFACE_ANY(GetServerFactory, effects, IEffects, IEFFECTS_INTERFACE_VERSION);

#ifndef __linux__
	GET_V_IFACE_CURRENT(GetEngineFactory, debugoverlay, IVDebugOverlay, VDEBUG_OVERLAY_INTERFACE_VERSION);
#endif

	gpGlobals = ismm->GetCGlobals();

	SH_ADD_HOOK_MEMFUNC(IServerGameDLL, LevelInit, gamedll, this, &AFKBot::LevelInit, true);
	SH_ADD_HOOK_MEMFUNC(IServerGameDLL, ServerActivate, gamedll, this, &AFKBot::ServerActivate, true);
	SH_ADD_HOOK_MEMFUNC(IServerGameDLL, GameFrame, gamedll, this, &AFKBot::GameFrame, true);
	SH_ADD_HOOK_MEMFUNC(IServerGameDLL, LevelShutdown, gamedll, this, &AFKBot::LevelShutdown, false);
	SH_ADD_HOOK_MEMFUNC(IServerGameClients, ClientActive, gameclients, this, &AFKBot::ClientActive, true);
	SH_ADD_HOOK_MEMFUNC(IServerGameClients, ClientDisconnect, gameclients, this, &AFKBot::ClientDisconnect, true);
	SH_ADD_HOOK_MEMFUNC(IServerGameClients, ClientPutInServer, gameclients, this, &AFKBot::ClientPutInServer, true);
	SH_ADD_HOOK_MEMFUNC(IGameEventManager2, FireEvent, gameevents, this, &AFKBot::FireGameEvent, false);

#if SOURCE_ENGINE >= SE_ORANGEBOX
	g_pCVar = icvar;
#endif
	CONVAR_REGISTER(this);

	// Read Signatures and Offsets
	CBotGlobals::InitModFolder();
	CBotGlobals::ReadRCBotFolder();

	if (!CBotGlobals::GameStart())
		return false;

	// Initialize bot variables
	CBotProfiles::SetupProfiles();

	CWaypointTypes::Setup();
	CWaypoints::SetupVisibility();

	CBotConfigFile::Reset();
	CBotConfigFile::Load();

	CClassInterface::Init();

	mp_stalemate_enable = g_pCVar->FindVar("mp_stalemate_enable");
	mp_stalemate_meleeonly = g_pCVar->FindVar("mp_stalemate_meleeonly");
	sv_cheats = g_pCVar->FindVar("sv_cheats");
	sv_gravity = g_pCVar->FindVar("sv_gravity");
	mp_friendlyfire = g_pCVar->FindVar("mp_friendlyfire");
	sv_tags = g_pCVar->FindVar("sv_tags");
	mp_teamplay = g_pCVar->FindVar("mp_teamplay");

	if (sv_tags != NULL)
	{
		char sv_tags_str[512];

		strcpy(sv_tags_str, sv_tags->GetString());

		// fix
		if (strstr(sv_tags_str, "afkbot") == NULL)
		{

			if (sv_tags_str[0] == 0)
				strcat(sv_tags_str, "afkbot");
			else
				strcat(sv_tags_str, ",afkbot");

			sv_tags->SetValue(sv_tags_str);

		}
	}

	return true;
}

void AFKBot::SDK_OnAllLoaded()
{
	SM_GET_LATE_IFACE(SDKTOOLS, g_pSDKTools);

	if (g_pSDKTools == NULL)
	{
		g_pSM->LogMessage(myself, "Unable to retrieve the SDKTools interface!");
	}

	// Register natives for Pawn
	sharesys->AddNatives(myself, g_ExtensionNatives);
	sharesys->RegisterLibrary(myself, "afkbot");
}

bool AFKBot::QueryRunning(char *error, size_t maxlength)
{
	SM_CHECK_IFACE(SDKTOOLS, g_pSDKTools);

	return true;
}

void AFKBot::SDK_OnUnload()
{
	gameconfs->CloseGameConfigFile(g_pGameConf);
	playerhelpers->RemoveClientListener(this);

	CBots::FreeAllMemory();
	CStrings::FreeAllMemory();
	CBotMods::FreeMemory();
	CBotEvents::FreeMemory();
	CWaypoints::FreeMemory();
	CWaypointTypes::FreeMemory();
	CBotProfiles::DeleteProfiles();
	CWeapons::FreeMemory();
	CGameRulesObject::Delete();
	//CNavMeshNavigator::FreeAllMemory();
}

bool AFKBot::SDK_OnMetamodUnload(char *error, size_t maxlen)
{
	SH_REMOVE_HOOK_MEMFUNC(IServerGameDLL, LevelInit, gamedll, this, &AFKBot::LevelInit, true);
	SH_REMOVE_HOOK_MEMFUNC(IServerGameDLL, ServerActivate, gamedll, this, &AFKBot::ServerActivate, true);
	SH_REMOVE_HOOK_MEMFUNC(IServerGameDLL, GameFrame, gamedll, this, &AFKBot::GameFrame, true);
	SH_REMOVE_HOOK_MEMFUNC(IServerGameDLL, LevelShutdown, gamedll, this, &AFKBot::LevelShutdown, false);
	SH_REMOVE_HOOK_MEMFUNC(IServerGameClients, ClientActive, gameclients, this, &AFKBot::ClientActive, true);
	SH_REMOVE_HOOK_MEMFUNC(IServerGameClients, ClientDisconnect, gameclients, this, &AFKBot::ClientDisconnect, true);
	SH_REMOVE_HOOK_MEMFUNC(IServerGameClients, ClientPutInServer, gameclients, this, &AFKBot::ClientPutInServer, true);
	SH_REMOVE_HOOK_MEMFUNC(IGameEventManager2, FireEvent, gameevents, this, &AFKBot::FireGameEvent, false);

	ConVar_Unregister();

	return true;
}

bool AFKBot::RegisterConCommandBase(ConCommandBase *pVar)
{
#if defined METAMOD_PLAPI_VERSION
	return g_SMAPI->RegisterConCommandBase(g_PLAPI, pVar);
#else
	return g_SMAPI->RegisterConCmdBase(g_PLAPI, pVar);
#endif
}

static cell_t SetClientAFKBot(IPluginContext *pContext, const cell_t *params)
{
	edict_t *pClient = INDEXENT(params[1]);
	IPlayerInfo *p = playerinfomanager->GetPlayerInfo(pClient);

	if (!bot_enabled.GetBool())
		return pContext->ThrowNativeError("Error: The bot extension is disabled. afkbot_enabled is currently set to off(0)");

	if (p->IsConnected() && !p->IsFakeClient() && !p->IsObserver())
	{
		if (params[2])
			CBots::MakeBot(pClient);
		else if (!params[2])	// Probably don't need elif
			CBots::MakeNotBot(pClient);
	}
	else
		return pContext->ThrowNativeError("Invalid client %s(%d)!\nHe is either not connected, a fake client, or in spectator.", p->GetName(), params[1]);

	return 0;
}

static cell_t IsClientAFKBot(IPluginContext *pContext, const cell_t *params)
{
	cell_t pClient = params[1];
	IPlayerInfo *p = playerinfomanager->GetPlayerInfo(INDEXENT(pClient));

	if (p->IsConnected() && !p->IsFakeClient() && !p->IsObserver())
	{
		if (CBots::Get(pClient)->InUse())
			return 1;
	}
	else
		return pContext->ThrowNativeError("Invalid client %s(%d)!\nHe is either not connected, a fake client, or in spectator.", p->GetName(), params[1]);

	return 0;
}
