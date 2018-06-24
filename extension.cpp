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
#include "rcbot2/bot_getprop.h"
#include "rcbot2/bot_fortress.h"
#include "rcbot2/bot_event.h"
#include "rcbot2/bot_weapons.h"
#include "rcbot2/bot_gamerules.h"

#if defined USE_NAVMESH
#include "rcbot2/bot_navmesh.h"
#else
#include "rcbot2/bot_waypoint.h"
#include "rcbot2/bot_waypoint_visibility.h"
#include "rcbot2/bot_wpt_dist.h"
#endif // USE_NAVMESH

SH_DECL_HOOK1_void(IServerGameDLL, GameFrame, SH_NOATTRIB, 0, bool);

SH_DECL_HOOK2(IGameEventManager2, FireEvent, SH_NOATTRIB, 0, bool, IGameEvent *, bool);

SH_DECL_MANUALHOOK2_void(PlayerRunCommand, 0, 0, 0, CUserCmd *, IMoveHelper *);

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

IServerTools *servertools = NULL;	// usefull entity finder funcs
IGameEventManager2 *gameevents = NULL;
IServerPluginCallbacks *vsp_callbacks = NULL;
ICvar *icvar = NULL;	// convars
IFileSystem *filesystem = NULL;
IPlayerInfoManager *playerinfomanager = NULL;
IServerPluginHelpers *helpers = NULL;
IServerGameClients* gameclients = NULL;
IEngineSound *engsound = NULL;
IEngineTrace *engtrace = NULL;
IEffects *effects = NULL;	// particle effects
IVDebugOverlay *debugoverlay = NULL;
IServerGameEnts *gameents = NULL;

IGameConfig *g_pGameConf = NULL;
IBinTools *g_pBinTools = NULL;	// to create calls to Valve functions
ISDKTools *g_pSDKTools = NULL;	// to retrieve the CGameRules pointer

ConVar bot_version("afkbot_version", SMEXT_CONF_VERSION, FCVAR_SPONLY|FCVAR_REPLICATED|FCVAR_DONTRECORD|FCVAR_NOTIFY, BOT_NAME_VER);
ConVar bot_enabled("afkbot_enabled", "1", FCVAR_NOTIFY, "Enable turning players into bots?", true, 0.0f, true, 1.0f);
ConVar bot_tags("afkbot_tag", "0", FCVAR_SPONLY|FCVAR_NOTIFY, "Add a tag onto the server to show it has this extension?", true, 0.0f, true, 1.0f);
ConVar bot_debug("afkbot_debug_enable", "0", FCVAR_SPONLY|FCVAR_NOTIFY, "Enable verbose debugging?", true, 0.0f, true, 1.0f, &DebugValueChanged);
ConVar bot_log_continuous("afkbot_log_continuous", "1", 0, "Should the log file be cleared ever map change?", true, 0.0f, true, 1.0f);

IForward *forwardOnAFK = NULL;

void DebugValueChanged(IConVar *var, const char *pOldValue, float flOldValue)
{
	if (bot_debug.GetBool())
		AFKBot::BeginLogging(true);
	else
		AFKBot::EndLogging();
}

void AFKBot::PlayerRunCmd(CUserCmd *pCmd, IMoveHelper *pMoveHelper)
{
	if (bot_stop.GetBool())
	{
		RETURN_META(MRES_IGNORED);
	}

	static CUserCmd *cmd;
	static CPlayerState *pls;
	static CBaseEntity *pEntity;
	static edict_t *pEdict;
	static CBot *pBot;

	pEntity = META_IFACEPTR(CBaseEntity);
	pEdict = gameents->BaseEntityToEdict(pEntity);
	pBot = CBots::GetBotPointer(pEdict);

	if (pBot && pBot->InUse() && pBot->IsAlive())
	{
		if (!bot_enabled.GetBool())
		{
			CBots::MakeNotBot(pEdict);
			RETURN_META(MRES_IGNORED);
		}

		cmd = pBot->GetUserCMD();
		pls = gameclients->GetPlayerState(pEdict);

		// put the bot's commands into this move frame
		pCmd->buttons = cmd->buttons;
		pCmd->forwardmove = cmd->forwardmove;
		pCmd->sidemove = cmd->sidemove;
		pCmd->upmove = cmd->upmove;
		pCmd->viewangles = cmd->viewangles;
		pCmd->impulse = cmd->impulse;

		pls->v_angle = cmd->viewangles;
		pls->fixangle = FIXANGLE_ABSOLUTE;
	}

	RETURN_META(MRES_IGNORED);
}

void AFKBot::OnClientPutInServer(int iClient)
{
	CBaseEntity *pEntity = gameents->EdictToBaseEntity(INDEXENT(iClient));
	CBotGlobals::GetCurrentMod()->PlayerSpawned(pEntity);
	SH_ADD_MANUALHOOK(PlayerRunCommand, pEntity, SH_MEMBER(this, &AFKBot::PlayerRunCmd), false);
}

void AFKBot::OnClientDisconnecting(int iClient)
{
	CBaseEntity *pEntity = gameents->EdictToBaseEntity(INDEXENT(iClient));
	CBots::MakeNotBot(gameents->BaseEntityToEdict(pEntity));
	SH_REMOVE_MANUALHOOK(PlayerRunCommand, pEntity, SH_MEMBER(this, &AFKBot::PlayerRunCmd), false);
}

void AFKBot::GameFrame(bool simulating)
{
	if (CBotGlobals::IsMapRunning())
	{
		CBots::BotThink(simulating);
	}
}

void AFKBot::OnCoreMapStart(edict_t *pEdictList, int edictCount, int clientMax)
{
	// Must set this
	CBotGlobals::SetMapName(gpGlobals->mapname.ToCStr());
	CBotGlobals::SetMapRunning(true);

	char error[288] = "";
#if defined USE_NAVMESH
	g_pNavMesh = CNavMeshLoader::Load(error, sizeof(error));
	if (error[0] && *error)
	{
		smutils->LogError(myself, error);
		CBotGlobals::SetMapRunning(false);
		return;
	}
#else
	CWaypointDistances::Reset();
	CWaypoints::Init();
	CWaypoints::Load();
#endif

	CGameRulesObject::GetGameRules(error, sizeof(error));
	if (error[0] && *error)
	{
		smutils->LogError(myself, error);
	}

	if (mp_teamplay)
		CBotGlobals::SetTeamplay(mp_teamplay->GetBool());
	else
		CBotGlobals::SetTeamplay(false);

	CBotEvents::SetupEvents();

	CBots::MapInit();
	CBotGlobals::GetCurrentMod()->MapInit();

	if (bot_debug.GetBool())
	{
		static bool continuous = !bot_log_continuous.GetBool();
		BeginLogging(continuous);
	}
}

void AFKBot::OnCoreMapEnd()
{
#if defined USE_NAVMESH
	if (g_pNavMesh)
		delete g_pNavMesh;
#else
	CWaypoints::Init();
	CWaypointDistances::Save();
#endif

	CBotGlobals::SetMapRunning(false);
	CBots::FreeMapMemory();
	CBotEvents::FreeMemory();

	if (bot_debug.GetBool())
	{
		EndLogging();
	}
}

bool AFKBot::FireEvent(IGameEvent *pEvent, bool bDontBroadcast)
{
#if defined USE_NAVMESH
	if (!strcasecmp(pEvent->GetName(), "nav_blocked"))
	{
		if (g_pNavMesh == NULL) return;
		unsigned int iAreaID = pEvent->GetInt("area");
		bool bBlocked = pEvent->GetBool("blocked");

		INavMeshArea *area = g_pNavMesh->GetAreaByID(iAreaID);
		if(area) area->SetBlocked(bBlocked);

		DebugMessage("Area %i became %sblocked", iAreaID, !bBlocked ? "un":"");

		RETURN_META_VALUE(MRES_IGNORED, true);
	}
#endif

	CBotEvents::ExecuteEvent(pEvent, TYPE_IGAMEEVENT);
	RETURN_META_VALUE(MRES_IGNORED, true);
}

bool AFKBot::SDK_OnLoad(char *error, size_t maxlength, bool late)
{
	sharesys->AddDependency(myself, "sdktools.ext", true, true);
	SM_GET_IFACE(SDKTOOLS, g_pSDKTools);

	sharesys->AddDependency(myself, "bintools.ext", true, true);
	SM_GET_IFACE(BINTOOLS, g_pBinTools);

	char conf_error[255] = "";
	if (!gameconfs->LoadGameConfigFile("afk.games", &g_pGameConf, conf_error, sizeof(conf_error)))
	{
		if (conf_error[0])
		{
			smutils->Format(error, maxlength, "Could not read afk.games.txt: %s", conf_error);
		}
		return false;
	}

	int iOffset;
	if (!g_pGameConf->GetOffset("PlayerRunCommand", &iOffset))
	{
		snprintf(error, maxlength, "Could not find offset for PlayerRunCommand!");
		return false;
	} else {
		SH_MANUALHOOK_RECONFIGURE(PlayerRunCommand, iOffset, 0, 0);
	}

	// Register natives for Pawn
	extern sp_nativeinfo_t g_ExtensionNatives[];
	sharesys->AddNatives(myself, g_ExtensionNatives);
	sharesys->RegisterLibrary(myself, "AFKBot");

	playerhelpers->AddClientListener(this);
	playerhelpers->RegisterCommandTargetProcessor(this);

	ParamType params[] = { Param_Cell, Param_Cell }; // int client, bool enabled
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

#if SOURCE_ENGINE >= SE_ORANGEBOX
	g_pCVar = icvar;
#endif
	CONVAR_REGISTER(this);

	MathLib_Init();

	SH_ADD_HOOK(IServerGameDLL, GameFrame, gamedll, SH_MEMBER(this, &AFKBot::GameFrame), true);
	SH_ADD_HOOK(IServerGameClients, ClientCommand, gameclients, SH_MEMBER(this, &AFKBot::OnClientCommand), false);
	SH_ADD_HOOK(IServerGameClients, SetCommandClient, gameclients, SH_MEMBER(this, &AFKBot::OnSetCommandClient), false);
	SH_ADD_HOOK(IGameEventManager2, FireEvent, gameevents, SH_MEMBER(this, &AFKBot::FireEvent), false);

	ConCommand *say = FindCommand("say"); ConCommand *team = FindCommand("say_team");
	SH_ADD_HOOK(ConCommand, Dispatch, say, SH_MEMBER(this, &AFKBot::OnSayCommand), false);
	SH_ADD_HOOK(ConCommand, Dispatch, team, SH_MEMBER(this, &AFKBot::OnSayCommand), false);

	g_pSharedChangeInfo = engine->GetSharedEdictChangeInfo();

	return true;
}

void AFKBot::SDK_OnAllLoaded()
{
	SM_GET_LATE_IFACE(BINTOOLS, g_pBinTools);
	SM_GET_LATE_IFACE(SDKTOOLS, g_pSDKTools);

	mp_stalemate_enable = icvar->FindVar("mp_stalemate_enable");
	mp_stalemate_meleeonly = icvar->FindVar("mp_stalemate_meleeonly");
	sv_cheats = icvar->FindVar("sv_cheats");
	sv_gravity = icvar->FindVar("sv_gravity");
	mp_friendlyfire = icvar->FindVar("mp_friendlyfire");
	sv_tags = icvar->FindVar("sv_tags");
	mp_teamplay = icvar->FindVar("mp_teamplay");

	if (sv_tags && bot_tags.GetBool())
	{
		char sv_tags_str[512];
		strcpy_s(sv_tags_str, sv_tags->GetString());

		if (strstr(sv_tags_str, "afkbot") == NULL)
		{
			if (sv_tags_str[0] == '\0')
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
		ke::SafeStrcpy(error, maxlength, "AFK Bot is not enabled.");
		return false;
	}

	SM_CHECK_IFACE(SDKTOOLS, g_pSDKTools);

	if (g_pGameRules == NULL)
	{
		ke::SafeStrcpy(error, maxlength, "The game rules object is invalid.");
		return false;
	}

#if defined USE_NAVMESH
	if (g_pNavMesh == NULL)
	{
		ke::SafeStrcpy(error, maxlength, "The navmesh object is invalid.");
		return false;
	}
#endif

	return true;
}

void AFKBot::SDK_OnUnload()
{
	playerhelpers->RemoveClientListener(this);
	playerhelpers->UnregisterCommandTargetProcessor(this);

	gameconfs->CloseGameConfigFile(g_pGameConf);

	forwards->ReleaseForward(forwardOnAFK);

	SH_REMOVE_HOOK(IServerGameDLL, GameFrame, gamedll, SH_MEMBER(this, &AFKBot::GameFrame), true);
	SH_REMOVE_HOOK(IServerGameClients, ClientCommand, gameclients, SH_MEMBER(this, &AFKBot::OnClientCommand), false);
	SH_REMOVE_HOOK(IServerGameClients, SetCommandClient, gameclients, SH_MEMBER(this, &AFKBot::OnSetCommandClient), false);
	SH_REMOVE_HOOK(IGameEventManager2, FireEvent, gameevents, SH_MEMBER(this, &AFKBot::FireEvent), false);

	ConCommand *say = FindCommand("say"); ConCommand *team = FindCommand("say_team");
	SH_REMOVE_HOOK(ConCommand, Dispatch, say, SH_MEMBER(this, &AFKBot::OnSayCommand), false);
	SH_REMOVE_HOOK(ConCommand, Dispatch, team, SH_MEMBER(this, &AFKBot::OnSayCommand), false);

	ConVar_Unregister();

	CBots::FreeAllMemory();
	CStrings::FreeAllMemory();
	CBotMods::FreeMemory();
	CBotEvents::FreeMemory();
	CBotProfiles::DeleteProfiles();
	CWeapons::FreeMemory();

#if defined USE_NAVMESH
	if (g_pNavesh)
		delete g_pNavMesh;
#else
	CWaypoints::FreeMemory();
	CWaypointTypes::FreeMemory();
#endif
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
		for (int i = gpGlobals->maxClients; i; --i)
		{
			IGamePlayer *pPlayer = playerhelpers->GetGamePlayer(i);
			if (pPlayer == NULL || !pPlayer->IsInGame())
				continue;

			IPlayerInfo *pInfo = pPlayer->GetPlayerInfo();
			if (pInfo == NULL)
				continue;

			if (playerhelpers->FilterCommandTarget(pAdmin, pPlayer, info->flags) == COMMAND_TARGET_VALID)
			{
				CBot *pBot = CBots::GetBotPointer(INDEXENT(i));
				if (pBot && pBot->InUse())
					info->targets[info->num_targets++] = i;
			}
		}

		info->reason = info->num_targets > 0 ? COMMAND_TARGET_VALID : COMMAND_TARGET_EMPTY_FILTER;

		info->target_name_style = COMMAND_TARGETNAME_RAW;
		ke::SafeStrcpy(info->target_name, info->target_name_maxlength, "all afk bots");
	}

	return true;
}

void AFKBot::OnClientCommand(edict_t *pClient, const CCommand &args)
{
	if (!pClient || pClient->IsFree())
		return;

	if (CBotGlobals::m_pCommands->IsCommand(args.Arg(0)))
	{
		eBotCommandResult iResult = CBotGlobals::m_pCommands->Execute(pClient, args.Arg(1), args.Arg(2), args.Arg(3), args.Arg(4), args.Arg(5), args.Arg(6));

		if (iResult == COMMAND_ACCESSED)
		{
			// ok
		}
		else if (iResult == COMMAND_REQUIRE_ACCESS)
		{
			CBotGlobals::BotMessage(pClient, 0, "You do not have access to this command");
		}
		else if (iResult == COMMAND_NOT_FOUND)
		{
			CBotGlobals::BotMessage(pClient, 0, "bot command not found");
		}
		else if (iResult == COMMAND_ERROR)
		{
			CBotGlobals::BotMessage(pClient, 0, "bot command returned an error");
		}

		RETURN_META(MRES_SUPERCEDE);
	}

	// Capture voice commands
	CBotGlobals::GetCurrentMod()->ClientCommand(pClient, args.ArgC(), args.Arg(0), args.Arg(1), args.Arg(2));
	RETURN_META(MRES_IGNORED);
}

void AFKBot::OnSayCommand(const CCommand &args)
{
	const char *pCmd = args.Arg(1);
	if (this->m_iCommandClient > 0 && this->m_iCommandClient < MAX_PLAYERS)
	{
		bool bIsSilent = false;
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

			CBot *pBot = CBots::GetBotPointer(pClient);

			if (!strcmp(pCmd, "!back"))
			{
				if (pBot && pBot->InUse())
					CBots::MakeNotBot(pClient);
			}
			else if (!strcmp(pCmd, "/back"))
			{
				bIsSilent = true;
				if (pBot && pBot->InUse())
					CBots::MakeNotBot(pClient);
			}
			else if (!strcmp(pCmd, "!afk"))
			{
				if (!adminsys->CheckClientCommandAccess(this->m_iCommandClient, "sm_afk", ADMFLAG_RESERVATION))
				{
					CBotGlobals::PrintToChat(this->m_iCommandClient, "[SM] You do not have access to this commmand.");
					RETURN_META(MRES_IGNORED);
				}

				if (pBot && pBot->InUse())
					CBots::MakeNotBot(pClient);
				else
					CBots::MakeBot(pClient);
			}
			else if (!strcmp(pCmd, "/afk"))
			{
				bIsSilent = true;
				if (!adminsys->CheckClientCommandAccess(this->m_iCommandClient, "sm_afk", ADMFLAG_RESERVATION))
				{
					CBotGlobals::PrintToChat(this->m_iCommandClient, "[SM] You do not have access to this commmand.");
					RETURN_META(MRES_SUPERCEDE);
				}

				if (pBot && pBot->InUse())
					CBots::MakeNotBot(pClient);
				else
					CBots::MakeBot(pClient);
			}

			RETURN_META((bIsSilent) ? MRES_SUPERCEDE : MRES_IGNORED);
		}
	}

	RETURN_META(MRES_IGNORED);
}

void AFKBot::OnSetCommandClient(int client)
{
	this->m_iCommandClient = client + 1;
}

bool AFKBot::RegisterConCommandBase(ConCommandBase *pVar)
{
#if defined METAMOD_PLAPI_VERSION
	return g_SMAPI->RegisterConCommandBase(g_PLAPI, pVar);
#else
	return g_SMAPI->RegisterConCmdBase(g_PLAPI, pVar);
#endif
}

void AFKBot::DebugMessage(const char *fmt, ...)
{
	if (bot_debug.GetBool() && !ferror(m_pLogFile))
	{
		char *argptr; char message[256];

		va_start(argptr, fmt);
		smutils->FormatArgs(message, 255, fmt, argptr);
		va_end(argptr);

		char buffer[32];
		time_t dt = smutils->GetAdjustedTime();
		tm *curTime = localtime(&dt);
		strftime(buffer, sizeof(buffer), "%m/%d/%Y - %H:%M:%S", curTime);

		fprintf(m_pLogFile, "L %s: %s\n", buffer, message);
		fflush(m_pLogFile);
	}
}

void AFKBot::BeginLogging(bool bReset)
{
	static char sPath[MAX_PATH];
	if (!sPath[0]) smutils->BuildPath(Path_SM, sPath, sizeof(sPath), "logs\\afkbot.log");

	if (bReset)
	{
		FILE *pLogFile = fopen(sPath, "w+");
		if (pLogFile)
		{
			smutils->LogMessage(myself, "Begin logging in '%s'", sPath);
			m_pLogFile = pLogFile;
		}
	}
	else
	{
		FILE *pLogFile = fopen(sPath, "at");
		if (pLogFile)
		{
			smutils->LogMessage(myself, "Begin logging in '%s'", sPath);
			m_pLogFile = pLogFile;
		}
	}
}

void AFKBot::EndLogging()
{
	smutils->LogMessage(myself, "End logging session");
	fclose(m_pLogFile);
}