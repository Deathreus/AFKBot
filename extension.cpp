/*
 * ================================================================================
 * AFK Bot Extension
 * Copyright (C) 2016 Chris Moore (Deathreus).  All rights reserved.
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

#include "rcbot2/bot_base.h"
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
#include "rcbot2/bot_navmesh.h"

SH_DECL_HOOK6(IServerGameDLL, LevelInit, SH_NOATTRIB, 0, bool, char const *, char const *, char const *, char const *, bool, bool);
//SH_DECL_HOOK1_void(IServerGameDLL, GameFrame, SH_NOATTRIB, 0, bool);
SH_DECL_HOOK0_void(IServerGameDLL, LevelShutdown, SH_NOATTRIB, 0);
SH_DECL_HOOK2(IGameEventManager2, FireEvent, SH_NOATTRIB, 0, bool, IGameEvent *, bool);

SH_DECL_MANUALHOOK2_void(PlayerRunCommand, 0, 0, 0, CUserCmd*, IMoveHelper*);

#if SOURCE_ENGINE >= SE_ORANGEBOX
SH_DECL_HOOK2_void(IServerGameClients, ClientCommand, SH_NOATTRIB, 0, edict_t *, const CCommand &);
#else
SH_DECL_HOOK1_void(IServerGameClients, ClientCommand, SH_NOATTRIB, 0, edict_t *);
#endif

#if SOURCE_ENGINE >= SE_ORANGEBOX
SH_DECL_HOOK1_void(ConCommand, Dispatch, SH_NOATTRIB, false, const CCommand &);
#else
SH_DECL_HOOK0_void(ConCommand, Dispatch, SH_NOATTRIB, false);
#endif

SH_DECL_HOOK1_void(IServerGameClients, SetCommandClient, SH_NOATTRIB, false, int);

AFKBot g_AFKBot;
SMEXT_LINK(&g_AFKBot);

IServerTools *servertools;	// usefull entity finder funcs
IGameEventManager2 *gameevents = NULL;
IServerPluginCallbacks *vsp_callbacks = NULL;
ICvar *icvar = NULL;	// convars
IFileSystem *filesystem = NULL;  // file I/O
IPlayerInfoManager *playerinfomanager = NULL;  // game dll interface to interact with players
IServerPluginHelpers *helpers = NULL;
IServerGameClients* gameclients = NULL;
IEngineSound *engsound = NULL;
IEngineTrace *engtrace = NULL;
IEffects *effects = NULL;
IVDebugOverlay *debugoverlay = NULL;
IServerGameEnts *gameents = NULL; // for accessing the server game entities
INetworkStringTableContainer *netstringtables = NULL;

IGameConfig *g_pGameConf = NULL;
ISDKTools *g_pSDKTools = NULL;	// to retrieve the CGameRules object

CSharedEdictChangeInfo *g_pSharedChangeInfo = NULL;

ConVar bot_version(BOT_VER_CVAR, SMEXT_CONF_VERSION, FCVAR_SPONLY|FCVAR_REPLICATED|FCVAR_DONTRECORD|FCVAR_NOTIFY, BOT_NAME_VER);
ConVar bot_enabled("afkbot_enabled", "1", FCVAR_NOTIFY, "Enable turning players into bots?", true, 0.0f, true, 1.0f);
ConVar bot_tags("afkbot_tag", "0", FCVAR_SPONLY|FCVAR_NOTIFY, "Add a tag onto the server to show it has this extension?", true, 0.0f, true, 1.0f);

int playerruncmd_offset;
IForward *forwardOnAFK = NULL;
extern sp_nativeinfo_t g_ExtensionNatives[];

void AFKBot::PlayerRunCmd(CUserCmd *pCmd, IMoveHelper *pMoveHelper)
{
	if (bot_stop.GetBool())
	{
		RETURN_META(MRES_IGNORED);
	}

	edict_t *pEdict = gameents->BaseEntityToEdict(META_IFACEPTR(CBaseEntity));
	CBot *pBot = CBots::GetBotPointer(pEdict);

	if (pBot && pBot->InUse())
	{
		if (!bot_enabled.GetBool())
		{
			CBots::MakeNotBot(pEdict);
			RETURN_META(MRES_IGNORED);
		}

		CUserCmd *cmd = pBot->GetUserCMD();
		CPlayerState *pls = gameclients->GetPlayerState(pEdict);

		// put the bot's commands into this move frame
		pCmd->buttons = cmd->buttons;
		pCmd->forwardmove = cmd->forwardmove;
		pCmd->sidemove = cmd->sidemove;
		pCmd->upmove = cmd->upmove;
		pCmd->viewangles = cmd->viewangles;
		pCmd->command_number = cmd->command_number;
		pCmd->impulse = cmd->impulse;

		pls->v_angle = cmd->viewangles;
		pls->fixangle = FIXANGLE_ABSOLUTE;
	}

	RETURN_META(MRES_IGNORED);
}

void AFKBot::OnClientPutInServer(int iClient)
{
	CBaseEntity *pEntity = gamehelpers->ReferenceToEntity(iClient);
	CBotGlobals::GetCurrentMod()->PlayerSpawned(pEntity);
	SH_ADD_MANUALHOOK(PlayerRunCommand, pEntity, SH_MEMBER(this, &AFKBot::PlayerRunCmd), false);
}

void AFKBot::OnClientDisconnected(int iClient)
{
	CBaseEntity *pEntity = gamehelpers->ReferenceToEntity(iClient);
	SH_REMOVE_MANUALHOOK(PlayerRunCommand, pEntity, SH_MEMBER(this, &AFKBot::PlayerRunCmd), false);
}

/*void AFKBot::GameFrame(bool simulating)
{
	if (simulating && CBotGlobals::IsMapRunning())
	{
		if (CBotGlobals::NumBots() > 0)
		{
			CBots::BotThink();
			CBotGlobals::GetCurrentMod()->ModFrame();
		}
	}
}*/

bool AFKBot::LevelInit(const char *pMapName,
	char const *pMapEntities,
	char const *pOldLevel,
	char const *pLandmarkName,
	bool loadGame,
	bool background)
{
	// Must set this
	CBotGlobals::SetMapName(pMapName);

	char error[288] = "";
	/*CNavMeshNavigator::Init(error, sizeof(error));
	if (error[0] != '\0')
	{
		smutils->LogError(myself, error);
	}*/
	CGameRulesObject::GetGameRules(error, sizeof(error));
	if (error[0] != '\0')
	{
		smutils->LogError(myself, error);
	}

	CWaypointDistances::Reset();

	CWaypoints::Init();
	CWaypoints::Load();

	CBotGlobals::SetMapRunning(true);

	if (mp_teamplay)
		CBotGlobals::SetTeamplay(mp_teamplay->GetBool());
	else
		CBotGlobals::SetTeamplay(false);

	CBotEvents::SetupEvents();

	CBots::MapInit();

	CBotGlobals::GetCurrentMod()->MapInit();

	CBotSquads::FreeMemory();

	return true;
}

void AFKBot::LevelShutdown()
{
	CBots::FreeMapMemory();
	CWaypoints::Init();
	CWaypointDistances::Save();
	//CNavMeshNavigator::FreeMemory();
	CGameRulesObject::FreeMemory();
	CBotGlobals::SetMapRunning(false);
	CBotEvents::FreeMemory();
}

void AFKBot::FireGameEvent(IGameEvent *pEvent)
{
	if (m_pNavMesh == NULL) return;
	unsigned int iAreaID = pEvent->GetInt("area");

	IList<INavMeshArea*> *areas = m_pNavMesh->GetAreas();
	for (unsigned int i = 0; i < areas->Size(); i++)
	{
		INavMeshArea *area = areas->At(i);
		if (area && area->GetID() == iAreaID)
		{
			bool bBlocked = pEvent->GetBool("blocked");
			area->SetBlocked(bBlocked);
			break;
		}
	}
	return;
}

bool AFKBot::FireEvent(IGameEvent *pEvent, bool bDontBroadcast)
{
	CBotEvents::ExecuteEvent(pEvent, TYPE_IGAMEEVENT);
	RETURN_META_VALUE(MRES_IGNORED, true);
}

bool AFKBot::SDK_OnLoad(char *error, size_t maxlength, bool late)
{
	sharesys->AddDependency(myself, "sdktools.ext", true, true);
	SM_GET_IFACE(SDKTOOLS, g_pSDKTools);

	char conf_error[255] = "";
	if (!gameconfs->LoadGameConfigFile("tf2.afk", &g_pGameConf, conf_error, sizeof(conf_error)))
	{
		if (conf_error[0])
		{
			smutils->Format(error, maxlength, "Could not read tf2.afk.txt: %s", conf_error);
		}
		return false;
	}

	if (!g_pGameConf->GetOffset("PlayerRunCommand", &playerruncmd_offset))
	{
		snprintf(error, maxlength, "Could not find offset for PlayerRunCommand!");
		return false;
	} else {
		SH_MANUALHOOK_RECONFIGURE(PlayerRunCommand, playerruncmd_offset, 0, 0);
	}

	gameconfs->CloseGameConfigFile(g_pGameConf);

	// Register natives for Pawn
	sharesys->AddNatives(myself, g_ExtensionNatives);
	sharesys->RegisterLibrary(myself, "afkbot");

	playerhelpers->AddClientListener(&g_AFKBot);
	playerhelpers->RegisterCommandTargetProcessor(this);

	smutils->AddGameFrameHook(&CBots::BotThink);

	ParamType params[] = { Param_Cell, Param_Cell }; // int client, bool enable
	if ((forwardOnAFK = forwards->CreateForward("OnBotEnable", ET_Hook, 2, params)) == NULL)
		smutils->LogError(myself, "Warning: Unable to initialize OnBotEnable forward");

	CBotGlobals::InitModFolder();
	CBotGlobals::ReadRCBotFolder();

	if (!CBotGlobals::GameStart())
		return false;

	// Initialize bot variables
	CBotProfiles::SetupProfile();

	CWaypointTypes::Setup();
	CWaypoints::SetupVisibility();

	CBotConfigFile::Load();

	// Read Signatures and Offsets
	CClassInterface::Init();

	return true;
}

bool AFKBot::SDK_OnMetamodLoad(ISmmAPI *ismm, char *error, size_t maxlen, bool late)
{
	GET_V_IFACE_CURRENT(GetEngineFactory, netstringtables, INetworkStringTableContainer, INTERFACENAME_NETWORKSTRINGTABLESERVER);
	GET_V_IFACE_CURRENT(GetEngineFactory, engtrace, IEngineTrace, INTERFACEVERSION_ENGINETRACE_SERVER);
	GET_V_IFACE_CURRENT(GetEngineFactory, engsound, IEngineSound, IENGINESOUND_SERVER_INTERFACE_VERSION);
	GET_V_IFACE_CURRENT(GetEngineFactory, gameevents, IGameEventManager2, INTERFACEVERSION_GAMEEVENTSMANAGER2);
	GET_V_IFACE_CURRENT(GetEngineFactory, helpers, IServerPluginHelpers, INTERFACEVERSION_ISERVERPLUGINHELPERS);
	GET_V_IFACE_CURRENT(GetEngineFactory, icvar, ICvar, CVAR_INTERFACE_VERSION);
	GET_V_IFACE_CURRENT(GetFileSystemFactory, filesystem, IFileSystem, FILESYSTEM_INTERFACE_VERSION);
	GET_V_IFACE_CURRENT(GetServerFactory, gameents, IServerGameEnts, INTERFACEVERSION_SERVERGAMEENTS);
	GET_V_IFACE_CURRENT(GetServerFactory, gameclients, IServerGameClients, INTERFACEVERSION_SERVERGAMECLIENTS);
	GET_V_IFACE_CURRENT(GetServerFactory, playerinfomanager, IPlayerInfoManager, INTERFACEVERSION_PLAYERINFOMANAGER);
	GET_V_IFACE_CURRENT(GetServerFactory, servertools, IServerTools, VSERVERTOOLS_INTERFACE_VERSION);
	GET_V_IFACE_CURRENT(GetServerFactory, effects, IEffects, IEFFECTS_INTERFACE_VERSION);

#ifndef __linux__
	GET_V_IFACE_CURRENT(GetEngineFactory, debugoverlay, IVDebugOverlay, VDEBUG_OVERLAY_INTERFACE_VERSION);
#endif

#if SOURCE_ENGINE == SE_CSS || SOURCE_ENGINE == SE_CSGO
	SH_ADD_HOOK(IVEngineServer, ClientCommand, engine, SH_MEMBER(this, &AFKBot::OnSendClientCommand), false);
#endif

#if SOURCE_ENGINE >= SE_ORANGEBOX
	g_pCVar = icvar;
#endif
	CONVAR_REGISTER(this);

	MathLib_Init();

	SH_ADD_HOOK(IServerGameDLL, LevelInit, gamedll, SH_MEMBER(this, &AFKBot::LevelInit), true);
	//SH_ADD_HOOK(IServerGameDLL, GameFrame, gamedll, SH_MEMBER(this, &AFKBot::GameFrame), true);
	SH_ADD_HOOK(IServerGameDLL, LevelShutdown, gamedll, SH_MEMBER(this, &AFKBot::LevelShutdown), false);
	SH_ADD_HOOK(IServerGameClients, ClientCommand, gameclients, SH_MEMBER(this, &AFKBot::OnClientCommand), false);
	SH_ADD_HOOK(IGameEventManager2, FireEvent, gameevents, SH_MEMBER(this, &AFKBot::FireEvent), false);

	ConCommand *say = FindCommand("say");
	ConCommand *team = FindCommand("say_team");

	SH_ADD_HOOK(ConCommand, Dispatch, say, SH_MEMBER(this, &AFKBot::OnSayCommand), false);
	SH_ADD_HOOK(ConCommand, Dispatch, team, SH_MEMBER(this, &AFKBot::OnSayCommand), false);
	SH_ADD_HOOK(IServerGameClients, SetCommandClient, gameclients, SH_MEMBER(this, &AFKBot::OnSetCommandClient), false);

	gameevents->AddListener(this, "nav_blocked", true);
	g_pSharedChangeInfo = engine->GetSharedEdictChangeInfo();

	return true;
}

void AFKBot::SDK_OnAllLoaded()
{
	SM_GET_LATE_IFACE(SDKTOOLS, g_pSDKTools);

	if (g_pSDKTools == NULL)
	{
		smutils->LogError(myself, "Unable to retrieve the SDKTools interface!");
	}

	mp_stalemate_enable = g_pCVar->FindVar("mp_stalemate_enable");
	mp_stalemate_meleeonly = g_pCVar->FindVar("mp_stalemate_meleeonly");
	sv_cheats = g_pCVar->FindVar("sv_cheats");
	sv_gravity = g_pCVar->FindVar("sv_gravity");
	mp_friendlyfire = g_pCVar->FindVar("mp_friendlyfire");
	sv_tags = g_pCVar->FindVar("sv_tags");
	mp_teamplay = g_pCVar->FindVar("mp_teamplay");

	if (sv_tags != NULL && bot_tags.GetBool())
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
}

bool AFKBot::QueryRunning(char *error, size_t maxlength)
{
	if (!bot_enabled.GetBool())
	{
		snprintf(error, maxlength, "AFK Bot is not enabled.");
		return false;
	}

	SM_CHECK_IFACE(SDKTOOLS, g_pSDKTools);

	if (m_pGameRules == NULL)
	{
		snprintf(error, maxlength, "The game rules object is invalid.");
		return false;
	}

	if (m_pNavMesh == NULL)
	{
		snprintf(error, maxlength, "The navmesh object is invalid.");
		return false;
	}

	return true;
}

void AFKBot::SDK_OnUnload()
{
	playerhelpers->RemoveClientListener(&g_AFKBot);
	playerhelpers->UnregisterCommandTargetProcessor(this);

	smutils->RemoveGameFrameHook(&CBots::BotThink);

	forwards->ReleaseForward(forwardOnAFK);

	gameevents->RemoveListener(this);

	SH_REMOVE_HOOK(IServerGameDLL, LevelInit, gamedll, SH_MEMBER(this, &AFKBot::LevelInit), true);
	//SH_REMOVE_HOOK(IServerGameDLL, GameFrame, gamedll, SH_MEMBER(this, &AFKBot::GameFrame), true);
	SH_REMOVE_HOOK(IServerGameDLL, LevelShutdown, gamedll, SH_MEMBER(this, &AFKBot::LevelShutdown), false);
	SH_REMOVE_HOOK(IServerGameClients, ClientCommand, gameclients, SH_MEMBER(this, &AFKBot::OnClientCommand), false);
	SH_REMOVE_HOOK(IGameEventManager2, FireEvent, gameevents, SH_MEMBER(this, &AFKBot::FireEvent), false);

	ConCommand *say = FindCommand("say");
	ConCommand *team = FindCommand("say_team");

	SH_REMOVE_HOOK(ConCommand, Dispatch, say, SH_MEMBER(this, &AFKBot::OnSayCommand), false);
	SH_REMOVE_HOOK(ConCommand, Dispatch, team, SH_MEMBER(this, &AFKBot::OnSayCommand), false);
	SH_REMOVE_HOOK(IServerGameClients, SetCommandClient, gameclients, SH_MEMBER(this, &AFKBot::OnSetCommandClient), false);

#if SOURCE_ENGINE == SE_CSS || SOURCE_ENGINE == SE_CSGO
	SH_REMOVE_HOOK(IVEngineServer, ClientCommand, engine, SH_MEMBER(this, &AFKBot::OnSendClientCommand), false);
#endif

	ConVar_Unregister();

	CBots::FreeAllMemory();
	CStrings::FreeAllMemory();
	CBotMods::FreeMemory();
	CBotEvents::FreeMemory();
	CWaypoints::FreeMemory();
	CWaypointTypes::FreeMemory();
	CBotProfiles::DeleteProfiles();
	CWeapons::FreeMemory();
	CGameRulesObject::FreeMemory();
	CNavMeshNavigator::FreeMemory();
}

bool AFKBot::SDK_OnMetamodUnload(char *error, size_t maxlen)
{
	return true;
}

bool AFKBot::ProcessCommandTarget(cmd_target_info_t *info)
{
	IGamePlayer *pAdmin;

	if ((info->flags & COMMAND_FILTER_NO_MULTI) == COMMAND_FILTER_NO_MULTI)
	{
		return false;
	}

	if (info->admin)
	{
		if ((pAdmin = playerhelpers->GetGamePlayer(info->admin)) == NULL)
		{
			return false;
		}
		if (!pAdmin->IsInGame())
		{
			return false;
		}
	}
	else
	{
		pAdmin = NULL;
	}

	if (!strcmp(info->pattern, "@afk"))
	{
		info->num_targets = 0;
		for (int i = gpGlobals->maxClients; i > 0; i--)
		{
			IGamePlayer *pPlayer = playerhelpers->GetGamePlayer(i);
			if (pPlayer == NULL || !pPlayer->IsInGame())
				continue;

			IPlayerInfo *pInfo = pPlayer->GetPlayerInfo();
			if (pInfo == NULL)
				continue;

			if (playerhelpers->FilterCommandTarget(pAdmin, pPlayer, info->flags) == COMMAND_TARGET_VALID)
			{
				if (CBots::Get(i)->InUse())
					info->targets[info->num_targets++] = i;
			}
		}

		info->reason = info->num_targets > 0 ? COMMAND_TARGET_VALID : COMMAND_TARGET_EMPTY_FILTER;

		info->target_name_style = COMMAND_TARGETNAME_RAW;
		snprintf(info->target_name, info->target_name_maxlength, "all afk bots");
	}

	return true;
}

#if SOURCE_ENGINE >= SE_ORANGEBOX
void AFKBot::OnClientCommand(edict_t *pClient, const CCommand &args)
#else
void AFKBot::OnClientCommand(edict_t *pEntity)
#endif
{
#if SOURCE_ENGINE <= SE_DARKMESSIAH
	CCommand args;
#endif

	if (!pClient || pClient->IsFree())
		return;

	// Capture voice commands
	CBotGlobals::GetCurrentMod()->ClientCommand(pClient, args.ArgC(), args.Arg(0), args.Arg(1), args.Arg(2));
	RETURN_META(MRES_IGNORED);
}

#if SOURCE_ENGINE >= SE_ORANGEBOX
void AFKBot::OnSayCommand(const CCommand &args)
#else
void AFKBot::OnSayCommand()
#endif
{
#if SOURCE_ENGINE <= SE_DARKMESSIAH
	CCommand args;
#endif

	const char *pCmd = args.Arg(1);
	if (this->m_iCommandClient > 0 && this->m_iCommandClient <= MAX_PLAYERS)
	{
		bool bIsSilent;
		if (strcont(pCmd, "afk") != -1 || strcont(pCmd, "back") != -1)
		{
			edict_t *pClient = INDEXENT(this->m_iCommandClient);
			if (!pClient || pClient->IsFree())
				return;

			IGamePlayer *pPlayer = playerhelpers->GetGamePlayer(this->m_iCommandClient);
			if (pPlayer == NULL || !pPlayer->IsInGame())
			{
				CBotGlobals::PrintToChat(0, "[SM] Command is ingame only.");
				return;
			}

			if (!strcmp(pCmd, "!back"))
			{
				bIsSilent = false;
				if (CBots::Get(pClient)->InUse())
					CBots::MakeNotBot(pClient);
			}
			else if (!strcmp(pCmd, "/back"))
			{
				bIsSilent = true;
				if (CBots::Get(pClient)->InUse())
					CBots::MakeNotBot(pClient);
			}
			else if (!strcmp(pCmd, "!afk"))
			{
				bIsSilent = false;
				if (!adminsys->CheckClientCommandAccess(this->m_iCommandClient, "sm_afk", ADMFLAG_RESERVATION))
				{
					CBotGlobals::PrintToChat(this->m_iCommandClient, "[SM] You do not have access to this commmand.");
					RETURN_META(MRES_IGNORED);
				}

				if (!CBots::Get(pClient)->InUse())
					CBots::MakeBot(pClient);
				else
					CBots::MakeNotBot(pClient);
			}
			else if (!strcmp(pCmd, "/afk"))
			{
				bIsSilent = true;
				if (!adminsys->CheckClientCommandAccess(this->m_iCommandClient, "sm_afk", ADMFLAG_RESERVATION))
				{
					CBotGlobals::PrintToChat(this->m_iCommandClient, "[SM] You do not have access to this commmand.");
					RETURN_META(MRES_SUPERCEDE);
				}

				if (!CBots::Get(pClient)->InUse())
					CBots::MakeBot(pClient);
				else
					CBots::MakeNotBot(pClient);
			}

			RETURN_META(bIsSilent ? MRES_SUPERCEDE : MRES_IGNORED);
		}
	}

	RETURN_META(MRES_IGNORED);
}

void AFKBot::OnSetCommandClient(int client)
{
	this->m_iCommandClient = client + 1;
}

#if SOURCE_ENGINE == SE_CSS || SOURCE_ENGINE == SE_CSGO
void AFKBot::OnSendClientCommand(edict_t *pPlayer, const char *szFormat)
{
	// Due to legacy code, CS:S and CS:GO still sends "name \"newname\"" to the
	// client after aname change. The engine has a change hook on name causing
	// it to reset to the player's Steam name. This quashes that to make
	// SetClientName work properly.
	if (!strncmp(szFormat, "name ", 5))
	{
		RETURN_META(MRES_SUPERCEDE);
	}

	RETURN_META(MRES_IGNORED);
}
#endif

bool AFKBot::RegisterConCommandBase(ConCommandBase *pVar)
{
#if defined METAMOD_PLAPI_VERSION
	return g_SMAPI->RegisterConCommandBase(g_PLAPI, pVar);
#else
	return g_SMAPI->RegisterConCmdBase(g_PLAPI, pVar);
#endif
}

// Stupid shim required for CBaseEdict::StateChanged to work
IChangeInfoAccessor *CBaseEdict::GetChangeAccessor()
{
	return engine->GetChangeAccessor((const edict_t *)this);
}
