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

#include "interface.h"

#ifdef __linux__
#include "shake.h"    //bir3yk
#endif

#include "Color.h"
#include "server_class.h"
#include "time.h"

#include "KeyValues.h"

#include "rcbot2/bot_cvars.h"

#include "rcbot2/bot.h"
#include "rcbot2/bot_configfile.h"
#include "rcbot2/bot_globals.h"
#include "rcbot2/bot_profile.h"
#include "rcbot2/bot_waypoint.h"
#include "rcbot2/bot_menu.h"
#include "rcbot2/bot_getprop.h"
#include "rcbot2/bot_fortress.h"
#include "rcbot2/bot_event.h"
//#include "rcbot2/bot_profiling.h"
#include "rcbot2/bot_wpt_dist.h"
#include "rcbot2/bot_squads.h"
#include "rcbot2/bot_weapons.h"
#include "rcbot2/bot_waypoint_visibility.h"
#include "rcbot2/bot_kv.h"
#include "rcbot2/bot_sigscan.h"

using namespace SourceMM;

CBotTF2 *g_pLastBot;

SH_DECL_HOOK6(IServerGameDLL, LevelInit, SH_NOATTRIB, 0, bool, char const *, char const *, char const *, char const *, bool, bool);
SH_DECL_HOOK3_void(IServerGameDLL, ServerActivate, SH_NOATTRIB, 0, edict_t *, int, int);
SH_DECL_HOOK1_void(IServerGameDLL, GameFrame, SH_NOATTRIB, 0, bool);
SH_DECL_HOOK0_void(IServerGameDLL, LevelShutdown, SH_NOATTRIB, 0);
SH_DECL_HOOK2_void(IServerGameClients, ClientActive, SH_NOATTRIB, 0, edict_t *, bool);
SH_DECL_HOOK1_void(IServerGameClients, ClientDisconnect, SH_NOATTRIB, 0, edict_t *);
SH_DECL_HOOK2_void(IServerGameClients, ClientPutInServer, SH_NOATTRIB, 0, edict_t *, char const *);
SH_DECL_HOOK1_void(IServerGameClients, SetCommandClient, SH_NOATTRIB, 0, int);
SH_DECL_HOOK1_void(IServerGameClients, ClientSettingsChanged, SH_NOATTRIB, 0, edict_t *);
SH_DECL_HOOK5(IServerGameClients, ClientConnect, SH_NOATTRIB, 0, bool, edict_t *, const char*, const char *, char *, int);
SH_DECL_HOOK2(IGameEventManager2, FireEvent, SH_NOATTRIB, 0, bool, IGameEvent *, bool);


#if SOURCE_ENGINE >= SE_ORANGEBOX
SH_DECL_HOOK2_void(IServerGameClients, NetworkIDValidated, SH_NOATTRIB, 0, const char *, const char *);
SH_DECL_HOOK2_void(IServerGameClients, ClientCommand, SH_NOATTRIB, 0, edict_t *, const CCommand &);
#else
SH_DECL_HOOK1_void(IServerGameClients, ClientCommand, SH_NOATTRIB, 0, edict_t *);
#endif

SH_DECL_MANUALHOOK2_void(PlayerRunCmd, 0, 0, 0, CUserCmd*, IMoveHelper*);

AFKBot g_AFKBot;

SMEXT_LINK(&g_AFKBot);

CBaseEntity* (CBaseEntity::*TF2PlayerWeaponSlot)(int) = 0x0;
void (CBaseEntity::*TF2WeaponEquip)(CBaseEntity*) = 0x0;

IGameEventManager2 *gameevents = NULL;
IServerPluginCallbacks *vsp_callbacks = NULL;
ICvar *icvar = NULL;
IFileSystem *filesystem = NULL;  // file I/O 
IGameEventManager2 *gameeventmanager2 = NULL;
IGameEventManager *gameeventmanager = NULL;  // game events interface
IPlayerInfoManager *playerinfomanager = NULL;  // game dll interface to interact with players
IServerPluginHelpers *helpers = NULL;  // special 3rd party plugin helpers from the engine
IServerGameClients* gameclients = NULL;
IEngineTrace *enginetrace = NULL;
IEffects *effects = NULL;
CGlobalVars *gpGlobals = NULL;
IVDebugOverlay *debugoverlay = NULL;
IServerGameEnts *servergameents = NULL; // for accessing the server game entities

ConVar bot_version("afkbot_version", SMEXT_CONF_VERSION, FCVAR_SPONLY|FCVAR_REPLICATED|FCVAR_DONTRECORD|FCVAR_NOTIFY, "AFK Bot Version");
ConVar bot_enabled("afkbot_enabled", "1", FCVAR_NONE, "Enable turning players into bots?", true, 0.0f, true, 1.0f);

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

///////////////
// hud message
///////////////
void AFKBot::HudTextMessage(edict_t *pEntity, const char *szMessage)
{
	int msgid = 0;
	int imsgsize = 0;
	char msgbuf[64];
	bool bOK;

	int hint = -1;
	int say = -1;

	while ((bOK = gamedll->GetUserMessageInfo(msgid, msgbuf, 63, imsgsize)) == true)
	{
		if (strcmp(msgbuf, "HintText") == 0)
			hint = msgid;
		else if (strcmp(msgbuf, "SayText") == 0)
			say = msgid;

		msgid++;
	}

	if (msgid == 0)
		return;

	// if (!bOK)
	// return;

	CBotRecipientFilter *filter = new CBotRecipientFilter(pEntity);

	bf_write *buf = nullptr;

	if (hint > 0) 
	{
		buf = engine->UserMessageBegin(filter, hint);
		buf->WriteString(szMessage);
		engine->MessageEnd();
	}

	if (say > 0) 
	{
		char chatline[128];
		snprintf(chatline, sizeof(chatline), "\x01\x04[RCBot2]\x01 %s\n", szMessage);

		buf = engine->UserMessageBegin(filter, say);
		buf->WriteString(chatline);
		engine->MessageEnd();
	}

	delete filter;
}

//////////////////////////
// chat broadcast message
//////////////////////////
void AFKBot::BroadcastTextMessage(const char *szMessage)
{
	int msgid = 0;
	int imsgsize = 0;
	char msgbuf[64];
	bool bOK;

	int hint = -1;
	int say = -1;

	while ((bOK = gamedll->GetUserMessageInfo(msgid, msgbuf, 63, imsgsize)) == true)
	{
		if (strcmp(msgbuf, "HintText") == 0)
			hint = msgid;
		else if (strcmp(msgbuf, "SayText") == 0)
			say = msgid;

		msgid++;
	}

	if (msgid == 0)
		return;

	CClientBroadcastRecipientFilter *filter = new CClientBroadcastRecipientFilter();

	bf_write *buf = nullptr;

	if (say > 0) 
	{
		char chatline[128];
		snprintf(chatline, sizeof(chatline), "\x01\x04[RCBot2]\x01 %s\n", szMessage);

		buf = engine->UserMessageBegin(filter, say);
		buf->WriteString(chatline);
		engine->MessageEnd();
	}

	delete filter;
}

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
	static CBot *pBot;

	CBaseEntity *pEnt = META_IFACEPTR(CBaseEntity);

	edict_t *pEdict = servergameents->BaseEntityToEdict(pEnt);

	pBot = CBots::GetBotPointer(pEdict);

	if (pBot)
	{
		static CUserCmd *cmd;
		static CPlayerState *pl;

		cmd = pBot->GetUserCMD();
		pl = gameclients->GetPlayerState(pEdict);

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

		pl->v_angle = pBot->GetViewAngles();
		pl->fixangle = FIXANGLE_ABSOLUTE;

		g_pLastBot = (CBotTF2*)pBot;
		pBot->SetUserCMD(*ucmd);
	}

	//g_pSM->LogMessage(NULL, "H %i | %i | %f | %f | %f | %f | %f | %i", ucmd->command_number, ucmd->tick_count, ucmd->viewangles.x, ucmd->viewangles.y, ucmd->viewangles.z, ucmd->forwardmove, ucmd->sidemove, ucmd->buttons); 

	RETURN_META(MRES_IGNORED);
}

void OnVSPListening(IServerPluginCallbacks *iface)
{
	vsp_callbacks = iface;
}

void AFKBot::ServerActivate(edict_t *pEdictList, int edictCount, int clientMax)
{
	META_LOG(g_PLAPI, "ServerActivate() called: edictCount = %d, clientMax = %d", edictCount, clientMax);

	//CAccessClients::load();

	CBotGlobals::SetClientMax(clientMax);
}

void AFKBot::ClientActive(edict_t *pEntity, bool bLoadGame)
{
	META_LOG(g_PLAPI, "Hook_ClientActive(%d, %d)", IndexOfEdict(pEntity), bLoadGame);

	CClients::ClientActive(pEntity);
}

#if SOURCE_ENGINE >= SE_ORANGEBOX
void AFKBot::ClientCommand(edict_t *pEntity, const CCommand &args)
#else
void AFKBot::ClientCommand(edict_t *pEntity)
#endif
{
	static CBotMod *pMod = NULL;

#if SOURCE_ENGINE <= SE_DARKMESSIAH
	CCommand args;
#endif

	const char *pcmd = args.Arg(0);

	if (!pEntity || pEntity->IsFree())
	{
		return;
	}

	CClient *pClient = CClients::Get(pEntity);

	// is bot command?
	if (CBotGlobals::m_pCommands->IsCommand(pcmd))
	{
		eBotCommandResult iResult = CBotGlobals::m_pCommands->Execute(pClient, args.Arg(1), args.Arg(2), args.Arg(3), args.Arg(4), args.Arg(5), args.Arg(6));

		if (iResult == COMMAND_ACCESSED)
		{
			// ok
		}
		else if (iResult == COMMAND_REQUIRE_ACCESS)
		{
			CBotGlobals::BotMessage(pEntity, 0, "You do not have access to this command");
		}
		else if (iResult == COMMAND_NOT_FOUND)
		{
			CBotGlobals::BotMessage(pEntity, 0, "Command not found");
		}
		else if (iResult == COMMAND_ERROR)
		{
			CBotGlobals::BotMessage(pEntity, 0, "Command returned an error");
		}

		RETURN_META(MRES_SUPERCEDE);
	}
	else if (strncmp(pcmd, "menuselect", 10) == 0) // menu command
	{
		if (pClient->IsUsingMenu())
		{
			int iCommand = atoi(args.Arg(1));

			// format is 1.2.3.4.5.6.7.8.9.0
			if (iCommand == 0)
				iCommand = 9;
			else
				iCommand--;

			pClient->GetCurrentMenu()->SelectedMenu(pClient, iCommand);
		}
	}

	// command capturing
	pMod = CBotGlobals::GetCurrentMod();

	// capture some client commands e.g. voice commands
	pMod->ClientCommand(pEntity, args.ArgC(), pcmd, args.Arg(1), args.Arg(2));

	RETURN_META(MRES_IGNORED);
}

void AFKBot::ClientSettingsChanged(edict_t *pEdict)
{

}

bool AFKBot::ClientConnect(edict_t *pEntity,
	const char *pszName,
	const char *pszAddress,
	char *reject,
	int maxrejectlen)
{
	META_LOG(g_PLAPI, "Hook_ClientConnect(%d, \"%s\", \"%s\")", IndexOfEdict(pEntity), pszName, pszAddress);

	CClients::Init(pEntity);

	return true;
}

void AFKBot::ClientPutInServer(edict_t *pEntity, char const *playername)
{
	CClient *pClient = CClients::ClientConnected(pEntity);

	CBotMod *pMod = CBotGlobals::GetCurrentMod();

	pMod->PlayerSpawned(pEntity);

	META_LOG(g_PLAPI, "Hook_ClientPutInServer(%d)", IndexOfEdict(pEntity));
}

void AFKBot::ClientDisconnect(edict_t *pEntity)
{
	CClients::ClientDisconnected(pEntity);

	META_LOG(g_PLAPI, "Hook_ClientDisconnect(%d)", IndexOfEdict(pEntity));
}

void AFKBot::GameFrame(bool simulating)
{
	/**
	* simulating:
	* ***********
	* true  | game is ticking
	* false | game is not ticking
	*/

	static CBotMod *currentmod;

	if (simulating && CBotGlobals::IsMapRunning())
	{
		CBots::BotThink();
		CClients::ClientThink();

		if (CWaypoints::GetVisiblity()->NeedToWorkVisibility())
		{
			CWaypoints::GetVisiblity()->WorkVisibility();
		}

		// Profiling
#ifdef _DEBUG
		if (CClients::clientsDebugging(BOT_DEBUG_PROFILE))
		{
			CProfileTimers::UpdateAndDisplay();
		}
#endif

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
	META_LOG(g_PLAPI, "Hook_LevelInit(%s)", pMapName);

	//CClients::initall();
	// Must set this
	CBotGlobals::SetMapName(pMapName);

	Msg("Level \"%s\" has been loaded\n", pMapName);

	CWaypoints::PrecacheWaypointTexture();

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

	CClients::SetListenServerClient(NULL);

	// Setup game rules
	extern void *g_pGameRules;
	g_pGameRules = g_pSDKTools->GetGameRules();

	return true;
}

void AFKBot::LevelShutdown()
{
	META_LOG(g_PLAPI, "Hook_LevelShutdown()");

	CClients::Initall();
	CWaypointDistances::Save();

	CBots::FreeMapMemory();
	CWaypoints::Init();

	CBotGlobals::SetMapRunning(false);
	CBotEvents::FreeMemory();
}

void AFKBot::SetCommandClient(int index)
{
	// META_LOG(g_PLAPI, "Hook_SetCommandClient(%d)", index);
}

bool AFKBot::FireGameEvent(IGameEvent * pEvent, bool bDontBroadcast)
{
	static char szKey[128];
	static char szValue[128];

	CBotEvents::ExecuteEvent((void*)pEvent, TYPE_IGAMEEVENT);

	RETURN_META_VALUE(MRES_IGNORED, true);
}

bool AFKBot::SDK_OnLoad(char *error, size_t maxlen, bool late) {

	char conf_error[255] = "";
	if (!gameconfs->LoadGameConfigFile("tf2.afk", &g_pGameConf, conf_error, sizeof(conf_error)))
	{
		if (conf_error[0])
		{
			g_pSM->Format(error, maxlen, "Could not read tf2.afk.txt: %s", conf_error);
		}
		return false;
	}

	if (!g_pGameConf->GetOffset("PlayerRunCommand", &bot_playerruncmd_offset))
	{
		snprintf(error, maxlen, "Could not find offset for PlayerRunCmd");
		return false;
	} else {
		SH_MANUALHOOK_RECONFIGURE(PlayerRunCmd, bot_playerruncmd_offset, 0, 0);
	}

	if (!g_pGameConf->GetOffset("Weapon_Equip", &bot_weaponequip_offset))
	{
		snprintf(error, maxlen, "Could not find offset for Weapon_Equip");
		return false;
	}

	if (!g_pGameConf->GetOffset("Weapon_GetSlot", &bot_getweaponslot_offset))
	{
		snprintf(error, maxlen, "Could not find offset for Weapon_GetSlot");
		return false;
	}

#if SOURCE_ENGINE >= SE_ORANGEBOX
	g_pCVar = icvar;
	ConVar_Register(0, this);
#endif

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

	/* Load the VSP listener.  This is usually needed for IServerPluginHelpers. */
	if ((vsp_callbacks = ismm->GetVSPInfo(NULL)) == NULL)
	{
		ismm->AddListener(this, this);
		ismm->EnableVSPListener();
	}

	SH_ADD_HOOK_MEMFUNC(IServerGameDLL, LevelInit, gamedll, this, &AFKBot::LevelInit, true);
	SH_ADD_HOOK_MEMFUNC(IServerGameDLL, ServerActivate, gamedll, this, &AFKBot::ServerActivate, true);
	SH_ADD_HOOK_MEMFUNC(IServerGameDLL, GameFrame, gamedll, this, &AFKBot::GameFrame, true);
	SH_ADD_HOOK_MEMFUNC(IServerGameDLL, LevelShutdown, gamedll, this, &AFKBot::LevelShutdown, false);
	SH_ADD_HOOK_MEMFUNC(IServerGameClients, ClientActive, gameclients, this, &AFKBot::ClientActive, true);
	SH_ADD_HOOK_MEMFUNC(IServerGameClients, ClientDisconnect, gameclients, this, &AFKBot::ClientDisconnect, true);
	SH_ADD_HOOK_MEMFUNC(IServerGameClients, ClientPutInServer, gameclients, this, &AFKBot::ClientPutInServer, true);
	SH_ADD_HOOK_MEMFUNC(IServerGameClients, SetCommandClient, gameclients, this, &AFKBot::SetCommandClient, true);
	SH_ADD_HOOK_MEMFUNC(IServerGameClients, ClientSettingsChanged, gameclients, this, &AFKBot::ClientSettingsChanged, false);
	SH_ADD_HOOK_MEMFUNC(IServerGameClients, ClientConnect, gameclients, this, &AFKBot::ClientConnect, false);
	SH_ADD_HOOK_MEMFUNC(IServerGameClients, ClientCommand, gameclients, this, &AFKBot::ClientCommand, false);
	SH_ADD_HOOK_MEMFUNC(IGameEventManager2, FireEvent, gameevents, this, &AFKBot::FireGameEvent, false);


	// Read Signatures and Offsets
	CBotGlobals::InitModFolder();
	CBotGlobals::ReadRCBotFolder();

	if (!CBotGlobals::GameStart())
		return false;

	CBotMod *pMod = CBotGlobals::GetCurrentMod();

	// Initialize bot variables
	CBotProfiles::SetupProfiles();

	//CBotEvents::setupEvents();
	CWaypointTypes::Setup();
	CWaypoints::SetupVisibility();

	CBotConfigFile::Reset();
	CBotConfigFile::Load();

	CBotMenuList::SetupMenus();

	CClassInterface::Init();

	mp_stalemate_enable = icvar->FindVar("mp_stalemate_enable");
	mp_stalemate_meleeonly = icvar->FindVar("mp_stalemate_meleeonly");
	sv_cheats = icvar->FindVar("sv_cheats");
	sv_gravity = icvar->FindVar("sv_gravity");
	mp_friendlyfire = icvar->FindVar("mp_friendlyfire");
	sv_tags = icvar->FindVar("sv_tags");
	mp_teamplay = icvar->FindVar("mp_teamplay");

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
		g_pSM->LogMessage(myself, "Unable to retrieve interface: SDKTOOLS!");
	}

	// Register natives for Pawn
	sharesys->AddNatives(myself, g_ExtensionNatives);
	sharesys->RegisterLibrary(myself, "AFKBot");
}

void AFKBot::SDK_OnUnload()
{
	gameconfs->CloseGameConfigFile(g_pGameConf);
}

bool AFKBot::QueryRunning(char *error, size_t maxlength)
{
	SM_CHECK_IFACE(SDKTOOLS, g_pSDKTools);

	return true;
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
	SH_REMOVE_HOOK_MEMFUNC(IServerGameClients, SetCommandClient, gameclients, this, &AFKBot::SetCommandClient, true);
	SH_REMOVE_HOOK_MEMFUNC(IServerGameClients, ClientSettingsChanged, gameclients, this, &AFKBot::ClientSettingsChanged, false);
	SH_REMOVE_HOOK_MEMFUNC(IServerGameClients, ClientConnect, gameclients, this, &AFKBot::ClientConnect, false);
	SH_REMOVE_HOOK_MEMFUNC(IServerGameClients, ClientCommand, gameclients, this, &AFKBot::ClientCommand, false);
	SH_REMOVE_HOOK_MEMFUNC(IGameEventManager2, FireEvent, gameevents, this, &AFKBot::FireGameEvent, false);

	CBots::FreeAllMemory();
	CStrings::FreeAllMemory();
	CBotGlobals::FreeMemory();
	CBotMods::FreeMemory();
	//CAccessClients::FreeMemory();
	CBotEvents::FreeMemory();
	CWaypoints::FreeMemory();
	CWaypointTypes::FreeMemory();
	CBotProfiles::DeleteProfiles();
	CWeapons::FreeMemory();
	CBotMenuList::FreeMemory();

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
	cell_t tClient = params[1];
	edict_t *pClient = PEntityOfEntIndex(tClient);

	IPlayerInfo *p = playerinfomanager->GetPlayerInfo(pClient);

	if (p && p->IsConnected() && !p->IsFakeClient() && !p->IsObserver())
	{
		if (params[2])
			CBots::MakeBot(pClient);
		else if (!params[2])
			CBots::MakeNotBot(pClient);
	}
	else
		return pContext->ThrowNativeError("Invalid client %d\nHe is either not connected, a fake client, or in spectator", params[1]);
}

static cell_t IsClientAFKBot(IPluginContext *pContext, const cell_t *params)
{
	cell_t tClient = params[1];

	IPlayerInfo *p = playerinfomanager->GetPlayerInfo(PEntityOfEntIndex(tClient));

	if (p && p->IsConnected() && !p->IsFakeClient() && !p->IsObserver())
	{
		if (CBots::Get(tClient)->InUse())
			return 1;
	}
	else
		return pContext->ThrowNativeError("Invalid client %d\nHe is either not connected, a fake client, or in spectator", params[1]);

	return 0;
}
