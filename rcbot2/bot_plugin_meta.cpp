/**
 * vim: set ts=4 sw=4 tw=99 noet :
 * ======================================================
 * Metamod:Source Sample Plugin
 * Written by AlliedModders LLC.
 * ======================================================
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from
 * the use of this software.
 *
 * This sample plugin is public domain.
 */

#include <stdio.h>

#include "bot_plugin_meta.h"

#include "interface.h"

#ifdef __linux__
#include "shake.h"    //bir3yk
#endif

#include "Color.h"
#include "server_class.h"
#include "time.h"

#include "KeyValues.h"

#include "bot_cvars.h"

// for IServerTools
#include "bot.h"
#include "bot_configfile.h"
#include "bot_globals.h"
#include "bot_profile.h"
#include "bot_waypoint.h"
#include "bot_menu.h"
#include "bot_getprop.h"
#include "bot_fortress.h"
#include "bot_event.h"
#include "bot_wpt_dist.h"
#include "bot_squads.h"
#include "bot_accessclient.h"
#include "bot_weapons.h"
#include "bot_waypoint_visibility.h"
#include "bot_kv.h"
#include "bot_sigscan.h"

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

SH_DECL_MANUALHOOK2_void(MHook_PlayerRunCmd, 0, 0, 0, CUserCmd*, IMoveHelper*);

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
INetworkStringTableContainer *netstringtables = NULL;
IEngineSound *engsound = NULL;
IServerGameDLL *servergamedll = NULL;

AFKBot g_AFKBot;

PLUGIN_EXPOSE(AFKBot, g_AFKBot);

IGameConfig *g_pGameConf = NULL;
ISDKTools *g_pSDKTools = NULL;

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

	CClientBroadcastRecipientFilter() {
		m_iMaxCount = 0;

		for (int i = 0; i < MAX_PLAYERS; ++i) {
			CClient* client = CClients::Get(i);

			if (client->IsUsed()) {
				IPlayerInfo *p = playerinfomanager->GetPlayerInfo(client->GetPlayer());

				if (p->IsConnected() && !p->IsFakeClient()) {
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

	while ((bOK = servergamedll->GetUserMessageInfo(msgid, msgbuf, 63, imsgsize)) == true)
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

CBaseEntity *AFKBot::TF2_GetPlayerWeaponSlot(edict_t *pPlayerEd, int iSlot)
{
	CPlayer *pPlayer;
	CEntity *pEntity = CEntity::Instance(pPlayerEd);
	if (pEntity)
	{
		pPlayer = dynamic_cast<CPlayer *>(pEntity);
		assert(pPlayer);
	}

	return pPlayer->Weapon_GetSlot(iSlot);
}

void AFKBot::TF2_EquipWeapon(edict_t *pPlayerEd, CBaseEntity *pWeapon)
{
	CPlayer *pPlayer;
	CEntity *pEntity = CEntity::Instance(pPlayerEd);
	if (pEntity)
	{
		pPlayer = dynamic_cast<CPlayer *>(pEntity);
		assert(pPlayer);
	}

	pPlayer->Weapon_Equip(pWeapon);
}

void AFKBot::Hook_PlayerRunCmd(CUserCmd *ucmd, IMoveHelper *moveHelper)
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
	}

	//g_pSM->LogMessage(NULL, "H %i | %i | %f | %f | %f | %f | %f | %i", ucmd->command_number, ucmd->tick_count, ucmd->viewangles.x, ucmd->viewangles.y, ucmd->viewangles.z, ucmd->forwardmove, ucmd->sidemove, ucmd->buttons); 

	RETURN_META(MRES_IGNORED);
}

/**
 * Something like this is needed to register cvars/CON_COMMANDs.
 */
class BaseAccessor : public IConCommandBaseAccessor
{
public:
	bool RegisterConCommandBase(ConCommandBase *pCommandBase)
	{
		/* Always call META_REGCVAR instead of going through the engine. */
		return META_REGCVAR(pCommandBase);
	}
} s_BaseAccessor;

bool AFKBot::Load(PluginId id, ISmmAPI *ismm, char *error, size_t maxlen, bool late)
{
	PLUGIN_SAVEVARS();

	GET_V_IFACE_CURRENT(GetEngineFactory, enginetrace, IEngineTrace, INTERFACEVERSION_ENGINETRACE_SERVER);
	GET_V_IFACE_CURRENT(GetEngineFactory, gameevents, IGameEventManager2, INTERFACEVERSION_GAMEEVENTSMANAGER2);
	GET_V_IFACE_CURRENT(GetEngineFactory, helpers, IServerPluginHelpers, INTERFACEVERSION_ISERVERPLUGINHELPERS);
	GET_V_IFACE_CURRENT(GetEngineFactory, icvar, ICvar, CVAR_INTERFACE_VERSION);

	GET_V_IFACE_CURRENT(GetEngineFactory, netstringtables, INetworkStringTableContainer, INTERFACENAME_NETWORKSTRINGTABLESERVER);
	GET_V_IFACE_CURRENT(GetEngineFactory, engsound, IEngineSound, IENGINESOUND_SERVER_INTERFACE_VERSION);

	GET_V_IFACE_ANY(GetEngineFactory, filesystem, IFileSystem, FILESYSTEM_INTERFACE_VERSION);

	GET_V_IFACE_ANY(GetServerFactory, servergameents, IServerGameEnts, INTERFACEVERSION_SERVERGAMEENTS);
	GET_V_IFACE_ANY(GetServerFactory, gameclients, IServerGameClients, INTERFACEVERSION_SERVERGAMECLIENTS);
	GET_V_IFACE_ANY(GetServerFactory, playerinfomanager, IPlayerInfoManager, INTERFACEVERSION_PLAYERINFOMANAGER);

	GET_V_IFACE_ANY(GetServerFactory, effects, IEffects, IEFFECTS_INTERFACE_VERSION);

#ifndef __linux__
	GET_V_IFACE_CURRENT(GetEngineFactory, debugoverlay, IVDebugOverlay, VDEBUG_OVERLAY_INTERFACE_VERSION);
#endif

	GET_V_IFACE_ANY(GetServerFactory, servergamedll, IServerGameDLL, INTERFACEVERSION_SERVERGAMEDLL);

	gpGlobals = ismm->GetCGlobals();

	META_LOG(g_PLAPI, "Starting plugin.");

	/* Load the VSP listener.  This is usually needed for IServerPluginHelpers. */
	if ((vsp_callbacks = ismm->GetVSPInfo(NULL)) == NULL)
	{
		ismm->AddListener(this, this);
		ismm->EnableVSPListener();
	}


	/*SH_ADD_HOOK_MEMFUNC(IVEngineServer, UserMessageBegin, engine, this, &AFKBot::Hook_MessageBegin, false);
	SH_ADD_HOOK_MEMFUNC(IVEngineServer, MessageEnd, engine, this, &AFKBot::Hook_MessageEnd, false);*/

	SH_ADD_HOOK_MEMFUNC(IServerGameDLL, LevelInit, servergamedll, this, &AFKBot::Hook_LevelInit, true);
	SH_ADD_HOOK_MEMFUNC(IServerGameDLL, ServerActivate, servergamedll, this, &AFKBot::Hook_ServerActivate, true);
	SH_ADD_HOOK_MEMFUNC(IServerGameDLL, GameFrame, servergamedll, this, &AFKBot::Hook_GameFrame, true);
	SH_ADD_HOOK_MEMFUNC(IServerGameDLL, LevelShutdown, servergamedll, this, &AFKBot::Hook_LevelShutdown, false);
	SH_ADD_HOOK_MEMFUNC(IServerGameClients, ClientActive, gameclients, this, &AFKBot::Hook_ClientActive, true);
	SH_ADD_HOOK_MEMFUNC(IServerGameClients, ClientDisconnect, gameclients, this, &AFKBot::Hook_ClientDisconnect, true);
	SH_ADD_HOOK_MEMFUNC(IServerGameClients, ClientPutInServer, gameclients, this, &AFKBot::Hook_ClientPutInServer, true);
	SH_ADD_HOOK_MEMFUNC(IServerGameClients, SetCommandClient, gameclients, this, &AFKBot::Hook_SetCommandClient, true);
	SH_ADD_HOOK_MEMFUNC(IServerGameClients, ClientSettingsChanged, gameclients, this, &AFKBot::Hook_ClientSettingsChanged, false);
	SH_ADD_HOOK_MEMFUNC(IServerGameClients, ClientConnect, gameclients, this, &AFKBot::Hook_ClientConnect, false);
	SH_ADD_HOOK_MEMFUNC(IServerGameClients, ClientCommand, gameclients, this, &AFKBot::Hook_ClientCommand, false);
	//Hook FireEvent to our function
	SH_ADD_HOOK_MEMFUNC(IGameEventManager2, FireEvent, gameevents, this, &AFKBot::FireGameEvent, false);


#if SOURCE_ENGINE >= SE_ORANGEBOX
	g_pCVar = icvar;
	ConVar_Register(0, &s_BaseAccessor);
#else
	ConCommandBaseMgr::OneTimeInit(&s_BaseAccessor);
#endif


	// Read Signatures and Offsets
	CBotGlobals::InitModFolder();
	CBotGlobals::ReadRCBotFolder();

	if (!CBotGlobals::GameStart())
		return false;

	CBotMod *pMod = CBotGlobals::GetCurrentMod();

	if (pMod->GetModId() == MOD_TF2)
		SH_MANUALHOOK_RECONFIGURE(MHook_PlayerRunCmd, 418, 0, 0);
	else if (pMod->GetModId() == MOD_DOD)
		SH_MANUALHOOK_RECONFIGURE(MHook_PlayerRunCmd, 418, 0, 0);

	// Find the RCBOT2 Path from metamod VDF
	extern IFileSystem *filesystem;
	KeyValues *mainkv = new KeyValues("metamodplugin");

	const char *rcbot2path;
	//CBotGlobals::botMessage(NULL, 0, "Reading rcbot2 path from VDF...");

	mainkv->LoadFromFile(filesystem, "addons/metamod/afkbot.vdf", "MOD");

	mainkv = mainkv->FindKey("Metamod Plugin");

	if (mainkv)
		rcbot2path = mainkv->GetString("rcbot2path", "\0");

	mainkv->deleteThis();
	//eventListener2 = new CRCBotEventListener();

	// Initialize bot variables
	CBotProfiles::SetupProfiles();

	//CBotEvents::setupEvents();
	CWaypointTypes::Setup();
	CWaypoints::SetupVisibility();

	CBotConfigFile::Reset();
	CBotConfigFile::Load();

	CBotMenuList::SetupMenus();

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


bool AFKBot::FireGameEvent(IGameEvent * pevent, bool bDontBroadcast)
{
	static char szKey[128];
	static char szValue[128];

	CBotEvents::ExecuteEvent((void*)pevent, TYPE_IGAMEEVENT);

	RETURN_META_VALUE(MRES_IGNORED, true);
}

bool AFKBot::Unload(char *error, size_t maxlen)
{
	SH_REMOVE_HOOK_MEMFUNC(IServerGameDLL, LevelInit, servergamedll, this, &AFKBot::Hook_LevelInit, true);
	SH_REMOVE_HOOK_MEMFUNC(IServerGameDLL, ServerActivate, servergamedll, this, &AFKBot::Hook_ServerActivate, true);
	SH_REMOVE_HOOK_MEMFUNC(IServerGameDLL, GameFrame, servergamedll, this, &AFKBot::Hook_GameFrame, true);
	SH_REMOVE_HOOK_MEMFUNC(IServerGameDLL, LevelShutdown, servergamedll, this, &AFKBot::Hook_LevelShutdown, false);
	SH_REMOVE_HOOK_MEMFUNC(IServerGameClients, ClientActive, gameclients, this, &AFKBot::Hook_ClientActive, true);
	SH_REMOVE_HOOK_MEMFUNC(IServerGameClients, ClientDisconnect, gameclients, this, &AFKBot::Hook_ClientDisconnect, true);
	SH_REMOVE_HOOK_MEMFUNC(IServerGameClients, ClientPutInServer, gameclients, this, &AFKBot::Hook_ClientPutInServer, true);
	SH_REMOVE_HOOK_MEMFUNC(IServerGameClients, SetCommandClient, gameclients, this, &AFKBot::Hook_SetCommandClient, true);
	SH_REMOVE_HOOK_MEMFUNC(IServerGameClients, ClientSettingsChanged, gameclients, this, &AFKBot::Hook_ClientSettingsChanged, false);
	SH_REMOVE_HOOK_MEMFUNC(IServerGameClients, ClientConnect, gameclients, this, &AFKBot::Hook_ClientConnect, false);
	SH_REMOVE_HOOK_MEMFUNC(IServerGameClients, ClientCommand, gameclients, this, &AFKBot::Hook_ClientCommand, false);

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

	GetEntityManager()->Shutdown();

	return true;
}

void AFKBot::OnVSPListening(IServerPluginCallbacks *iface)
{
	vsp_callbacks = iface;
}

void AFKBot::Hook_ServerActivate(edict_t *pEdictList, int edictCount, int clientMax)
{
	META_LOG(g_PLAPI, "ServerActivate() called: edictCount = %d, clientMax = %d", edictCount, clientMax);

	//CAccessClients::load();

	CBotGlobals::SetClientMax(clientMax);
}

void AFKBot::AllPluginsLoaded()
{
	char conf_error[255] = "";
	if (!gameconfs->LoadGameConfigFile("centity.offsets", &g_pGameConf, conf_error, sizeof(conf_error)))
	{
		if (conf_error[0])
		{
			META_LOG(g_PLAPI, "Could not read centity.offsets.txt: %s", conf_error);
			AFKBot::Unload(conf_error, sizeof(conf_error));
		}
	}

	if (!GetEntityManager()->Init(g_pGameConf))
	{
		META_LOG(g_PLAPI, "CEntity failed to init.");
		AFKBot::Unload("CEntity failed to init.", 24);
	}

	char *error; size_t maxlength;
	if (!g_pSDKTools)
	{
		META_LOG(g_PLAPI, "Could not find interface: ISDKTools");
		AFKBot::Unload("Could not find interface: ISDKTools", 36);
	}
}

void AFKBot::Hook_ClientActive(edict_t *pEntity, bool bLoadGame)
{
	META_LOG(g_PLAPI, "Hook_ClientActive(%d, %d)", IndexOfEdict(pEntity), bLoadGame);

	CClients::ClientActive(pEntity);
}

#if SOURCE_ENGINE >= SE_ORANGEBOX
void AFKBot::Hook_ClientCommand(edict_t *pEntity, const CCommand &args)
#else
void AFKBot::Hook_ClientCommand(edict_t *pEntity)
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
			CBotGlobals::BotMessage(pEntity, 0, "bot command not found");
		}
		else if (iResult == COMMAND_ERROR)
		{
			CBotGlobals::BotMessage(pEntity, 0, "bot command returned an error");
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

void AFKBot::Hook_ClientSettingsChanged(edict_t *pEdict)
{

}

bool AFKBot::Hook_ClientConnect(edict_t *pEntity,
	const char *pszName,
	const char *pszAddress,
	char *reject,
	int maxrejectlen)
{
	META_LOG(g_PLAPI, "Hook_ClientConnect(%d, \"%s\", \"%s\")", IndexOfEdict(pEntity), pszName, pszAddress);

	CClients::Init(pEntity);

	return true;
}

void AFKBot::Hook_ClientPutInServer(edict_t *pEntity, char const *playername)
{
	CBaseEntity *pEnt = servergameents->EdictToBaseEntity(pEntity);

	CClient *pClient = CClients::ClientConnected(pEntity);

	CBotMod *pMod = CBotGlobals::GetCurrentMod();

	pMod->PlayerSpawned(pEntity);

	if (pEnt)
	{
		//if (CBots::controlBots())
		SH_ADD_MANUALHOOK_MEMFUNC(MHook_PlayerRunCmd, pEnt, this, &AFKBot::Hook_PlayerRunCmd, false);
	}
}

void AFKBot::Hook_ClientDisconnect(edict_t *pEntity)
{
	CBaseEntity *pEnt = servergameents->EdictToBaseEntity(pEntity);

	if (pEnt)
	{
		CBotMod *pMod = CBotGlobals::GetCurrentMod();

		//if (CBots::controlBots())
		SH_REMOVE_MANUALHOOK_MEMFUNC(MHook_PlayerRunCmd, pEnt, this, &AFKBot::Hook_PlayerRunCmd, false);
	}

	CClients::ClientDisconnected(pEntity);

	META_LOG(g_PLAPI, "Hook_ClientDisconnect(%d)", IndexOfEdict(pEntity));
}

void AFKBot::Hook_GameFrame(bool simulating)
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
		if ( CClients::clientsDebugging(BOT_DEBUG_PROFILE) )
		{
			CProfileTimers::updateAndDisplay();
		}
#endif

		// Config Commands
		CBotConfigFile::DoNextCommand();
		currentmod = CBotGlobals::GetCurrentMod();

		currentmod->ModFrame();
	}
}

bool AFKBot::Hook_LevelInit(const char *pMapName,
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

	CProfileTimers::Reset();

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

void AFKBot::Hook_LevelShutdown()
{
	META_LOG(g_PLAPI, "Hook_LevelShutdown()");

	CClients::Initall();
	CWaypointDistances::Save();

	CBots::FreeMapMemory();
	CWaypoints::Init();

	CBotGlobals::SetMapRunning(false);
	CBotEvents::FreeMemory();
}

void AFKBot::Hook_SetCommandClient(int index)
{
	// META_LOG(g_PLAPI, "Hook_SetCommandClient(%d)", index);
}

bool AFKBot::Pause(char *error, size_t maxlen)
{
	return true;
}

bool AFKBot::Unpause(char *error, size_t maxlen)
{
	return true;
}

const char *AFKBot::GetLicense()
{
	return "GPL General Public License v3";
}

const char *AFKBot::GetVersion()
{
	return "1.0.0";
}

const char *AFKBot::GetDate()
{
	return __DATE__;
}

const char *AFKBot::GetLogTag()
{
	return "AFKBOT";
}

const char *AFKBot::GetAuthor()
{
	return "Deathreus, Cheeseh, Nightc0re";
}

const char *AFKBot::GetDescription()
{
	return "AFK player bot for most source games";
}

const char *AFKBot::GetName()
{
	return "AFK Bot";
}

const char *AFKBot::GetURL()
{
	return "";
}
