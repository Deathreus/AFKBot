/*
 * ================================================================================
 * AFK Bot Extension
 * Copyright (C) 2018 Chris Moore (Deathreus).  All rights reserved.
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
#include "rcbot2/bot_events.h"
#include "rcbot2/bot_weapons.h"
#include "rcbot2/bot_gamerules.h"

#if defined USE_NAVMESH
#include "rcbot2/bot_navmesh.h"
#else
#include "rcbot2/bot_waypoint.h"
#include "rcbot2/bot_waypoint_visibility.h"
#include "rcbot2/bot_wpt_dist.h"
#endif // USE_NAVMESH

#define GAMERULES_FRAME_ACTION \
	[](void *) -> void {											\
		char error[288] = "";										\
		if(!CGameRulesObject::GetGameRules(error, sizeof(error)))	\
		{															\
			smutils->LogError(myself, error);						\
			CBotGlobals::SetMapRunning(false);						\
		}															\
	}

SH_DECL_HOOK1_void(IServerGameDLL, GameFrame, SH_NOATTRIB, 0, bool)

SH_DECL_MANUALHOOK2_void(PlayerRunCommand, 0, 0, 0, CUserCmd *, IMoveHelper *)

#if SOURCE_ENGINE >= SE_ORANGEBOX
SH_DECL_HOOK2_void(IServerGameClients, ClientCommand, SH_NOATTRIB, 0, edict_t *, const CCommand &)
SH_DECL_HOOK2_void(IServerPluginHelpers, ClientCommand, SH_NOATTRIB, 0, edict_t *, const char *)
#else
SH_DECL_HOOK1_void(IServerGameClients, ClientCommand, SH_NOATTRIB, 0, edict_t *)
SH_DECL_HOOK1_void(IServerPluginHelpers, ClientCommand, SH_NOATTRIB, 0, edict_t *)
#endif

SMEXT_EXPOSE(AFKBot)
FILE *AFKBot::m_pLogFile = NULL;

IServerTools *servertools = NULL;	// usefull entity finder funcs
IGameEventManager2 *gameevents = NULL;
IServerPluginCallbacks *vsp_callbacks = NULL;
ICvar *icvar = NULL;	// convars
IFileSystem *filesystem = NULL;
IPlayerInfoManager *playerinfomanager = NULL;
IServerPluginHelpers *helpers = NULL;
IServerGameClients* gameclients = NULL;
IEngineSound *engsound = NULL;
IEngineTrace *engtrace = NULL;	// ray tracing
IEffects *effects = NULL;	// particle effects
IServerGameEnts *gameents = NULL;
IVDebugOverlay *debugoverlay = NULL;

IGameConfig *g_pGameConf = NULL;
IBinTools *g_pBinTools = NULL;	// to create calls to Valve functions
ISDKTools *g_pSDKTools = NULL;	// to retrieve the CGameRules pointer

IForward *forwardOnAFK = NULL;

void EnabledValueChanged(IConVar *var, const char *pOldValue, float flOldValue);
void DebugValueChanged(IConVar *var, const char *pOldValue, float flOldValue);

ConVar bot_version("afkbot_version", SMEXT_CONF_VERSION, FCVAR_SPONLY|FCVAR_REPLICATED|FCVAR_DONTRECORD|FCVAR_NOTIFY, BOT_NAME_VER);
ConVar bot_enabled("afkbot_enabled", "1", 0, "Enable turning players into bots?", true, 0.0f, true, 1.0f, &EnabledValueChanged);
ConVar bot_tags("afkbot_tag", "0", FCVAR_SPONLY|FCVAR_NOTIFY, "Add a tag onto the server to show it has this extension?", true, 0.0f, true, 1.0f);
ConVar bot_debug("afkbot_debug_enable", "0", FCVAR_HIDDEN|FCVAR_NOTIFY, "Enable debugging? Writes to a log file.", true, 0.0f, true, 1.0f, &DebugValueChanged);
ConVar bot_debug_verbose("afkbot_debug_verbose", "0", FCVAR_HIDDEN|FCVAR_NOTIFY, "Turn on verbose debugging? This will make the log file real big real fast", true, 0.0f, true, 1.0f);
ConVar bot_log_resets("afkbot_log_resets", "1", 0, "Should the log file be cleared every map change?", true, 0.0f, true, 1.0f);

void EnabledValueChanged(IConVar *var, const char *pOldValue, float flOldValue)
{
	if(!bot_enabled.GetBool())
		CBotGlobals::SetMapRunning(false);
}

void DebugValueChanged(IConVar *var, const char *pOldValue, float flOldValue)
{
	if(bot_debug.GetBool())
		AFKBot::BeginLogging(bot_log_resets.GetBool());
	else if( (int)flOldValue == 1 )
		AFKBot::EndLogging();
}

#if SOURCE_ENGINE >= SE_ORANGEBOX
#define HELPER_ARGS edict_t *pClient, const char *args
#else
#define HELPER_ARGS edict_t *pClient
#endif

static float s_fNextClientCommand[MAX_PLAYERS+1];
void RateLimitedClientCommand(HELPER_ARGS)
{
	if(bot_enabled.GetBool())
	{
		if(s_fNextClientCommand[ SlotOfEdict(pClient) ] > gpGlobals->curtime)
			RETURN_META(MRES_SUPERCEDE);

		s_fNextClientCommand[ SlotOfEdict(pClient) ] = gpGlobals->curtime + 0.33;
	}

	RETURN_META(MRES_IGNORED);
}

#if SOURCE_ENGINE >= SE_ORANGEBOX
#define COMMAND_ARGS edict_t *pClient, const CCommand &args
#else
class ECommand
{
public:
	ECommand() { Assert( engine ); }
	const char *Arg(int n) const { return engine->Cmd_Argv(n); }
	int ArgC() const { return engine->Cmd_Argc(); }
	const char *ArgS() const { return engine->Cmd_Args(); }
	const char *operator[](int n) const { return this->Arg(n); }
};

#define COMMAND_ARGS edict_t *pClient
#endif

void OnClientCommand(COMMAND_ARGS)
{
	if(!pClient || pClient->IsFree())
		return;

#if SOURCE_ENGINE < SE_ORANGEBOX
	ECommand args;
#endif

	if(CBotGlobals::m_pCommands->IsCommand(args[0]))
	{
		eBotCommandResult iResult = CBotGlobals::m_pCommands->Execute(pClient, args[1], args[2], args[3], args[4], args[5], args[6]);
		switch(iResult)
		{
			case COMMAND_REQUIRE_ACCESS:
				CBotGlobals::BotMessage(pClient, 0, "You do not have access to this command.");
				break;
			case COMMAND_NOT_FOUND:
				CBotGlobals::BotMessage(pClient, 0, "Command not found.");
				break;
			case COMMAND_ERROR:
				CBotGlobals::BotMessage(pClient, 0, "Something went wrong.");
				break;
			default:
				break;
		}

		RETURN_META(MRES_SUPERCEDE);
	}

	// Capture voice commands
	CBotGlobals::GetCurrentMod()->ClientCommand(pClient, args.ArgC(), args[0], args[1], args[2]);
	RETURN_META(MRES_IGNORED);
}

void PlayerRunCmd(CUserCmd *pCmd, IMoveHelper *pMoveHelper)
{
	if (bot_stop.GetBool() || !CBotGlobals::IsMapRunning())
	{
		RETURN_META(MRES_IGNORED);
	}

	CBaseEntity *pEntity = META_IFACEPTR(CBaseEntity);
	edict_t *pEdict = gameents->BaseEntityToEdict(pEntity);
	CBot *pBot = CBots::GetBotPointer(pEdict);

	if (pBot && pBot->InUse() && pBot->IsAlive())
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
		pCmd->impulse = cmd->impulse;

		pls->v_angle = cmd->viewangles;
		pls->fixangle = FIXANGLE_NONE;
	}

	RETURN_META(MRES_IGNORED);
}

void AFKBot::OnClientPutInServer(int iClient)
{
	CBaseEntity *pEntity = gameents->EdictToBaseEntity(INDEXENT(iClient));
	CBotGlobals::GetCurrentMod()->PlayerSpawned(pEntity);
	SH_ADD_MANUALHOOK(PlayerRunCommand, pEntity, SH_STATIC(&PlayerRunCmd), false);
}

void AFKBot::OnClientDisconnecting(int iClient)
{
	CBaseEntity *pEntity = gameents->EdictToBaseEntity(INDEXENT(iClient));
	CBots::MakeNotBot(gameents->BaseEntityToEdict(pEntity));
	SH_REMOVE_MANUALHOOK(PlayerRunCommand, pEntity, SH_STATIC(&PlayerRunCmd), false);
}

void AFKBot::GameFrame(bool simulating)
{
	if (CBotGlobals::IsMapRunning())
	{
		CBots::BotThink(simulating);

	#if !defined USE_NAVMESH
		if(CWaypoints::WantToGenerate())
		{
			CWaypoints::ProcessGeneration();
		}

		CWaypointVisibilityTable *pTable = CWaypoints::GetVisiblity();
		if(pTable->NeedToWorkVisibility())
		{
			pTable->WorkVisibility();
		}
	#endif
	}
}

void AFKBot::NotifyInterfaceDrop(SMInterface *pInterface)
{
	if (pInterface == g_pSDKTools)
	{
		CBotGlobals::SetMapRunning(false);
		CBots::FreeMapMemory();
		CBotEvents::FreeMemory();
	}
}

void AFKBot::OnCoreMapStart(edict_t *pEdictList, int edictCount, int clientMax)
{
	if(bot_debug.GetBool())
	{
		AFKBot::BeginLogging(bot_log_resets.GetBool());
	}

	// Must set this
	CBotGlobals::SetMapName(gpGlobals->mapname);
	CBotGlobals::SetMapRunning(true);

	char error[288] = "";
	if(!CGameRulesObject::GetGameRules(error, sizeof(error)))
	{
		smutils->LogError(myself, error);
		CBotGlobals::SetMapRunning(false);
		return;
	}

#if defined USE_NAVMESH
	g_pNavMesh = CNavMeshLoader::Load(error, sizeof(error));
	if (error && *error)
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

	if (mp_teamplay)
		CBotGlobals::SetTeamplay(mp_teamplay->GetBool());
	else
		CBotGlobals::SetTeamplay(false);

	CBotEvents::SetupEvents();

	CBots::MapInit();
	CBotGlobals::GetCurrentMod()->MapInit();
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
	CGameRulesObject::FreeMemory();

	if (bot_debug.GetBool())
	{
		AFKBot::EndLogging();
	}
}

bool AFKBot::SDK_OnLoad(char *error, size_t maxlength, bool late)
{
	char conf_error[255] = "";
	if (!gameconfs->LoadGameConfigFile("afk.games", &g_pGameConf, conf_error, sizeof(conf_error)))
	{
		if (conf_error && *conf_error)
		{
			ke::SafeSprintf(error, maxlength, "Could not read afk.games.txt: %s", conf_error);
		}
		return false;
	}

	int iOffset;
	if (!g_pGameConf->GetOffset("PlayerRunCommand", &iOffset))
	{
		ke::SafeStrcpy(error, maxlength, "Could not find offset for PlayerRunCommand!");
		return false;
	} else {
		SH_MANUALHOOK_RECONFIGURE(PlayerRunCommand, iOffset, 0, 0);
	}

	if(!g_pGameConf->GetOffset("team_control_point_master", &iOffset))
	{
		ke::SafeStrcpy(error, maxlength, "Could not find offset to retrieve control point data!");
		return false;
	}
	else
	{
		bot_const_point_master_offset.SetValue(iOffset);
		bot_const_round_offset.SetValue(iOffset);
	}

	sharesys->AddDependency(myself, "sdktools.ext", true, true);
	sharesys->AddDependency(myself, "bintools.ext", true, true);

	// Register natives for Pawn
	extern sp_nativeinfo_t g_ExtensionNatives[];
	sharesys->AddNatives(myself, g_ExtensionNatives);
	sharesys->RegisterLibrary(myself, "AFKBot");

	playerhelpers->AddClientListener(this);
	playerhelpers->RegisterCommandTargetProcessor(this);

	ParamType params[] = { Param_Cell, Param_Cell }; // int client, bool enabled
	if ((forwardOnAFK = forwards->CreateForward("OnBotEnable", ET_Hook, 2, params)) == NULL)
		smutils->LogError(myself, "Warning: Unable to initialize OnBotEnable forward");

	SM_GET_IFACE(SDKTOOLS, g_pSDKTools);
	SM_GET_IFACE(BINTOOLS, g_pBinTools);

	CBotGlobals::InitModFolder();
	CBotGlobals::ReadRCBotFolder();

	if (!CBotGlobals::GameStart())
		return false;

	// Initialize bot variables
	CBotProfiles::SetupDefaultProfile();

#if !defined USE_NAVMESH
	CWaypointTypes::Setup();
	CWaypoints::SetupVisibility();
#endif

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
	GET_V_IFACE_ANY(GetServerFactory, gameents, IServerGameEnts, INTERFACEVERSION_SERVERGAMEENTS);
	GET_V_IFACE_ANY(GetServerFactory, gameclients, IServerGameClients, INTERFACEVERSION_SERVERGAMECLIENTS);
	GET_V_IFACE_ANY(GetServerFactory, playerinfomanager, IPlayerInfoManager, INTERFACEVERSION_PLAYERINFOMANAGER);
	GET_V_IFACE_ANY(GetServerFactory, servertools, IServerTools, VSERVERTOOLS_INTERFACE_VERSION);
	GET_V_IFACE_ANY(GetServerFactory, effects, IEffects, IEFFECTS_INTERFACE_VERSION);

#ifndef __linux__
	GET_V_IFACE_CURRENT(GetEngineFactory, debugoverlay, IVDebugOverlay, VDEBUG_OVERLAY_INTERFACE_VERSION);
#endif

#if SOURCE_ENGINE >= SE_ORANGEBOX
	g_pCVar = icvar;
#endif
	CONVAR_REGISTER(this);

	MathLib_Init();

	SH_ADD_HOOK(IServerGameDLL, GameFrame, gamedll, SH_MEMBER(this, &AFKBot::GameFrame), true);
	SH_ADD_HOOK(IServerGameClients, ClientCommand, gameclients, SH_STATIC(&OnClientCommand), true);
	SH_ADD_HOOK(IServerPluginHelpers, ClientCommand, helpers, SH_STATIC(&RateLimitedClientCommand), false);

	return true;
}

void AFKBot::SDK_OnAllLoaded()
{
	smutils->AddFrameAction(GAMERULES_FRAME_ACTION, NULL); // This is when the gamerules is created in SDKTools, so we wait a moment for it to be valid

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

		if (Q_stristr(sv_tags_str, "afkbot") == NULL)
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
	SM_CHECK_IFACE(SDKTOOLS, g_pSDKTools);

	return true;
}

void AFKBot::SDK_OnUnload()
{
	playerhelpers->RemoveClientListener(this);
	playerhelpers->UnregisterCommandTargetProcessor(this);

	gameconfs->CloseGameConfigFile(g_pGameConf);

	forwards->ReleaseForward(forwardOnAFK);

	CBots::FreeAllMemory();
	CStrings::FreeAllMemory();
	CBotMods::FreeMemory();
	CBotEvents::FreeMemory();
	CBotProfiles::DeleteProfiles();
	CWeapons::FreeMemory();
	CBotGlobals::FreeMemory();

#if defined USE_NAVMESH
	if (g_pNavMesh)
		delete g_pNavMesh;
#else
	CWaypoints::FreeMemory();
	CWaypointTypes::FreeMemory();
#endif
}

bool AFKBot::SDK_OnMetamodUnload(char *error, size_t maxlen)
{
	SH_REMOVE_HOOK(IServerGameDLL, GameFrame, gamedll, SH_MEMBER(this, &AFKBot::GameFrame), true);
	SH_REMOVE_HOOK(IServerGameClients, ClientCommand, gameclients, SH_STATIC(&OnClientCommand), true);
	SH_REMOVE_HOOK(IServerPluginHelpers, ClientCommand, helpers, SH_STATIC(&RateLimitedClientCommand), false);

	ConVar_Unregister();

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
			if (pInfo == NULL || pInfo->IsObserver())
				continue;

			if (playerhelpers->FilterCommandTarget(pAdmin, pPlayer, info->flags) == COMMAND_TARGET_VALID)
			{
				CBot *pBot = CBots::GetBotPointer(pPlayer->GetEdict());
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
		va_list argptr; char message[256];

		va_start(argptr, fmt);
		ke::SafeVsprintf(message, 255, fmt, argptr);
		va_end(argptr);

		char buffer[32];
		time_t dt = smutils->GetAdjustedTime();
		tm *curTime = localtime(&dt);
		strftime(buffer, sizeof(buffer), "%m/%d/%Y - %H:%M:%S", curTime);

		fprintf(m_pLogFile, "L %s: %s\n", buffer, message);
		fflush(m_pLogFile);
	}
}

void AFKBot::VerboseDebugMessage(const char *fmt, ...)
{
	if (bot_debug_verbose.GetBool())
	{
		va_list argptr; char message[256];
		va_start(argptr, fmt);
		ke::SafeVsprintf(message, 255, fmt, argptr);
		va_end(argptr);

		DebugMessage(message);
	}
}

void AFKBot::BeginLogging(bool bReset)
{
	static char sPath[MAX_PATH];
	if (sPath[0] == '\0') smutils->BuildPath(Path_SM, sPath, sizeof(sPath), "logs\\afkbot.log");

	const char *pMode = bReset ? "w+" : "at";
	FILE *pLogFile = fopen(sPath, pMode);
	if (pLogFile)
	{
		smutils->LogMessage(myself, "Begin logging in '%s'", sPath);
		m_pLogFile = pLogFile;
	}
}

void AFKBot::EndLogging()
{
	Assert( m_pLogFile );
	smutils->LogMessage(myself, "End logging session");
	fclose(m_pLogFile);
}